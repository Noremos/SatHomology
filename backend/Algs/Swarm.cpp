
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Usings.h"
#include <random>


#include "../MatrImg.h"
#include "../MLSettings.h"

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

class Swarm : public CellBarcode
{
public:
	bc::Baritem* run(const bc::DatagridProvider* src, bc::barstruct& str, float& startEnergy)
	{
		settings = str;
		init(src, str.proctype);
		field.resize(bc::BarcodeCreator::totalSize);

		neck.resize(workingImg->length());
		std::fill(neck.begin(), neck.end(), 0.5);
		for (size_t i = 0; i < 10; i++)
		{
			std::fill(field.begin(), field.end(), nullptr);
			comps.clear();
			runClassic(startEnergy);
		}
		maxEnergy = *std::max_element(neck.begin(), neck.end());


		int iwid = workingImg->wid();
		bc::Baritem* itm = new bc::Baritem(iwid, type);

		static std::vector<Barscalar> colors;
		if (colors.size() == 0)
		{
			// srand(300);
			colors.push_back(Barscalar(255, 0, 0));
			colors.push_back(Barscalar(0, 0, 0));
			colors.push_back(Barscalar(0, 255, 0));
			colors.push_back(Barscalar(0, 0, 255));
			for (int k = 0; k < 40; ++k)
				colors.push_back(Barscalar(5 + rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

			colors.push_back(Barscalar(255, 255, 255));
		}

		int cl = 0;

		for (auto& c : comps)
		{
			// comm
			if (c->parent)
				continue;

			if (c->dead)
				continue;

			auto* line = new bc::barline(workingImg->get(c->pixels[0]), workingImg->get(c->pixels.back()));
			line->initRoot(itm);
			for (auto& p : c->pixels)
			{
				// line->matr.push_back(bc::barvalue(p, {neck[p.y * iwid + p.x], BarType::FLOAT32_1}));
				line->matr.push_back(bc::barvalue(p, colors[cl]));
				cl = (++cl % colors.size());
			}
		}

		return itm;
	}
private:
	std::vector<double> neck;
	virtual void runClassic(float& maxEnergy)
	{
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
				std::sort(found.begin(), found.end(), [](Cmp* a, Cmp* b){ return a->getAvg() > b->getAvg(); });
				for (size_t i = 1; i < found.size(); i++)
				{
					float avg = found[i]->getAvg();
					for (auto& p : found[i]->pixels)
					{
						auto& neckCur = neck[p.getLiner(iwid)];

						neckCur += 1. / avg;
					}
					found[i]->parent = found[i-1];
					found[i-1]->dead = true;
				}

				found.resize(1);
			}
			else if (found.size() == 0)
			{
				comps.push_back(std::make_unique<Cmp>());
				field[lin] = comps.back().get();
				field[lin]->pixels.push_back(curpix);
				field[lin]->energy += high.getAvgFloat();
				continue;
			}

			if (found.size() == 1)
			{
				auto& neckCur = neck[curpoindex];

				// Generate random float value with new c++ 14 classes
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> dis(0, neckCur + 1 / found[0]->getAvg());
				float randf = dis(gen);
				if (randf > neckCur)
				{
					found[0]->pixels.push_back(curpix);
					found[0]->energy += high.getAvgFloat();;
				}
			}
		}

	}
};



RetLayers exeSwarm(InOutLayer iol, const MLSettings& setting)
{
	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);

	BackImage& out = rasterSpot->mat;

	Swarm bcc;
	float dummy = 0;
	bc::barstruct constr = getConstr(setting);
	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
	// bc::Baritem* item = containner.get();

	bc::Baritem* item = (bcc.run(&src, constr, dummy));


	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		const auto& matr = item->barlines[i]->matr;

		// BackSize maskSize = b;
		for (const auto& pm : matr)
		{
			// out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat() / dummy));
			out.set(pm.getX(), pm.getY(), pm.value);
			//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());
		}
	}

	return ret;
}

volatile static AlgFuncRegister registerWorms("Swarm", exeSwarm, mkSettingsType);
