#include "../Algs/Gen2/GenCommon.h"
#include "../Algs/Gen2/Trainer.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"

import RasterLayers;
import IBlock;
import LayersCore;
import ProjectModule;
import AlgUtils;
import BackBind;
import MatrModule;

constexpr int keylen = 9;
constexpr static char poss[keylen][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 }, {0,0}};


struct ProcField : public Field<float>
{
public:
	ProcField(int w, int h) : Field(w, h)
	{
	}

	ProcField(BackSize size) : Field(size.wid, size.hei)
	{
	}

	ProcField(const ProcField& other) : Field(other.width, other.height)
	{
		std::copy(other.field.begin(), other.field.end(), field.begin());
	}

	void operator=(ProcField&& other)
	{
		field = std::move(other.field);
	}

	void operator=(const ProcField& other)
	{
		field = other.field;
	}

	void fillRandom()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distrib(0, 1.0);

		for (auto& c : field)
		{
			c = distrib(gen);
			// std::fill_n(c.pos, 8, 0);
		}
	}

	void setRandom(size_t i)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> distrib(0, 1.0);
		field[i] = distrib(gen);
	}
};

class MinMax
{
public:
	MinMax(BackImage& src) : MinMax()
	{
		// src.maxAndMin(imgmin, imgmax);
		// if (imgmin > imgmax)
		// {
		// 	std::swap(imgmin, imgmax);
		// }

		// diffcolor = imgmax - imgmin;

		// lower = imgmin.getAvgFloat();
		// upper = imgmax.getAvgFloat();
		// ludiff = upper - lower;
	}

	MinMax()
	{
		imgmin = Barscalar(0,0,0);
		imgmax = Barscalar(255,255,255);

		diffcolor = imgmax - imgmin;

		lower = imgmin.getAvgFloat();
		upper = imgmax.getAvgFloat();
		ludiff = upper - lower;
	}

	float getValue(const Barscalar& val) const
	{
		return (val.getAvgFloat() - lower) / ludiff;
	}

	Barscalar getColor(float colorProc) const
	{
		if (colorProc > 1.0)
			colorProc = 1.0;
		else if (colorProc < 0)
			colorProc = 0;

		Barscalar newColor = imgmin + diffcolor * colorProc;
		return newColor;
	}

	void restoreImage(BackImage& out, const ProcField& cells)
	{
		for (int i = 0; i < cells.length(); i++)
		{
			float colorProc = cells.getLiner(i);
			Barscalar newColor = getColor(colorProc);
			out.setLiner(i, newColor);
		}
	}

private:
	Barscalar diffcolor;
	Barscalar imgmin;
	Barscalar imgmax;
	float lower;
	float upper;
	float ludiff;
};

class GenColorBlock : public IBlock
{
	int step = 1;
	// float adj;
	bool debugDraw = false;
	int noiseProcent = 80;
	bool skipNotFull = true;
	bool useImageAsNoise = false;
	int swid = 30, shei = 30;

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
		};
	}

	virtual const BackString name() const override
	{
		return "Generate Color";
	}

	virtual RetLayers execute(InOutLayer iol) override
	{
		if (useImageAsNoise)
			return generateFromSource(iol);
		else
			return generateFromNoise(iol);
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

	void trainByImage(BackImage& src, const MinMax& mm)
	{
		bc::BarcodeCreator bcc;
		bc::barstruct constr;// = getConstr(setting);
		std::cout << "Creating barcode..." << std::endl;
		bc::Baritem* item = (bcc.createBarcode(&src, constr));

		std::cout << "Training..." << std::endl;
		for (size_t i = 0; i < item->barlines.size(); ++i)
		{
			auto line = item->barlines[i];
			const auto& matr = line->matr;

			for (const auto& pm : matr)
			{
				int x = pm.getX();
				int y = pm.getY();

				auto rscal = src.get(x, y);
				Trainer<keylen>::Hash h;
				h.value = mm.getValue(rscal);

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

					auto srcscalr = src.get(xi, yi);
					h.key[j] = mm.getValue(srcscalr);
				}

				if (!skipThis)
					train.add(h);
			}
		}
	}

	void diffuse(ProcField& cells, const MinMax& mm)
	{
		train.train();
		if (train.linesCollector.empty())
			return;

		std::random_device rd;
		std::mt19937 gen(rd());

		ProcField newCells(cells);
		std::cout << "Generating..." << std::endl;
		int offset = 0;
		for (int k = 0; k < step; k++)
		{
			std::cout << "Step " << k + 1 << "/" << step << "..." << std::endl;
			for (int i = offset; i < cells.length(); i++)
			{
				int x = i % cells.width;
				int y = i / cells.width;

				bool skip = false;
				HashK<keylen> h;
				for (buchar j = 0; j < keylen; ++j)
				{
					int xi = x + poss[j][0];
					int yi = y + poss[j][1];
					if (xi < 0 || xi >= cells.width || yi < 0 || yi >= cells.height)
					{
						h.key[j] = 0;
						skip = true;
						break;
					}

					float srcscalr = cells.get(xi, yi);
					h.key[j] = srcscalr;
				}

				if (skip)
					continue;

				int id = train.getCloser(h);
				auto* line = train.linesCollector[id].get();
				float val = line->value.get(gen);

				float& newCell = newCells.get(x, y);
				// newCell += abs(newCell - val) * adj;
				newCell = val;
				if (debugDraw)
					std::cout << "id> " << id << "(" << val << "->" << newCell << ")" << std::endl;
			}
			cells = newCells;
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
		diffuse(cells, mm);

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
		diffuse(cells, mm);

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
