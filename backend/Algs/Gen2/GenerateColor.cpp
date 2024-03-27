#include <algorithm>
#include <cmath>
#include <cstddef>
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

enum CellState
{
	SourceColor = 0,
	DirectionLeft = 1,
	DirectionUpLeft,
	DirectionUp,
	DirectionUpRight,
	DirectionRight,
	DirectionDownRight,
	DirectionDown,
	DirectionDownLeft,
	SourceGet
};

struct Cell
{
	Barscalar color;
	CellState state = SourceColor;
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
struct HashK
{
	float key[9];
	float sum;
	void set()
	{
		sum = 0;
		for (int i = 0; i < 9; ++i)
		{
			sum += key[i];
		}
	}
	float value;
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

	bc::Baritem* item = (bcc.createBarcode(&src, constr));


	std::vector<HashK> train;

	ProcField cells(src.width(), src.height());
	cells.fillRandom();//out.getType());

	BarField bar(src.width(), src.height());
	Field<Barscalar> srcField(src.width(), src.height());

	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto line = item->barlines[i];
		const auto& matr = line->matr;

		float lower = line->start.getAvgFloat();
		float upper = line->end().getAvgFloat();
		if (lower > upper)
			std::swap(lower, upper);

		// BackSize maskSize = b;
		for (const auto& pm : matr)
		{
			int x = pm.getX();
			int y = pm.getY();
			bar.set(x, y, line);
			srcField.set(x, y, pm.value);
			// out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat() / dummy));
			//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());

			HashK h;
			h.value = (pm.value.getAvgFloat() - lower) / (upper - lower);
			static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };
			for (buchar j = 0; j < 8; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
				{
					h.key[j] = 0;
					continue;
				}
				auto srcscalr = srcField.get(xi, yi);

				h.key[j] = (srcscalr.getAvgFloat() - lower) / (upper - lower);
			}
			h.set();
			train.push_back(h);
		}
	}

	// std::sort(train.begin(), train.end(), [](auto& a, auto& b) { return a.value < b.value; });


	ProcField newCells(cells);

	for (int k = 0; k < step; k++)
	{
		for (int i = 0; i < cells.length(); i++)
		{
			int x = i % cells.width;
			int y = i / cells.width;

			bc::barline* line = bar.get(x, y);
			float& cell = cells.get(x, y);
			static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };

			HashK h;
			h.value = cell;
			for (buchar j = 0; j < 8; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
				{
					h.key[j] = 0;
					continue;
				}
				auto srcscalr = cells.get(xi, yi);

				h.key[j] = srcscalr;
			}
			h.set();

			// Find the closest value in the train by binary search
			int minIndex = 0;
			int left = 0;
			int right = train.size() - 1;
			while (left <= right)
			{
				int mid = (left + right) / 2;
				float distance = train[mid].value - h.value;
				if (distance < 0)
					right = mid - 1;
				else if (distance > 0)
					left = mid + 1;
				else
				{
					minIndex = mid;
					break;
				}
			}


			// float minDistance = 10000.0;
			// int minIndex = -1;
			// int j = 0;
			// for (auto& t : train)
			// {
			// 	float distance = 0.0;
			// 	// for (int off = 0; off < 9; off++)
			// 	// {

			// 	// }
			// 	for (int k = 0; k < 9; k++)
			// 	{
			// 		float diff = h.key[k] - t.key[k];
			// 		distance += diff*diff;
			// 	}
			// 	distance = sqrt(distance);
			// 	if (distance < minDistance)
			// 	{
			// 		minDistance = distance;
			// 		minIndex = j;
			// 	}
			// 	j++;
			// }

			if (cell < train[minIndex].value)
				cell += adj;
			else
				cell -= adj;
		}
		cells = newCells;
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

	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		float colorProc = cells.getLiner(i);
		auto line = item->barlines[i];
		const auto& matr = line->matr;

		Barscalar upper = line->end().getAvgFloat();
		Barscalar lower = line->start.getAvgFloat();
		if (lower > upper)
			std::swap(lower, upper);
		Barscalar diff = upper - lower;
		// Barscalar maxColor(255, item->getType());
		// auto b = maxColor - lower;
		// b /= colorProc;
		// if ()



		// BackSize maskSize = b;
		for (const auto& pm : matr)
		{
			// float r = (pm.value.getAvgFloat() - lower) / (upper - lower);
			auto newColor = lower + diff * colorProc;
			out.set(pm.getX(), pm.getY(), newColor);
		}
	}
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


static AlgFuncRegister registerExeGenColor("exeGenColor", exeGenColor, mkSettingsTypeColor);




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

// static AlgFuncRegister registerExeGenColor2("exeGenColor2", exeGenColorPos, mkSettingsType);