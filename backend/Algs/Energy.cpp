
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Usings.h"

import LayersCore;
import RasterLayers;
import BackBind;
import ProjectModule;
import MLSettings;
import MatrModule;
import IAlgorithm;
import AlgUtils;

// Linear interpolation function
Barscalar lerp(double t)
{
	if (t < 0)
		t = 0;

	assert(t <= 1.0 && t >= 0);
	//t = log(1 + t);
	const BackColor start(0, 255, 255);
	const BackColor end(255, 0, 0);
	uint8_t r = static_cast<uint8_t>(start.r + t * (end.r - start.r));
	uint8_t g = static_cast<uint8_t>(start.g + t * (end.g - start.g));
	uint8_t b = static_cast<uint8_t>(start.b + t * (end.b - start.b));
	return Barscalar(r, g, b);
}


class EnetrgyBarcode : public bc::BarcodeCreator
{
public:
	float* run(const bc::DatagridProvider* src, bc::barstruct& str, float& startEnergy)
	{
		settings = str;
		init(src, str.proctype);
		switch (str.proctype)
		{
		case bc::ProcType::Radius:
			return runGeo(startEnergy);
			break;
		default:
			return runClassic(startEnergy);
		}
	}

private:
	virtual float* runGeo(float& startEnergy)
	{
		float* energy = new float[workingImg->length()];
		memset(energy, 0, workingImg->length() * sizeof(float));

		auto* get = geometrySortedArr.get();
		int iwid = workingImg->wid();
		for (curIndexInSortedArr = 0; curIndexInSortedArr < processCount; ++curIndexInSortedArr)
		{
			const bc::indexCov& val = get[curIndexInSortedArr];
			auto curpoindex = val.offset;
			float& left = energy[curpoindex];

			auto curpix = getPoint(curpoindex);
			bc::point NextPoint = val.getNextPoint(curpix);
			auto NextPindex = NextPoint.getLiner(iwid);
			float& right = energy[NextPindex];

			if (left == 0)
			{
				if (right == 0)
				{
					left = startEnergy;
					right = startEnergy;
				}
				else
				{
					// 1
					//left = right / 2;
					//right /= 2;

					// 2
					left = right / 2;

				}
			}
			else
			{
				right = left / 2;
				//left /= 2;
			}
		}

		return energy;
	}

	virtual float* runClassic(float& startEnergy)
	{
		float* energy = new float[workingImg->length()];
		memset(energy, 0, workingImg->length() * sizeof(float));

		// 3
		startEnergy = 0;


		int iwid = workingImg->wid();
		bc::poidex* indexarr = sortedArr.get();
		for (curIndexInSortedArr = 0; curIndexInSortedArr < processCount; ++curIndexInSortedArr)
		{
			auto  curpoindex = indexarr[curIndexInSortedArr];

			auto curpix = getPoint(curpoindex);

			bool found = false;
			float& cur = energy[curpoindex];
			static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };
			for (buchar i = 0; i < 8; ++i)
			{
				const bc::point IcurPoint(curpix + poss[i]);

				if (IS_OUT_OF_REG(IcurPoint.x, IcurPoint.y))
					continue;

				const bc::poidex IcurPindex = IcurPoint.getLiner(iwid);
				float& side = energy[IcurPindex];
				if (side != 0)
				{
					found = true;
					// 1
					//cur = side / 2;
					//side /= 2;

					// 2
					//cur = side / 2;

					// 3
					//cur = side + 1;

					// 4
					//++side;
					++cur;
					if (cur > startEnergy)
						startEnergy = cur;

					//if (cur == 9)
					//	cur = 1;

					if (side > startEnergy)
						startEnergy = side;
				}
			}

			if (!found)
				cur = 1;
		}

		return energy;
	}
};
//



struct Cmp
{
	bool marked = false;
	bool dead = false;
	Cmp* parent = nullptr;
	Cmp* cachedMaxParent = nullptr;
	Cmp* getMaxParent()
	{
		if (parent == nullptr)
			return this;

		if (cachedMaxParent == nullptr)
			cachedMaxParent = parent;

		while (cachedMaxParent->parent)
		{
			cachedMaxParent = cachedMaxParent->parent;
		}
		return cachedMaxParent;
	}

	float energy = 0;
	std::vector<bc::point> pixels;
};

