#include <cstddef>
#include <vector>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Barcode/PrjBarlib/include/barscalar.h"

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

RetLayers exeTestSizeInv(InOutLayer iol, const MLSettings& setting)
{
	int rstep = static_cast<int>(*setting.getInt("step")) ;
	// float procSize = 1.f -  static_cast<float>(rstep) / 100.f;

	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);

	BackImage& out = rasterSpot->mat;


	bc::BarcodeCreator bcc;
	bc::barstruct constr = getConstr(setting);
	bc::Baritem* item1 = (bcc.createBarcode(&src, constr));

	src.resize(src.width() - rstep, src.height() - rstep);

	bc::Baritem* item2 = (bcc.createBarcode(&src, constr));
	item1->sortByLen();
	item2->sortByLen();
	size_t i = 0, j = 0, k =0;
	for (; i < item1->barlines.size(); ++i)
	{
		auto line1 = item1->barlines[i];

		if (line1->len() <= rstep)
		{
			++k;
			continue;
		}

		//*****
		j++;
		continue;
		//*****

		if (j >= item2->barlines.size())
		{
			out.fill(Barscalar(255, 0, 0));
			return ret;
		}
		auto line2 = item2->barlines[j++];

		if (line1->start != line2->start || line1->m_end != line2->m_end)
		{
			out.fill(Barscalar(255, 0, 0));
			return ret;
		}
	}
	if (j == item2->barlines.size())
	{
		out.fill(Barscalar(0, 255, 0));
	}
	else
	{
		std::cout << j << " " << item2->barlines.size() << std::endl;
		out.fill(Barscalar(255, 0, 0));
	}


	return ret;
}

MLSettings mkSettingsTypeInvSize()
{
	MLSettings settings = mkSettingsType();

	OptionValue comp2("step", 1);
	settings.values.push_back(comp2);

	return settings;
}

volatile static AlgFuncRegister registerExeTestSizeInv("exeTestSizeInv", exeTestSizeInv, mkSettingsTypeInvSize, "Validate");


RetLayers exeTestColorInv(InOutLayer iol, const MLSettings& setting)
{
	int rstep = static_cast<int>(*setting.getInt("step")) ;
	// float procSize = 1.f -  static_cast<float>(rstep) / 100.f;

	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);

	BackImage& out = rasterSpot->mat;


	bc::BarcodeCreator bcc;
	bc::barstruct constr = getConstr(setting);
	bc::Baritem* item1 = (bcc.createBarcode(&src, constr));

	for (int w = 0; w < src.width(); ++w)
	{
		for (int h = 0; h < src.height(); ++h)
		{
			if (src.get(w, h) > 0)
			{
				Barscalar s = src.get(w, h);
				// if (s >= rstep)
				{
					src.set(w, h, s - s % rstep);
				}
			}
		}
	}


	bc::Baritem* item2 = (bcc.createBarcode(&src, constr));

	// item1->sortByLen();
	// item2->sortByLen();
	size_t i = 0, j = 0, k = 0;
	for (; i < item1->barlines.size(); ++i)
	{
		auto line1 = item1->barlines[i];

		if (line1->len() <= rstep)
		{
			++k;
			continue;
		}

		//*****
		j++;
		continue;
		//*****

		if (j >= item2->barlines.size())
		{
			out.fill(Barscalar(255, 0, 0));
			return ret;
		}
		auto line2 = item2->barlines[j++];

		auto l1s = (line1->start - line1->start % rstep);
		auto l1e = (line1->m_end - line1->m_end % rstep);
		auto l2s = (line2->start - line2->start % rstep);
		auto l2e = (line2->m_end - line2->m_end % rstep);
		if (l1s != l2s || l1e != l2e)
		{
			std::cout << i << " " << j - 1 << std::endl;
			out.fill(Barscalar(255, 0, 0));
			return ret;
		}
	}

	if (j == item2->barlines.size())
	{
		out.fill(Barscalar(0, 255, 0));
	}
	else
	{
		std::cout << j << " " << item2->barlines.size() << std::endl;
		out.fill(Barscalar(255, 0, 0));
	}


	return ret;
}

volatile static AlgFuncRegister registerExeTestColorInv("exeTestColorInv", exeTestColorInv, mkSettingsTypeInvSize, "Validate");
