
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Barcode/PrjBarlib/include/barscalar.h"
#include "Usings.h"
#include <random>


#include "../MatrImg.h"
#include "../MLSettings.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"
#include "../backend/project.h"
#include "../backend/Interfaces/IAlgorithm.h"
#include "../backend/Algs/SimpleBar.h"

// import LayersCore;
// import RasterLayers;
//import BackBind;
//import IAlgorithm
// import MLSettings;
// import MatrModule;
//import IAlgorithm
//import AlgUtils;
//import SimpleBar;

// Linear interpolation function

class Swarm2 : public CellBarcode
{
public:
	bc::Baritem* run(const bc::DatagridProvider* src, bc::barstruct& str, float& startEnergy)
	{
		settings = str;
		init(src, str.proctype);
		field.resize(bc::BarcodeCreator::totalSize);

		int iwid = workingImg->wid();
		neck.resize(workingImg->length());
		std::fill(neck.begin(), neck.end(), 0.5);
		for (size_t i = 0; i < 10; i++)
		{
			std::fill(field.begin(), field.end(), nullptr);
			comps.clear();
			runClassic(startEnergy);

			for (auto& c : comps)
			{
				// comm
				double avg = c->getAvg();
				for (auto& p : c->pixels)
				{
					neck[p.y * iwid + p.x] += (1.0 / c->getAvg());
				}
			}

			// !for
		}
		maxEnergy = *std::max_element(neck.begin(), neck.end());


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

			struct Chanse{
				int j;
				double change;
			};
			std::vector<Chanse> chanses;
			float totlaChanse = 0;

			for (buchar j = 0; j < 8; ++j)
			{
				const bc::point IcurPoint2(curpix + poss[j]);
				if (IS_OUT_OF_REG(IcurPoint2.x, IcurPoint2.y))
					continue;

				totlaChanse += neck[IcurPoint2.getLiner(iwid)];
				chanses.push_back({ (int)j, neck[IcurPoint2.getLiner(iwid)] });
			}

			auto& neckCur = neck[curpoindex];

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<double> dis(0, totlaChanse);
			double randf = dis(gen);

			double acum = 0;
			for (size_t i = 0; i < chanses.size(); i++)
			{
				acum += chanses[i].change;
				if (randf < acum)
				{
					const bc::point IcurPoint2(curpix + poss[chanses[i].j]);

					Cmp* temp = getCmp(IcurPoint2.getLiner(iwid));
					if (temp != nullptr)
					{
						temp->pixels.push_back(curpix);
						temp->energy += high.getAvgFloat();;
					}
					else
					{
						comps.push_back(std::make_unique<Cmp>());
						auto* p = comps.back().get();
						p->pixels.push_back(curpix);
						p->energy += high.getAvgFloat();
					}

					break;
				}
			}
		}

	}
};



RetLayers exeResize(InOutLayer iol, const MLSettings& setting)
{
	Project* proj = Project::getProject();
	IRasterLayer* input = proj->getInRaster(iol);

	int resolution = *setting.getInt("resolution");

	RetLayers ret;
	BackImage src;

	BackSize inputSize(input->getCachedImage()->width(), input->getCachedImage()->height());

	float aspect = static_cast<float>(inputSize.wid) / inputSize.hei;

	inputSize.wid = aspect * resolution;
	inputSize.hei = 1.0 / aspect * resolution;


	RasterLayer* rasterSpot = genOutputFromInput(iol, src, inputSize);
	ret.push_back(rasterSpot);


	bc::BarcodeCreator bcc;
	float dummy = 0;
	bc::barstruct constr = getConstr(setting);
	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
	// bc::Baritem* item = containner.get();

	if (!*setting.getBool("withBar"))
	{
		rasterSpot->mat = src;
		return ret;
	}

	BackImage& out = rasterSpot->mat;
	bc::Baritem* item = (bcc.createBarcode(&src, constr));
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
	delete item;

	return ret;
}
MLSettings mkResizeSettings()
{
	MLSettings settings =  mkSettingsType();
	OptionValue comp("resolution", (int)2);
	settings.values.push_back(comp);

	OptionValue comp2("withBar", (bool)true);
	settings.values.push_back(comp2);


	return settings;
}

volatile static AlgFuncRegister registerResize("Resize", exeResize, mkResizeSettings);




RetLayers exeRecolor(InOutLayer iol, const MLSettings& setting)
{
	Project* proj = Project::getProject();
	IRasterLayer* input = proj->getInRaster(iol);

	int resolution = *setting.getInt("resolution");

	RetLayers ret;
	BackImage src;

	RasterLayer* rasterSpot = genOutputFromInput(iol, src);
	ret.push_back(rasterSpot);


	bc::BarcodeCreator bcc;
	float dummy = 0;
	bc::barstruct constr = getConstr(setting);
	// std::unique_ptr<bc::Baritem> containner(bcc.run(&src, constr, dummy));
	// bc::Baritem* item = containner.get();


	Barscalar minColor;
	Barscalar maxColor;
	src.maxAndMin(minColor, maxColor);
	Barscalar diff = maxColor - minColor;

	Barscalar part = diff / resolution;

	for (int h = 0; h < src.height(); ++h)
	{
		for (int i = 0; i < src.width(); ++i)
		{
			auto cur = src.get(i, h);
			auto colro = cur - cur % part;
			src.set(i, h, colro);
		}
	}

	if (!*setting.getBool("withBar"))
	{
		rasterSpot->mat = src;
		return ret;
	}


	BackImage& out = rasterSpot->mat;
	bc::Baritem* item = (bcc.createBarcode(&src, constr));
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
	delete item;

	return ret;
}

volatile static AlgFuncRegister registerRecolor("Recolor", exeRecolor, mkResizeSettings);
