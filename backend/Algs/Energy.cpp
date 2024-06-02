
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Usings.h"


#include "../MatrImg.h"

import LayersCore;
import RasterLayers;
//import BackBind;
import ProjectModule;
import MLSettings;
// import MatrModule;
import IAlgorithm;
import AlgUtils;
import SimpleBar;



class Worms : public EnetrgyBarcode
{
	std::vector<std::unique_ptr<Cmp>> comps;
	std::vector<Cmp*> field;
private:

	Cmp* getCmp(bc::poidex pid)
	{
		Cmp* c = field[pid];
		if (c)
			c = c->getMaxParent();
		else
			return nullptr;

		return c->dead ? nullptr : c;
	}

	virtual float* runClassic(float& maxEnergy)
	{
		float* neck = new float[workingImg->length()];
		memset(neck, 0, workingImg->length() * sizeof(float));
		field.resize(bc::BarcodeCreator::totalSize);
		std::fill(field.begin(), field.end(), nullptr);

		int iwid = workingImg->wid();
		bc::poidex* indexarr = sortedArr.get();
		for (curIndexInSortedArr = 0; curIndexInSortedArr < processCount; ++curIndexInSortedArr)
		{
			auto curpoindex = indexarr[curIndexInSortedArr];
			auto curpix = getPoint(curpoindex);
			auto lin = curpix.getLiner(iwid);

			Barscalar high = workingImg->getLiner(lin);

			static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };

			std::vector<Cmp*> found;
			bool foundMarked = false;
			for (buchar j = 0; j < 8; ++j)
			{
				const bc::point IcurPoint2(curpix + poss[j]);
				if (IS_OUT_OF_REG(IcurPoint2.x, IcurPoint2.y))
					continue;

				Cmp* temp = getCmp(IcurPoint2.getLiner(iwid));
				if (temp == nullptr)
					continue;

				// if (temp->marked)
				// 	foundMarked = true;

				found.push_back(temp);
			}

			if (found.size() >= 2)
			{
				std::sort(found.begin(), found.end(), [](Cmp * a, Cmp * b){ return a->getAvg() > b->getAvg(); });
				for (size_t i = 1; i < found.size(); i++)
				{
					float avg = found[i]->getAvg();
					for (auto& p : found[i]->pixels)
					{
						neck[p.getLiner(iwid)] += avg;
					}
					found[i]->parent = found[i-1];
					found[i-1]->dead = true;
				}

				found[0]->pixels.push_back(curpix);
				found[0]->energy += high.getAvgFloat();
			}
			else if (found.size() == 1)
			{
				found[0]->pixels.push_back(curpix);
				found[0]->energy += high.getAvgFloat();
			}
			else //if (found.size() == 0)
			{
				comps.push_back(std::make_unique<Cmp>());
				field[lin] = comps.back().get();
				field[lin]->pixels.push_back(curpix);
				field[lin]->energy += high.getAvgFloat();
				continue;
			}
		}

		// Find maxEnergy
		maxEnergy = *std::max_element(neck, neck + workingImg->length());
		return neck;
	}
};


RetLayers exeEnergyCells(InOutLayer iol, const MLSettings& setting)
{
	float energyStart = *setting.getInt("energyStart");

	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);


	//bool useEmbeded = false;

	BackImage& out = rasterSpot->mat;

	/*	bcc.maxe = energyStart;
		std::unique_ptr<bc::Barcontainer> containner(bcc.createBarcode(&src, constr));

		for (size_t i = 0; i < src.length(); i++)
		{
			out.setLiner(i, lerp(static_cast<float>(bcc.energy[i]) / static_cast<float>(bcc.maxe)));
		}*/

	CellBarcode bce;
	bc::barstruct constr = getConstr(setting);
	std::unique_ptr<bc::Baritem> containner(bce.run(&src, constr, energyStart));

	bc::Baritem* item = containner.get();
	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		const auto& matr = item->barlines[i]->matr;

		// BackSize maskSize = b;
		for (const auto& pm : matr)
		{
			out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat()));
			//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());
		}
	}

	return ret;
}


RetLayers exeEnergy(InOutLayer iol, const MLSettings& setting)
{
	bc::ProcType type = setting.getEnumValue<bc::ProcType>("type");
	float energyStart = *setting.getInt("energyStart");

	Project* proj = Project::getProject();

	IRasterLayer* input = proj->getInRaster(iol);

	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);

	CellBarcode bce;

	BackImage& out = rasterSpot->mat;

	EnetrgyBarcode eb;
	bc::barstruct constr = getConstr(setting);
	float* outenergy = eb.run(&src, constr, energyStart);
	for (size_t i = 0; i < src.length(); i++)
	{
		out.setLiner(i, lerp(outenergy[i] / static_cast<float>(energyStart)));
	}
	delete[] outenergy;

	return ret;
}



RetLayers exeWorms(InOutLayer iol, const MLSettings& setting)
{
	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);

	BackImage& out = rasterSpot->mat;

	Worms bcc;
	float dummy = 0;
	bc::barstruct constr = getConstr(setting);
	float* outenergy = bcc.run(&src, constr, dummy);
	for (size_t i = 0; i < src.length(); i++)
	{
		out.setLiner(i, lerp(outenergy[i] / static_cast<float>(dummy)));
	}
	delete[] outenergy;

	return ret;
}


MLSettings mkSettingsTypeEnergy()
{
	MLSettings settings = mkSettingsType();
	OptionValue enr("energyStart", 100);
	settings.values.push_back(enr);
	return settings;
}



volatile static AlgFuncRegister registerEnergyCells("exeEnergyCells", exeEnergyCells, mkSettingsTypeEnergy);
volatile static AlgFuncRegister registerEnergy("exeEnergy", exeEnergy, mkSettingsTypeEnergy);
volatile static AlgFuncRegister registerWorms("Worms", exeWorms, mkSettingsType);
