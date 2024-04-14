#include <algorithm>
#include <cmath>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Barcode/PrjBarlib/include/barscalar.h"
#include <random>

import LayersCore;
import RasterLayers;
import BackBind;
import ProjectModule;
import MLSettings;
import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;

// Linear interpolation function

struct Cell
{
	Barscalar color;
	float proc = 1.f;
};

template <class T>
struct Field
{
public:
	Field(int w, int h) : width(w), height(h)
	{
		field.resize(width * height);
	}

	std::vector<T> field;
	int width;
	int height;

	T& get(int x, int y)
	{
		return field[y * width + x];
	}
	void set(int x, int y, const T& c)
	{
		field[y * width + x] = c;
	}

	T& getLiner(int pos)
	{
		return field[pos];
	}

	size_t length() const
	{
		return field.size();
	}
};


// 1. Диффьюзия - сначала случайные числа
// 2. Новое состояние на основе соседей

// Приблизить
// Если есть 3 похожих - то взять новое
// Если нет похожих - приблизить к исходному

// Меняем цвет на основе состояния?



struct CellsField : public Field<Cell>
{
public:
	CellsField(int w, int h) : Field(w, h)
	{
	}

	CellsField(const CellsField& other) : Field(other.width, other.height)
	{
		std::copy(other.field.begin(), other.field.end(), field.begin());
	}

	void operator=(CellsField&& other)
	{
		field = std::move(other.field);
	}

	void operator=(const CellsField& other)
	{
		field = other.field;
	}

	void fillRandom(BarType type)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, 255);
		if (type == BarType::BYTE8_3)
		{
			for (auto& c : field)
			{
				c.color = Barscalar(distrib(gen), distrib(gen), distrib(gen));
				// std::fill_n(c.pos, 8, 0);
			}
		}
		else
		{
			for (auto& c : field)
			{
				c.color = Barscalar(distrib(gen),type);
				// std::fill_n(c.pos, 8, 0);
			}
		}
	}
};

struct BarField : public Field<bc::barline*>
{
public:
	BarField(int w, int h) : Field(w, h)
	{
	}
};

struct ProcField : public Field<float>
{
public:
	ProcField(int w, int h) : Field(w, h)
	{
	}

	ProcField(const ProcField& other) : Field(other.width, other.height)
	{
		std::copy(other.field.begin(), other.field.end(), field.begin());
	}

	void operator=(ProcField&& other)
	{
		field = std::move(other.field);
	}

	void operator=(const ProcField& other)
	{
		field = other.field;
	}

	void fillRandom()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distrib(0, 1.0);

		for (auto& c : field)
		{
			c = distrib(gen);
			// std::fill_n(c.pos, 8, 0);
		}
	}
};

struct MM
{
	float l = 1.0, u = 1.0;
};

// * * * * *
// * # # # *
// * # @ # *
// * # # # *
// * * * * *

// constexpr int keylen = 24;
// constexpr static char poss[keylen][2] = {
// 	{-2,-2}, {-2, -1}, {-2, 0}, {-2, 1}, {-2, 2},
// 	{-1,-2}, {-1, -1}, {-1, 0}, {-1, 1}, {-1, 2},
// 	{0,-2}, {0, -1},         {0, 1}, {0, 2},
// 	{1,-2}, {1, -1}, {1, 0}, {1, 1}, {1, 2},
// 	{2,-2}, {2, -1}, {2, 0}, {2, 1}, {2, 2}
// };

constexpr int keylen = 9;
constexpr static char poss[keylen][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 }, {0,0}};

// constexpr int keylen = 4;
// constexpr static char poss[keylen][2] = { { -1,0 },{ 0,-1 },{ 1,0 },{ 0,1 },{ -1,0 } };


struct HashK
{
	float key[keylen];
	float sum;
	void set()
	{
		sum = 0;
		for (int i = 0; i < keylen; ++i)
		{
			sum += key[i];
		}
	}
	float value;
};

class Trainer
{
	std::vector<HashK> train;
public:
	void add(const HashK& scase)
	{
		train.push_back(scase);
	}

	struct Line
	{
		float value;
		int id = 0;
		int marks = 0;
		int nodesId[keylen];
	};

	class Node
	{
	public:
		// Line* line = nullptr;
		Node(Line* l, float edge)
		{
			addLine(l);
			this->edge = edge;
		}