class CellBarcode : public bc::BarcodeCreator
{
protected:
	float maxEnergy = 0;
	std::vector<std::unique_ptr<Cmp>> comps;
	std::vector<Cmp*> field;
public:
	bc::Baritem* run(const bc::DatagridProvider* src, bc::barstruct& str, float& startEnergy)
	{
		settings = str;
		init(src, str.proctype);
		field.resize(bc::BarcodeCreator::totalSize);
		std::fill(field.begin(), field.end(), nullptr);
		switch (str.proctype)
		{
		//case bc::ProcType::Radius:
		//	return runGeo(startEnergy);
		//	break;
		default:
			runClassic(startEnergy);
		}

		int iwid = workingImg->wid();
		bc::Baritem* itm = new bc::Baritem(iwid, type);
		for (auto& c : comps)
		{
			// comm
			if (c->parent)
				continue;

			if (c->dead)
				continue;

			if (c->energy > maxEnergy)
				maxEnergy = c->energy;
		}

		for (auto& c : comps)
		{
			// comm
			if (c->parent)
				continue;

			if (c->dead)
				continue;

			auto* line = new bc::barline(workingImg->get(c->pixels[0]), workingImg->get(c->pixels.back()));
			line->initRoot(itm);
			itm->barlines.push_back(line);
			for (auto& p : c->pixels)
			{
				line->matr.push_back(bc::barvalue(p, Barscalar(c->energy / maxEnergy, BarType::FLOAT32_1)));
			}
		}

		return itm;
	}

protected:
	/*float runGeo(float& startEnergy)
	{
		return nullptr;
	}*/

	Cmp* getCmp(bc::poidex pid)
	{
		Cmp* c = field[pid];
		if (c)
			c = c->getMaxParent();
		else
			return nullptr;

		return c->dead ? nullptr : c;
	}

	virtual void runClassic(float& startEnergy)
	{
		int iwid = workingImg->wid();
		bc::poidex* indexarr = sortedArr.get();
		for (curIndexInSortedArr = 0; curIndexInSortedArr < processCount; ++curIndexInSortedArr)
		{
			auto curpoindex = indexarr[curIndexInSortedArr];

			auto curpix = getPoint(curpoindex);
			auto lin = curpix.getLiner(iwid);
			Barscalar high = workingImg->getLiner(lin);

			bool found = false;
			static char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };

			bc::point minPoint;
			Cmp* first = nullptr;
			Barscalar minDiff = high;
			for (buchar j = 0; j < 8; ++j)
			{
				const bc::point IcurPoint2(curpix + poss[j]);
				if (IS_OUT_OF_REG(IcurPoint2.x, IcurPoint2.y))
					continue;

				Cmp* temp = getCmp(IcurPoint2.getLiner(iwid));
				if (temp == nullptr)
					continue;

				Barscalar nextHigh = workingImg->get(IcurPoint2.x, IcurPoint2.y);
				Barscalar tempDiff = nextHigh.absDiff(high);
				if (tempDiff < minDiff)
				{
					minPoint = IcurPoint2;
					first = temp;
					minDiff = tempDiff;
				}
			}

			if (first == nullptr)
			{
				comps.push_back(std::make_unique<Cmp>());
				field[lin] = comps.back().get();
				field[lin]->pixels.push_back(curpix);
				field[lin]->energy = startEnergy;
				continue;
			}

			// else
			first->pixels.push_back(curpix);
			first->energy += startEnergy;
			first->energy -= minDiff.getAvgFloat();
			if (first->energy <= 0)
			{
				first->dead = true;
				first = nullptr;
			}

			for (buchar i = 0; i < 8; ++i)
			{
				const bc::point IcurPoint(curpix + poss[i]);

				if (IS_OUT_OF_REG(IcurPoint.x, IcurPoint.y))
					continue;

				const bc::poidex IcurPindex = IcurPoint.getLiner(iwid);
				Cmp* second = getCmp(IcurPindex);
				if (!second || second == first)
					continue;

				Barscalar nextHigh = workingImg->getLiner(IcurPindex);
				Barscalar minDiff = nextHigh.absDiff(high);
				for (buchar j = i + 1; j < 8; ++j)
				{
					const bc::point IcurPoint2(curpix + poss[j]);
					if (IS_OUT_OF_REG(IcurPoint2.x, IcurPoint2.y))
						continue;


					const bc::poidex IcurPindex2 = IcurPoint2.getLiner(iwid);
					if (getCmp(IcurPindex2) == second)
					{
						nextHigh = workingImg->get(IcurPoint2.x, IcurPoint2.y);
						Barscalar temp = nextHigh.absDiff(high);
						if (temp < minDiff)
							minDiff = temp;
					}
				}
				found = true;
				if (first)
				{
					if (first->energy < second->energy)
					{
						auto* temp = second;
						second = first;
						first = temp;
					}


					second->parent = first;
					second->energy -= minDiff.getAvgFloat();
					first->energy += second->energy;

					// Comm
					first->pixels.insert(first->pixels.end(), second->pixels.begin(), second->pixels.end());
					second->pixels.clear();

					if (first->energy <= 0)
					{
						first->dead = true;
						first = nullptr;
					}
				}
			} //
		}
	}
};

