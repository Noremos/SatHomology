module;
#include <vector>
#include <memory>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Usings.h"

export module EnergyModule;

// import BarcodeModule;
// import BarTypes;
import BackBind;

export class EnetrgyBarcode : public bc::BarcodeCreator
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
	float* runGeo(float& startEnergy)
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

	float* runClassic(float& startEnergy)
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

	float energy;
	std::vector<bc::point> pixels;
};

export class CellBarcode : public bc::BarcodeCreator
{
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

private:
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

	void runClassic(float& startEnergy)
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