		void addLine(Line* l)
		{
			lines.push_back(l);
		}
		std::vector<Line*> lines;
		float edge; // Edge
	};

	struct KeyColumn
	{
		std::vector<Node*> nodes;
		std::vector<float> trainValues;
		std::vector<Line*> lines;

		void sort()
		{
			std::sort(nodes.begin(), nodes.end(), [](const Node* a,const Node* b) {
				// for (int i = 0; i < 9; ++i)
				// {
				// 	if (a.key[i] != b.key[i])
				// 		return a.key[i] < b.key[i];
				// }
				return a->edge < b->edge;
			});
		}

		void findCloser(float value, std::unordered_map<int, int>& clines)
		{
			Node* closer;
			// Implement binary search
			int l = 0, r = nodes.size() - 1;
			while (l < r)
			{
				int m = (l + r) / 2;
				if (nodes[m]->edge < value)
					l = m + 1;
				else
					r = m;
			}
			closer = nodes[l];

			float diff = abs(value - closer->edge);
			for (auto* line : closer->lines)
			{
				auto it = clines.find(line->id);
				if (it != clines.end())
				{
					it->second++;
				}
				else
					clines[line->id] = 1;
			}
		}
	};

	std::vector<Line> lines;
	KeyColumn columns[keylen];
	std::vector<std::unique_ptr<Node>> nodeCollector;

	void traint()
	{
		lines.resize(train.size());
		for (int i = 0; i < train.size(); i++)
		{
			Line* l = &lines[i];
			l->id = i;
			l->value = train[i].value;

			for (int j = 0; j < keylen; ++j)
			{
				columns[j].trainValues.push_back(train[i].key[j]);
				columns[j].lines.push_back(l);
			}
		}

		for (int j = 0; j < keylen; ++j)
		{
			auto& column = columns[j];
			std::sort(column.trainValues.begin(), column.trainValues.end(), [](float a, float b){
				return a < b;
			});

			// Aggregate
			nodeCollector.push_back(std::make_unique<Node>(column.lines[0], train[0].key[j]));
			column.nodes.push_back(nodeCollector.back().get());
			for (int i = 1; i < column.trainValues.size(); i++)
			{
				float prevValue = column.trainValues[i - 1];
				float curValue = column.trainValues[i];
				auto* line = column.lines[i];
				if (prevValue == curValue)
				{
					line->nodesId[j] = nodeCollector.size() - 1;
					column.nodes.back()->addLine(line);
				}
				else
				{
					line->nodesId[j] = nodeCollector.size();
					nodeCollector.push_back(std::make_unique<Node>(line, curValue));
					column.nodes.push_back(nodeCollector.back().get());
				}
			}
		}
	}

	int getCloser(const HashK& scase)
	{
		// for (int i = 0; i < train.size(); i++)
		// {
		// 	lines[i].marks = 0;
		// }

		std::unordered_map<int, int> closesLines;
		for (int j = 0; j < keylen; ++j)
		{
			columns[j].findCloser(scase.key[j], closesLines);
		}
		auto max = std::max_element(closesLines.begin(), closesLines.end());

		// std::vector<std::pair<int, float>> diffs;
		// for (int j = 0; j < closesLines.size(); ++j)
		// {
		// 	float diff = 0;
		// 	auto& closestTrain = train[closesLines[j]->id];
		// 	for (int j = 0; j < keylen; ++j)
		// 	{
		// 		float a = closestTrain.key[j] - scase.key[j];
		// 		diff += a * a;
		// 	}
		// 	diff = sqrt(diff);
		// 	diffs.push_back({j, diff});

		// 	closesLines[j]->marks = 0;
		// }

		// auto y = std::min_element(diffs.begin(), diffs.end(), [](auto a, auto b) {
		// 	return a.second < b.second;
		// });


		return max->first;
	}
};