class Worms : public EnetrgyBarcode
{
	std::vector<std::unique_ptr<Cmp>> comps;
	std::vector<Cmp*> field;
private:
	std::vector<float> neck;
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
				std::sort(found.begin(), found.end(), [](Cmp * a, Cmp * b){ return a->energy / a->pixels.size() > b->energy / b->pixels.size(); });
				for (size_t i = 1; i < found.size(); i++)
				{
					float avg = found[i]->energy / found[i]->pixels.size();
					for (auto& p : found[i]->pixels)
					{
						neck[p.getLiner(iwid)] += avg;
						if (maxEnergy < neck[p.getLiner(iwid)])
							maxEnergy = neck[p.getLiner(iwid)];
					}
					found[i]->parent = found[i-1];
					found[i-1]->dead = true;
				}
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

		return neck;
	}
};

RasterLayer* getSrcFromInput(InOutLayer iol , BackImage& out)
{
	Project* proj = Project::getProject();
	IRasterLayer* input = proj->getInRaster(iol);

	const BackImage& srcl = *(input->getCachedImage());
	const BackSize srcsize(srcl.width(), srcl.height());
	float aspect = 1.f;

	out = BackImage(srcsize.wid, srcsize.hei, input->getRect(0, 0, 1, 1).get(0, 0).type);
	if (input->realWidth() != srcsize.wid)
	{
		aspect = static_cast<float>(input->realWidth()) / srcsize.wid;
		for (int h = 0; h < srcsize.hei; ++h)
		{
			for (int i = 0; i < srcsize.wid; ++i)
			{
				out.set(i, h, input->getRect(i * aspect, h * aspect, 1, 1).get(0, 0));
			}
		}
	}
	else
		out = srcl;

	RasterLayer* rasterSpot = proj->addLayerData<RasterLayer>(input->cs.getProjId());
	rasterSpot->initCSFrom(input->cs);
	rasterSpot->aspect = aspect;
	rasterSpot->init(srcsize.wid, srcsize.hei, 3);

	return rasterSpot;
}

bc::barstruct getConstr(const MLSettings& setting)
{
	bc::ProcType type = setting.getEnumValue<bc::ProcType>("type");
	const int* energyStart = setting.getInt("energyStart");

	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = false;
	constr.attachMode = bc::AttachMode::morePointsEatLow;
	//constr.attachMode = bc::AttachMode::closer;
	constr.returnType = bc::ReturnType::barcode2d;
	if (energyStart)
		constr.energyStart = *energyStart;

	constr.addStructure(type, bc::ColorType::native, bc::ComponentType::Component);

	return constr;
}

RetLayers exeEnergyCells(InOutLayer iol, const MLSettings& setting)
{
	float energyStart = *setting.getInt("energyStart");

	RetLayers ret;
	BackImage src;
	RasterLayer* rasterSpot = getSrcFromInput(iol, src);
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
	RasterLayer* rasterSpot = getSrcFromInput(iol, src);
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
	RasterLayer* rasterSpot = getSrcFromInput(iol, src);
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



static AlgFuncRegister registerEnergyCells("exeEnergyCells", exeEnergyCells, mkSettingsTypeEnergy);
static AlgFuncRegister registerEnergy("exeEnergy", exeEnergy, mkSettingsTypeEnergy);
static AlgFuncRegister registerWorms("Worms", exeWorms, mkSettingsType);
