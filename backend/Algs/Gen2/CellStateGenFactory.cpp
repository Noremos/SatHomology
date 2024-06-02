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
#include "GenCommon.h"
#include "Common.h"


#include "../../MatrImg.h"


import LayersCore;
import RasterLayers;
//import BackBind;
import ProjectModule;
import MLSettings;
// import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;

// Linear interpolation function

struct Cell
{
	float proc = 1.f;
};

struct MM
{
	float l = 1.0, u = 1.0;
};


using CellField =  Field<Cell>;


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


RetLayers exeGenColor3(InOutLayer iol, const MLSettings& setting)
{
	float adj = *setting.getDouble("aje");
	int step = *setting.getInt("step");

	RetLayers ret;
	BackImage src;

	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
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


	CellField cells(src.width(), src.height());
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib2(0, 1);
	for (auto& u : cells.field)
	{
		u.proc = distrib2(gen);
	}

	CellField newCells(cells);

	int offset = 0;
	for (int k = 0; k < step; k++)
	{
		for (int i = 0; i < cells.length(); i++)
		{
			int x = i % cells.width;
			int y = i / cells.width;

			BackImage img(3, 3, 1, BarType::FLOAT32_1);
			for (uchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];
				float val = 0;
				if (!(xi < 0 || xi >= src.width() || yi < 0 || yi >= src.height()))
				{
					val = cells.get(x, y).proc;
				}

				img.set(1 + poss[j][0], 1 + poss[j][1], val);
			}

			bc::Baritem* item = (bcc.createBarcode(&img, constr));
			item->sortByLen();
			for (int o = 1; o < item->barlines.size(); o++)
			{
				const auto& matr = item->barlines[o]->matr;

				for (const auto& pm : matr)
				{
					int x = i % src.width();
					int y = i / src.width();
					newCells.get(x, y).proc -= pm.value.getAvgFloat();
				}
			}
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

	float ludiffas = ludiff / upper;
	for (int i = 0; i < cells.length(); i++)
	{
	 	float colorProc = cells.getLiner(i).proc;
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

MLSettings mkSettingsTypeColor3()
{
	MLSettings settings = mkSettingsType();

	OptionValue comp2("step", 1);
	OptionValue comp3("aje", 0.01);
	settings.values.push_back(comp2);
	settings.values.push_back(comp3);

	return settings;
}

static AlgFuncRegister registerExeGenColor3("CellGen", exeGenColor3, mkSettingsTypeColor3, "Diffusion");