RetLayers exeGenColor(InOutLayer iol, const MLSettings& setting)
{
	float adj = *setting.getDouble("aje");
	int step = *setting.getInt("step");

	RetLayers ret;
	BackImage src;

	RasterLayer* rasterSpot = getSrcFromInput(iol, src);
	ret.push_back(rasterSpot);

	BackImage& out = rasterSpot->mat;

	bc::BarcodeCreator bcc;
	bc::barstruct constr = getConstr(setting);
	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
	// bc::Baritem* item = containner.get();

	Barscalar imgmin;
	Barscalar imgmax;
	src.maxAndMin(imgmin, imgmax);
	if (imgmin > imgmax)
	{
		std::swap(imgmin, imgmax);
	}

	Barscalar diffcolor = imgmax - imgmin;

	float lower = imgmin.getAvgFloat();
	float upper = imgmax.getAvgFloat();
	float ludiff = upper - lower;

	bc::Baritem* item = (bcc.createBarcode(&src, constr));


	ProcField cells(src.width(), src.height());
	cells.fillRandom();//out.getType());

	BarField bar(src.width(), src.height());
	//Field<Barscalar> srcField(src.width(), src.height());

	Trainer train;
	for (size_t i = 0; i < src.length(); ++i)
	{
		// auto line = item->barlines[i];
		// const auto& matr = line->matr;

		// // float lower = line->start.getAvgFloat();
		// // float upper = line->end().getAvgFloat();
		// // if (lower > upper)
		// // 	std::swap(lower, upper);
		// // float ludiff = upper - lower;

		// // BackSize maskSize = b;
		// for (const auto& pm : matr)
		// {
			int x = i % src.width();
			int y = i / src.width();
			// bar.set(x, y, line);
			// //srcField.set(x, y, pm.value);
			// // out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat() / dummy));
			// //out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());

			auto rscal = src.get(x, y);
			HashK h;
			h.value = (rscal.getAvgFloat() - lower) / ludiff;
			cells.set(x, y, h.value);
			for (buchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= src.width() || yi < 0 || yi >= src.height())
				{
					h.key[j] = 0;
					continue;
				}

				auto srcscalr = src.get(xi, yi);
				h.key[j] = (srcscalr.getAvgFloat() - lower) / ludiff;
			}
			h.set();
			train.add(h);
		// }
	}

	train.traint();

	ProcField newCells(cells);

	int offset = 0;
	for (int k = 0; k < step; k++)
	{
		for (int i = offset; i < cells.length(); i+= 3)
		{
			int x = i % cells.width;
			int y = i / cells.width;

			// bc::barline* line = bar.get(x, y);

			HashK h;
			for (buchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
				{
					h.key[j] = 0;
					continue;
				}

				float srcscalr = cells.get(xi, yi);
				h.key[j] = srcscalr;
			}
			// h.set();


			int id = train.getCloser(h);
			auto& line = train.lines[id];
			float val = line.value;

			// float& newCell = newCells.get(x, y);
			// newCell += abs(cell - val) * adj;


			for (int j = 0; j < keylen; j++)
			{
				val = train.nodeCollector[line.nodesId[j]]->edge;
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				float cell = cells.get(xi, yi);
				float& newCell = newCells.get(x, y);

				newCell += (val - cell) * adj;
			}

			// newCell = val;
			// if (cell < val)
			// 	newCell += adj;
			// else
			// 	newCell -= adj;
		}
		cells = newCells;
		offset = (offset + 1) % 3;
	}

	// for (int i = 0; i < cells.length(); i++)
	// {
	// 	auto line = bar.getLiner(i);
	// 	Barscalar upper = line->end().getAvgFloat();
	// 	Barscalar lower = line->start.getAvgFloat();
	// 	if (lower > upper)
	// 		std::swap(lower, upper);
	// 	Barscalar diff = upper - lower;


	// 	if (color > 1.0)
	// 		color = 1.0;
	// 	if (color < 0.0)
	// 		color = 0.0;
	// 	lien->setAvg(color);

	// 	out.setLiner(i, cells.getLiner(i));
	// }

	float ludiffas = ludiff / upper;
	for (int i = 0; i < cells.length(); i++)
	{
	 	float colorProc = cells.getLiner(i);
		if (colorProc > 1.0)
			colorProc = 1.0;
		else if (colorProc < 0)
			colorProc = 0;

		Barscalar newColor = imgmin + diffcolor * colorProc;
		out.setLiner(i, newColor);
	}


	// for (size_t i = 0; i < item->barlines.size(); ++i)
	// {
	// 	float colorProc = cells.getLiner(i);
	// 	auto line = item->barlines[i];
	// 	const auto& matr = line->matr;

	// 	Barscalar upper = line->end().getAvgFloat();
	// 	Barscalar lower = line->start.getAvgFloat();
	// 	if (lower > upper)
	// 		std::swap(lower, upper);
	// 	Barscalar diff = upper - lower;
	// 	// Barscalar maxColor(255, item->getType());
	// 	// auto b = maxColor - lower;
	// 	// b /= colorProc;
	// 	// if ()



	// 	// BackSize maskSize = b;
	// 	// for (const auto& pm : matr)
	// 	// {
	// 	// 	float r = (pm.value.getAvgFloat() - lower) / (upper - lower);
	// 	// 	auto newColor = imgmax * colorProc;
	// 	// 	out.set(pm.getX(), pm.getY(), newColor);
	// 	// }
	// }
	return ret;
}

