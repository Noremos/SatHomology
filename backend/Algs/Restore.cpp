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


#include "../MatrImg.h"
#include "../MLSettings.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"
#include "../CachedBarcode.h"

// import LayersCore;
// import RasterLayers;
//import BackBind;
import ProjectModule;
// import MLSettings;
// import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;

// Linear interpolation function

void restore(bc::Baritem* item, bool normal, BackImage& img, size_t start, size_t end, float hilight)
{
	item->sortByLen();
	double total = 0.01 * item->barlines.size();
	start = total * start;
	end = total * end;

	for (size_t i = start; i < end; ++i)
	{
		auto line = item->barlines[i];
		const auto& matr = line->matr;
		for (const auto& pm : matr)
		{
			int x = pm.getX();
			int y = pm.getY();
			if (normal)
				img.minusSafe(x, y, pm.value * hilight);
			else
				img.addSafe(x, y, pm.value * hilight);
		}
	}
}

void restore(bc::Baritem* item, bool normal, BackImage& img, const MLSettings& setting)
{
	size_t start = *setting.getInt("Slice start");
	size_t end = *setting.getInt("Slice end");
	float hilight = *setting.getDouble("K");

	restore(item, normal, img, start, end, hilight);
}

BackImage prepareImg(const bool normal, const BackImage& src, int channels = 3)
{
	Barscalar minColor;
	Barscalar maxColor;
	src.maxAndMin(minColor, maxColor);
	maxColor =maxColor.getAvgUchar();
	minColor =minColor.getAvgUchar();

	int wid = src.width();
	int hei = src.height();

	BackImage img(wid, hei, channels);
	for (size_t i = 0; i < wid * hei; i++)
	{
		if (normal)
			img.setLiner(i, maxColor);
		else
			img.setLiner(i, minColor);
	}

	return img;
}

void testRestore(InOutLayer iol, const MLSettings& setting)
{
	bc::barstruct constr = getConstr(setting);
	constr.coltype = bc::ColorType::gray;

	const bool normal = constr.proctype != bc::ProcType::f255t0;


	using namespace std::string_literals;
	BackString root = "/Users/sam/Edu/papers/2024_split/"s;

	std::vector<BackString> paths = {"1_commercial_area_231.jpg"s, "2_freeway_258.jpg"s, "3_ground_track_field_028.jpg"s, "4_lake_013.jpg"s};
	int starts[] =
	{
		5, 5, 5, 3
	};

	int middles[] =
	{
		75, 50, 35, 20
	};

	int middleK[] =
	{
		5, 5, 5, 5
	};

	int hightK[] =
	{
		50, 15, 15, 15
	};


	for (size_t i = 0; i < paths.size(); i++)
	{
		// break;
		int start = starts[i];
		int middle = middles[i];
		BackImage main = imread(root + paths[i]);

		std::unique_ptr<bc::Baritem> citem(bc::BarcodeCreator::create(main, constr));
		// citem->sortByLen();
		// restore(citem.get(), normal, main, 0, middle, 1.f);
		BackImage out = prepareImg(normal, main, 3);
		// restore(citem.get(), normal, out, middle, 100, 1);

		int k = middleK[i];
		{
			BackImage middleHilight = out;
			restore(citem.get(), normal, middleHilight, 0, start, 1.f);
			restore(citem.get(), normal, middleHilight, middle, 100, 1.f);
			restore(citem.get(), !normal, middleHilight, start, middle, k);


			imwrite(root + intToStr(i + 1) + "/rest + "s + intToStr(start) + "-"s + intToStr(middle) + "% * " + intToStr(k) + ".png"s, middleHilight);
		}

		k = hightK[i];
		{
			BackImage highHilight = out;
			restore(citem.get(), normal, highHilight, 0, start, 1.f);
			restore(citem.get(), normal, highHilight, start, middle, 1.f);
			restore(citem.get(), !normal, highHilight, middle, 100, k);

			imwrite(root + intToStr(i + 1) + "/rest + "s + intToStr(middle) + "-100% * "+ intToStr(k) +".png"s, highHilight);
		}

		// StateBinFile::BinStateWriter writer;
		// writer.open(root + intToStr(i + 1) + "/bar.bin"s);
		// auto count = writer.pInt(citem->barlines.size());
		// for (int i = 0; i < count; ++i)
		// {
		// 	writer.pShort(citem->barlines[i]->start.getInt());
		// 	writer.pShort(citem->barlines[i]->end().getInt());
		// }

	}
	return {};
}

RetLayers exeRestore(InOutLayer iol, const MLSettings& setting)
{
	// testRestore()
	bc::barstruct constr = getConstr(setting);
	constr.coltype = bc::ColorType::gray;

	const bool normal = constr.proctype != bc::ProcType::f255t0;



	BackImage src;
	RetLayers ret;

	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);
	auto auitem = (bc::BarcodeCreator::create(src, constr));
	bc::Baritem* item = auitem.get();



	Barscalar minColor;
	Barscalar maxColor;
	src.maxAndMin(minColor, maxColor);
	maxColor =maxColor.getAvgUchar();
	minColor =minColor.getAvgUchar();

	int wid = src.width();
	int hei = src.height();

	BackImage img = prepareImg(normal, src, rasterSpot->mat.channels());
	float hilight = *setting.getDouble("K");



	bool multiplyMode = *setting.getBool("Multiply mode");

	if (multiplyMode)
	{
		restore(item, normal, img, 0, *setting.getInt("Slice start"), 1.f);
		restore(item, !normal, img, *setting.getInt("Slice start"), *setting.getInt("Slice end"), hilight);
		restore(item, normal, img, *setting.getInt("Slice end"), 100, 1.f);
	}
	else
		restore(item, normal, img, setting);

	// restore(item, normal, img, setting);
	rasterSpot->mat = img;

	// {
	// 	double total = 0.01 * item->barlines.size();

	// 	size_t start = total * *setting.getInt("Slice start");
	// 	size_t end = total * *setting.getInt("Slice end");

	// 	item->sortByLen();
	// 	restore(item, normal, img, hilight, start, end);
	// }


	return ret;
}


MLSettings mkSettingsRestore()
{
	MLSettings settings = mkSettingsType();

	OptionValue::Flags flags = static_cast<OptionValue::Flags>(OptionValue::HOT_RELOAR | OptionValue::SLIDER);
	settings.values.push_back({"Slice start", 0, flags});
	settings.values.push_back({"Slice end", 100, flags});
	// settings.values.push_back({"Add end", 100, flags});
	settings.values.push_back({"K", 1.0, flags});
	settings.values.push_back({"Multiply mode", false, OptionValue::HOT_RELOAR});
	return settings;
}

static AlgFuncRegister registerExeRestore("Restore", exeRestore, mkSettingsRestore, "Preview");

// class AutoRun
// {
// public:
// 	AutoRun()
// 	{
// 		exeRestore({}, testRestore());
// 	}
// };
// static AutoRun autoee;
