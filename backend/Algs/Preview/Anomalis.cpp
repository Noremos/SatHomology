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


#include "../../MatrImg.h"
#include "../../MLSettings.h"


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

RetLayers findAnomalies(InOutLayer iol, const MLSettings& setting)
{
	int size = *setting.getInt("max size");
	int length = *setting.getInt("min length");
	bool remove = *setting.getBool("remove");

	RetLayers ret;
	BackImage src;

	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);

	BackImage& out = rasterSpot->mat = src;

	bc::BarcodeCreator bcc;
	bc::barstruct constr = getConstr(setting);

	bc::Baritem* item = (bcc.createBarcode(&src, constr));
	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		auto line = item->barlines[i];
		const auto& matr = line->matr;
		if (matr.size() <= size && line->len() >= length)
		{
			for (const auto& pm : matr)
			{
				int x = pm.getX();
				int y = pm.getY();

				if (remove)
					out.set(x, y, out.get(x, y) - pm.value);
				else
					out.set(x, y, Barscalar(255,0,0));
			}
		}
	}
	return ret;
}

MLSettings anomalySettings()
{
	MLSettings settings = mkSettingsType();

	OptionValue comp1("min length", 100);
	OptionValue comp2("max size", 5);
	OptionValue comp3("remove", false);
	settings.values.push_back(comp1);
	settings.values.push_back(comp2);
	settings.values.push_back(comp3);

	return settings;
}

static AlgFuncRegister registerExeGenColor3("findAnomalies", findAnomalies, anomalySettings, "Preview");
