#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Barcode/PrjBarlib/include/barscalar.h"
#include <random>
#include "GenCommon.h"
#include "Common.h"
#include "Trainer.h"


#include "../../MLSettings.h"
#include "../../MatrImg.h"
import LayersCore;
import RasterLayers;
//import BackBind;
import ProjectModule;
// import MLSettings;
// import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;

// Linear interpolation function

struct Cell
{
	Barscalar color;
	float proc = 1.f;
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

constexpr int keylen = 8;
constexpr static char poss[keylen][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 }};

// constexpr int keylen = 4;
// constexpr static char poss[keylen][2] = { { -1,0 },{ 0,-1 },{ 1,0 },{ 0,1 },{ -1,0 } };



RetLayers exeGenColor(InOutLayer iol, const MLSettings& setting)
{
	float adj = *setting.getDouble("aje");
	int step = *setting.getInt("step");
	const bool debugDraw = *setting.getBool("DebugDraw");
	const bool halfRand = *setting.getBool("HalfRand");
	const bool skipNotFull = *setting.getBool("SkipNotFull");
	const bool useImageAsNoise = *setting.getBool("UseImageAsNoise");

	RetLayers ret;
	BackImage src;

	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	RasterLayer* srcNoise;

	if (!useImageAsNoise)
	{
		srcNoise = genOutputFromInput(iol, src);
		ret.push_back(srcNoise);
	}
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



	ProcField cells(src.width(), src.height());

	std::random_device rd;
	std::default_random_engine genrd(0);
	// std::mt19937 gen(genrd());
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(1, 100);

	Trainer<keylen> train(debugDraw);
	std::cout << "Training" << std::endl;
	if (!useImageAsNoise)
	{
		cells.fillRandom();//out.getType());
	}

	bc::Baritem* item = (bcc.createBarcode(&src, constr));
	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto line = item->barlines[i];
		const auto& matr = line->matr;

		for (const auto& pm : matr)
		{
			int x = pm.getX();
			int y = pm.getY();

			auto rscal = src.get(x, y);
			Trainer<keylen>::Hash h;
			h.value = (rscal.getAvgFloat() - lower) / ludiff;

			if (useImageAsNoise)
			{
				cells.set(x, y, h.value);
			}
			else if (halfRand)
			{
				if (distrib(gen) >= 80)
					cells.set(x, y, h.value);
			}

			bool skipThis = false;
			for (buchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= src.width() || yi < 0 || yi >= src.height())
				{
					if (skipNotFull)
					{
						skipThis = true;
						break;
					}
					h.key[j] = 0;
					continue;
				}

				auto srcscalr = src.get(xi, yi);
				h.key[j] = (srcscalr.getAvgFloat() - lower) / ludiff;
			}

			if (!skipThis)
				train.add(h);
		}
	}


	if (!useImageAsNoise)
	{

		BackImage& outn = srcNoise->mat;
		float ludiffas = ludiff / upper;
		for (int i = 0; i < cells.length(); i++)
		{
			float colorProc = cells.getLiner(i);
			if (colorProc > 1.0)
				colorProc = 1.0;
			else if (colorProc < 0)
				colorProc = 0;

			Barscalar newColor = imgmin + diffcolor * colorProc;
			outn.setLiner(i, newColor);
		}
	}

	train.train();

	ProcField newCells(cells);
	//distrib(gen)

	std::cout << "Generating" << std::endl;
	int offset = 0;
	for (int k = 0; k < step; k++)
	{
		std::cout << "Step " << k + 1 << "/" << step << "..." << std::endl;
		for (int i = offset; i < cells.length(); i++)
		{
			int x = i % cells.width;
			int y = i / cells.width;

			// bc::barline* line = bar.get(x, y);
			bool skip = false;
			HashK<keylen> h;
			for (buchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
				{
					h.key[j] = 0;
					skip = true;
					break;
				}

				float srcscalr = cells.get(xi, yi);
				h.key[j] = srcscalr;
			}
			if (skip)
				continue;
			// h.set();

			int id = train.getCloser(h);
			auto* line = train.linesCollector[id].get();
			float val = line->chanceValue.getChanceValue(gen);

			float& newCell = newCells.get(x, y);
			// newCell += abs(newCell - val) * adj;
			newCell = val;
			if (debugDraw)
				std::cout << "id> " << id << "(" << val << "->" << newCell << ")" << std::endl;

			// for (int j = 0; j < keylen; j++)
			// {
			// 	val = train.nodeCollector[line.nodesId[j]]->edge;
			// 	int xi = x + poss[j][0];
			// 	int yi = y + poss[j][1];
			// 	float cell = cells.get(xi, yi);
			// 	float& newCell = newCells.get(xi, yi);

			// 	newCell += (val - cell) * adj;
			// }
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

	{ // Restore
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
	OptionValue comp4("DebugDraw", false);
	OptionValue comp5("HalfRand", false);
	OptionValue comp6("SkipNotFull", false);
	OptionValue comp7("UseImageAsNoise", false);
	settings.values.push_back(comp2);
	settings.values.push_back(comp3);
	settings.values.push_back(comp4);
	settings.values.push_back(comp5);
	settings.values.push_back(comp6);
	settings.values.push_back(comp7);

	return settings;
}


volatile static AlgFuncRegister registerExeGenColor("Generate Closest Color", exeGenColor, mkSettingsTypeColor, "Diffusion");
