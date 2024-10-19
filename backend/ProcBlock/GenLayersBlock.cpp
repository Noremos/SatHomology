#include "DiffuseCommon.h"
#include "Common.h"
#include "../Interfaces/IBlock.h"

#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "../MatrImg.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"

// import RasterLayers;
// import IBlock;
// import LayersCore;
//import IAlgorithm
#include "../project.h"
#include "../Algs/Common.h"

//import AlgUtils;
//import BackBind;
// import MatrModule;

class GenLayersBlock : public IBlock
{
	int step = 1;
	// float adj;
	bool debugDraw = false;
	int noiseProcent = 80;
	bool useImageAsNoise = false;
	int swid = 30, shei = 30;


	std::vector<Trainer<keylen>> trains;
	MinMax globmm;
public:
	GenLayersBlock()
	{
		IBlock::settings = {
			// {"Aje", adj},
			{"Step", step},
			{"Size w", swid},
			{"Size h", shei},
			{"Debug Draw", debugDraw}
		};
	}

	virtual const BackString name() const override
	{
		return "Generate Layers";
	}

	virtual RetLayers execute(InOutLayer iol) override
	{
		return generateFromNoise(iol);
	}

	void clear() override
	{
		for (auto &train : trains)
		{
			train.clear();
		}
	}

	void addInput(InOutLayer iol) override
	{
		// Project* proj = Project::getProject();
		addToTrain(iol);
		// proj->saveProject();
	}


	void addToTrain(InOutLayer iol)
	{
		BackImage src;

		getSrcFromInput(iol, src);

		bc::BarcodeCreator bcc;
		bc::barstruct constr;// = getConstr(setting);
		constr.createGraph = true;

		MinMax mm(src);
		std::cout << "Training..." << std::endl;

		bc::Baritem* item = (bcc.createBarcode(&src, constr));
		for (size_t i = 0; i < item->barlines.size(); ++i)
		{
			auto line = item->barlines[i];
			const auto& matr = line->matr;
			int depth = line->getDeath();

			std::unordered_map<uint, size_t> map;

			for (size_t i = 0; i < matr.size(); i++)
			{
				auto& pm = matr[i];
				int x = pm.getX();
				int y = pm.getY();
				map.insert(std::pair(pm.getIndex(), i));
			}

			for (auto &pm : matr)
			{
				int x = pm.getX();
				int y = pm.getY();

				Trainer<keylen>::Hash h;
				h.value = mm.getValue(src.get(x, y));

				for (buchar j = 0; j < keylen; ++j)
				{
					int xi = x + poss[j][0];
					int yi = y + poss[j][1];
					uint sideid = bc::barvalue::getStatInd(xi, yi);
					auto it = map.find(sideid);
					if (it == map.end())
					{
						h.key[j] = 0;
						continue;
					}

					h.key[j] = mm.getValue(matr[it->second].value);
				}

				if (trains.size() <= depth)
					trains.resize(depth + 1);

				trains[depth].add(h);
			}
		}

		std::cout << "Done" << std::endl;
	}

	RetLayers generateFromNoise(InOutLayer iol)
	{
		Project* proj = Project::getProject();

		BackSize size(swid, shei);
		RetLayers ret;

		// Generate
		ProcField main(size);
		for (auto& train : trains)
		{
			ProcField cells(size);
			cells.fillRandom();//out.getType());

			diffuse(train, step, cells);
			main += cells;
		}

		// Output
		{
			RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>();
			rasterSpot->aspect = 1.f;
			rasterSpot->init(size.wid, size.hei, 3);
			ret.push_back(rasterSpot);
			MinMax mm;
			mm.restoreImage(rasterSpot->mat, main);
		}

		// Restore
		return ret;
	}
};

BlockRegister<GenLayersBlock> regblockGenLayers;
