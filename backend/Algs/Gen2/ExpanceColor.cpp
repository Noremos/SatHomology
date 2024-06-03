#include "GenCommon.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Barcode/PrjBarlib/include/barscalar.h"
#include <random>
#include "Common.h"


#include "../../MatrImg.h"
#include "../../Clusterizers/MLSettings.h"

import LayersCore;
import RasterLayers;
//import BackBind;
import ProjectModule;
// import MLSettings;
// import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;


constexpr int keylen = 8;
constexpr static char poss[keylen][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 }};


enum CellState
{
	SourceColor = 0,
	Expance = 0
};


struct ExCell
{
	CellState state = SourceColor;
	float color;
};

struct ExpnField : public Field<ExCell>
{
public:
	ExpnField(int w, int h) : Field(w, h)
	{
	}

	void fillRandom(BarType type)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, 255);
		std::uniform_int_distribution<> distrib2(0, 1);
		if (type == BarType::BYTE8_3)
		{
			for (auto& c : field)
			{
				// c.color = Barscalar(distrib(gen), distrib(gen), distrib(gen));
				c.state = (CellState)distrib2(gen);
			}
		}
		else
		{
			for (auto& c : field)
			{
				// c.color = Barscalar(distrib(gen),type);
				c.state = (CellState)distrib2(gen);
			}
		}
	}
};


RetLayers exeGenColorPos(InOutLayer iol, const MLSettings& setting)
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
	bc::Baritem* item = (bcc.createBarcode(&src, constr));

	ExpnField cells(src.width(), src.height());
	cells.fillRandom(out.getType());

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

	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto line = item->barlines[i];
		const auto& matr = line->matr;

		// BackSize maskSize = b;
		for (const auto& pm : matr)
		{
			auto srcscalr = src.get(pm.getX(), pm.getY());
			cells.get(pm.getX(), pm.getY()).color = (srcscalr.getAvgFloat() - lower) / ludiff;
			// out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat() / dummy));
			//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());
		}
	}


	for (int s = 0; s < step; s++)
	{
		ExpnField newCells(cells);
		for (int i = 0; i < cells.length(); i++)
		{
			int x = i % cells.width;
			int y = i / cells.width;

			ExCell& cell = cells.get(x, y);

			float avgc = 0;
			Barscalar maxcol(0, BarType::BYTE8_3);
			int cee = 0;
			std::vector<int> toChange;
			for (uchar j = 0; j < keylen; ++j)
			{
				int xi = x + poss[j][0];
				int yi = y + poss[j][1];

				if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
					continue;

				auto& pcell = cells.get(xi, yi);

				if (pcell.state == Expance)
				{
					++cee;
					toChange.push_back(j);
					avgc += pcell.color;
				}
			}

			if (cee >= 4)
			{
				newCells.get(x, y).state = Expance;
				newCells.get(x, y).color = avgc / cee;

				for (int j : toChange)
				{
					int xi = x + poss[j][0];
					int yi = y + poss[j][1];
					auto& cl = newCells.get(xi, yi);
					cl.state = SourceColor;
				}
			}
		}

		cells = newCells;
	}

	float ludiffas = ludiff / upper;
	for (int i = 0; i < cells.length(); i++)
	{
	 	float colorProc = cells.getLiner(i).color;
		if (colorProc > 1.0)
			colorProc = 1.0;
		else if (colorProc < 0)
			colorProc = 0;

		Barscalar newColor = imgmin + diffcolor * colorProc;
		out.setLiner(i, newColor);
	}
	return ret;
}


MLSettings mkSettingsTypeColor2()
{
	MLSettings settings = mkSettingsType();

	OptionValue comp2("step", 1);
	OptionValue comp3("aje", 0.01);
	settings.values.push_back(comp2);
	settings.values.push_back(comp3);

	return settings;
}

static AlgFuncRegister registerExeGenColor2("ExpanceColor2", exeGenColorPos, mkSettingsTypeColor2, "Diffusion");