MLSettings mkSettingsTypeColor()
{
	MLSettings settings = mkSettingsType();

	OptionValue comp2("step", 1);
	OptionValue comp3("aje", 0.01);
	settings.values.push_back(comp2);
	settings.values.push_back(comp3);

	return settings;
}


volatile static AlgFuncRegister registerExeGenColor("exeGenColor", exeGenColor, mkSettingsTypeColor, "Diffusion");




enum CellState
{
	SourceColor = 0,
	Expance = 0
};


struct ExCell
{
	CellState state = SourceColor;
};

// struct ExpnField : public Field<ExCell>
// {
// public:
// 	ExpnField(int w, int h) : Field(w, h)
// 	{
// 	}

// 	void fillRandom(BarType type)
// 	{
// 		std::random_device rd;
// 		std::mt19937 gen(rd());
// 		std::uniform_int_distribution<> distrib(0, 255);
// 		if (type == BarType::BYTE8_3)
// 		{
// 			for (auto& c : field)
// 			{
// 				c.color = Barscalar(distrib(gen), distrib(gen), distrib(gen));
// 				// std::fill_n(c.pos, 8, 0);
// 			}
// 		}
// 		else
// 		{
// 			for (auto& c : field)
// 			{
// 				c.color = Barscalar(distrib(gen),type);
// 				// std::fill_n(c.pos, 8, 0);
// 			}
// 		}
// 	}
// };


// RetLayers exeGenColorPos(InOutLayer iol, const MLSettings& setting)
// {
// 	RetLayers ret;
// 	BackImage src;
// 	RasterLayer* rasterSpot = getSrcFromInput(iol, src);
// 	ret.push_back(rasterSpot);

// 	BackImage& out = rasterSpot->mat;

// 	bc::BarcodeCreator bcc;
// 	bc::barstruct constr = getConstr(setting);
// 	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
// 	// bc::Baritem* item = containner.get();

// 	bc::Baritem* item = (bcc.createBarcode(&src, constr));


// 	CellsField cells(src.width(), src.height());
// 	cells.fillRandom(out.getType());

// 	BarField bar(src.width(), src.height());

// 	for (size_t i = 0; i < item->barlines.size(); ++i)
// 	{
// 		auto line = item->barlines[i];
// 		const auto& matr = line->matr;

// 		// BackSize maskSize = b;
// 		for (const auto& pm : matr)
// 		{
// 			bar.set(pm.getX(), pm.getY(), line);
// 			// out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat() / dummy));
// 			//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());
// 		}
// 	}


// 	CellsField newCells(cells);
// 	for (int i = 0; i < cells.length(); i++)
// 	{
// 		int x = i % cells.width;
// 		int y = i / cells.width;

// 		bc::barline* line = bar.get(x, y);
// 		Cell& cell = cells.get(x, y);
// 		static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };

// 		Barscalar avgc(0, BarType::FLOAT32_3);
// 		Barscalar maxcol(0, BarType::BYTE8_3);
// 		int cee = 0;
// 		for (buchar j = 0; j < 8; ++j)
// 		{
// 			int xi = x + poss[j][0];
// 			int yi = y + poss[j][1];

// 			if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
// 				continue;

// 			bc::barline* pline = bar.get(xi, yi);
// 			Cell& pcell = cells.get(xi, yi);

// 			if (pcell.state == DirectionLeft + j)
// 			{
// 				++cee;
// 			}
// 		}

// 		if (cee < 3)
// 		{
// 			newCells.get(x, y).state = SourceColor;
// 		}
// 	}

// 	for (int i = 0; i < cells.length(); i++)
// 	{
// 		out.setLiner(i, newCells.getLiner(i).color);
// 	}

// 	return ret;
// }

// static AlgFuncRegister registerExeGenColor2("exeGenColor2", exeGenColorPos, mkSettingsType, "Diffusion");