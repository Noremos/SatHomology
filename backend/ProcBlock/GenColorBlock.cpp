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
};

class MinMax
{
public:
	MinMax(BackImage& src)
	{
		src.maxAndMin(imgmin, imgmax);
		if (imgmin > imgmax)
		{
			std::swap(imgmin, imgmax);
		}

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
	int step;
	float adj;
	bool debugDraw;
	bool halfRand;
	bool skipNotFull;
	bool useImageAsNoise;

	Trainer<keylen> train;
public:
	GenColorBlock()
	{
		IBlock::settings = {
			{"aje", adj},
			{"step", step},
			{"DebugDraw", debugDraw},
			{"HalfRand", halfRand},
			{"SkipNotFull", skipNotFull},
			{"UseImageAsNoise", useImageAsNoise}
		};
	}

	void addInput(InOutLayer iol) override
	{
		Project* proj = Project::getProject();
		addToTrain(iol);
		// proj->saveProject();
	}


	void addToTrain(InOutLayer iol)
	{
		BackImage src;

		getSrcFromInput(iol, src);
		RasterLayer* srcNoise;

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

	void restoreImage(BackImage& out, const ProcField& cells, const MinMax& mm)
	{
		for (int i = 0; i < cells.length(); i++)
		{
			float colorProc = cells.getLiner(i);
			Barscalar newColor = mm.getColor(colorProc);
			out.setLiner(i, newColor);
		}
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

	void generateByImage(ProcField& cells, const MinMax& mm)
	{
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
	}

	RetLayers generateFromNoise(InOutLayer iol, BackSize size)
	{
		RetLayers ret;

		BackImage src;
		RasterLayer* srcNoise = genOutputFromInput(iol, src);
		ret.push_back(srcNoise);
		MinMax mm(src);

		ProcField cells(size);
		cells.fillRandom();//out.getType());

		// Train
		trainByImage(src, mm);

		train.train();

		// Generate
		generateByImage(cells, mm);

		// Output
		Project* proj = Project::getProject();
		RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>();
		rasterSpot->aspect = 1.f;
		rasterSpot->init(size.wid, size.hei, 3);
		ret.push_back(rasterSpot);

		// Restore
		restoreImage(rasterSpot->mat, cells, mm);
		return ret;
	}

	RetLayers generateFromSource(InOutLayer iol)
	{
		RetLayers ret;
		BackImage src;

		RasterLayer* rasterSpot = genOutputFromInput(iol, src);
		BackImage& out = rasterSpot->mat;

		bc::BarcodeCreator bcc;
		bc::barstruct constr;// = getConstr(setting);

		MinMax mm(src);

		ProcField cells(src.width(), src.height());

		// Train
		trainByImage(src, mm);

		{ // Generage noise
			std::random_device rd;
			std::default_random_engine genrd(0);
			// std::mt19937 gen(genrd());
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distrib(1, 100);
			for (int i = 0; i < src.length(); i++)
			{
				float val = mm.getValue(src.getLiner(i));
				if (halfRand)
				{
					if (distrib(gen) >= 80)
						cells.setLiner(i, val);
				}
				else
					cells.setLiner(i, val);
			}

			RasterLayer* noiseLayer = genOutputFromInput(iol, src);
			BackImage& outNoise = noiseLayer->mat;
			restoreImage(outNoise, cells, mm);
			ret.push_back(noiseLayer);
		}


		train.train();

		// Generate iamge
		generateByImage(cells, mm);

		// Restore
		ret.push_back(rasterSpot);
		restoreImage(out, cells, mm);
		return ret;
	}
};
