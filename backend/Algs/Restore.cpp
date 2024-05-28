#include <algorithm>
#include <cmath>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Barcode/PrjBarlib/include/barscalar.h"
#include "Barcode/PrjBarlib/include/barstrucs.h"
#include <random>

import LayersCore;
import RasterLayers;
//import BackBind;
import ProjectModule;
import MLSettings;
import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;

// Linear interpolation function

RetLayers exeRestore(InOutLayer iol, const MLSettings& setting)
{
	RetLayers ret;
	BackImage src;

	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);


	bc::BarcodeCreator bcc;
	bc::barstruct constr = getConstr(setting);
	bc::Baritem* item = (bcc.createBarcode(&src, constr));

	const bool normal = constr.proctype != bc::ProcType::f255t0;

	Barscalar minColor;
	Barscalar maxColor;
	src.maxAndMin(minColor, maxColor);

	int wid = src.width();
	int hei = src.height();

	BackImage img(wid, hei, rasterSpot->mat.channels());
	for (size_t i = 0; i < wid * hei; i++)
	{
		if (normal)
			img.setLiner(i, maxColor);
		else
			img.setLiner(i, minColor);
	}

	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto line = item->barlines[i];
		const auto& matr = line->matr;
		for (const auto& pm : matr)
		{
			int x = pm.getX();
			int y = pm.getY();
			if (normal)
				img.minus(x, y, pm.value);
			else
				img.add(x, y, pm.value);
		}
	}
	rasterSpot->mat = img;

	return ret;
}


volatile static AlgFuncRegister registerExeRestore("Restore", exeRestore, mkSettingsType, "Preview");
