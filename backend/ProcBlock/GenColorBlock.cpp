
#include "DiffuseCommon.h"

#include "Barcode/PrjBarlib/include/barcodeCreator.h"
import RasterLayers;
import IBlock;
import LayersCore;
import ProjectModule;
import AlgUtils;
import BackBind;
import MatrModule;

class GenColorBlock : public IBlock
{
	int step = 1;
	// float adj;
	bool debugDraw = false;
	int noiseProcent = 80;
	bool skipNotFull = true;
	bool useImageAsNoise = false;
	int swid = 30, shei = 30;
	bool withputBar = false;

	Trainer<keylen> train;
	MinMax globmm;
public:
	GenColorBlock()
	{
		IBlock::settings = {
			// {"Aje", adj},
			{"Step", step},
			{"Size w", swid},
			{"Size h", shei},
			{"Debug Draw", debugDraw},
			{"Noise procent", noiseProcent},
			{"Skip Not Full", skipNotFull},
			{"Use Image As Noise", useImageAsNoise},
			{"Without barcode", withputBar}
		};
	}

	const BackString name() const override
	{
		return "Generate Color";
	}

	RetLayers execute(InOutLayer iol) override
	{
		if (useImageAsNoise)
			return generateFromSource(iol);
		else
			return generateFromNoise(iol);
	}

	void clear() override
	{
		train.clear();
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

		MinMax mm(src);
		std::cout << "Training..." << std::endl;

		if (withputBar)
		{
			std::cout << "Withput bar..." << std::endl;

			for (size_t i = 0; i < src.length(); ++i)
			{
				int x = i % src.width();
				int y = i / src.width();

				Trainer<keylen>::Hash h;
				h.value = mm.getValue(src.getLiner(i));

				bool skipThis = false;
				for (buchar j = 0; j < keylen; ++j)
				{
					int xi = x + poss[j][0];
					int yi = y + poss[j][1];
					if (xi < 0 || xi >= src.width() || yi < 0 || yi >= src.height())
					{
						if (skipNotFull)
						{
							skipThis = true;
							break;
						}
						h.key[j] = 0;
						continue;
					}

					h.key[j] = mm.getValue(src.get(xi, yi));
				}

				if (!skipThis)
					train.add(h);
			}
			std::cout << "Done" << std::endl;
			return;
		}

		std::cout << "With barcode..." << std::endl;

		bc::Baritem* item = (bcc.createBarcode(&src, constr));
		for (size_t i = 0; i < item->barlines.size(); ++i)
		{
			auto line = item->barlines[i];
			const auto& matr = line->matr;

			for (const auto& pm : matr)
			{
				int x = pm.getX();
				int y = pm.getY();

				Trainer<keylen>::Hash h;
				h.value = mm.getValue(src.get(x, y));

				bool skipThis = false;
				for (buchar j = 0; j < keylen; ++j)
				{
					int xi = x + poss[j][0];
					int yi = y + poss[j][1];
					if (xi < 0 || xi >= src.width() || yi < 0 || yi >= src.height())
					{
						if (skipNotFull)
						{
							skipThis = true;
							break;
						}
						h.key[j] = 0;
						continue;
					}

					h.key[j] = mm.getValue(src.get(xi, yi));
				}

				if (!skipThis)
					train.add(h);
			}
		}

		std::cout << "Done" << std::endl;
	}

	RetLayers generateFromNoise(InOutLayer iol)
	{
		Project* proj = Project::getProject();

		BackSize size(swid, shei);
		RetLayers ret;


		// Gen nosie
		ProcField cells(size);
		cells.fillRandom();//out.getType());

		// Create nosie layer
		RasterLayer* srcNoise = proj->addLayerData<RasterLayer>();
		srcNoise->aspect = 1.f;
		srcNoise->init(size.wid, size.hei, 3);
		MinMax mm;
		mm.restoreImage(srcNoise->mat, cells);
		ret.push_back(srcNoise);

		// Generate
		diffuse(train, step, cells);

		// Output
		{
			RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>();
			rasterSpot->aspect = 1.f;
			rasterSpot->init(size.wid, size.hei, 3);
			ret.push_back(rasterSpot);

			mm.restoreImage(rasterSpot->mat, cells);
		}

		// Restore
		return ret;
	}

	RetLayers generateFromSource(InOutLayer iol)
	{
		RetLayers ret;
		BackImage src;

		RasterLayer* noiseLayer = genOutputFromInput(iol, src);

		bc::BarcodeCreator bcc;
		bc::barstruct constr;// = getConstr(setting);

		MinMax mm(src);

		ProcField cells(src.width(), src.height());

		{ // Generage noise
			std::random_device rd;
			std::default_random_engine genrd(0);
			// std::mt19937 gen(genrd());
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(1, 100);
			for (int i = 0; i < src.length(); i++)
			{
				float val = mm.getValue(src.getLiner(i));
				if (distrib(gen) < noiseProcent)
					cells.setRandom(i);
				else
					cells.setLiner(i, val);
			}

			mm.restoreImage(noiseLayer->mat, cells);
			ret.push_back(noiseLayer);
		}


		// Generate iamge
		diffuse(train, step, cells);

		// Restore
		Project* proj = Project::getProject();
		RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>();
		{
			rasterSpot->aspect = 1.f;
			rasterSpot->init(src.width(), src.height(), 3);
			ret.push_back(rasterSpot);
		}

		mm.restoreImage(rasterSpot->mat, cells);
		return ret;
	}
};

BlockRegister<GenColorBlock> regblockGenColor;
