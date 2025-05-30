// module;
#pragma once
#include <memory>

#include <functional>
#include <vector>
#include <assert.h>
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Common.h"


#include "../../side/Barcode/PrjBarlib/modules/StateBinFile.h"
#include "../Bind/MHashMap.h"
#include "../Interfaces/IItem.h"
#include "../../Bind/Framework.h"
#include "ClassInterface.h"
#include "../TrainData.h"
// export module Classifiers;

// import BarcodeModule;
// import StateBinIO;
//import BackBind;
// import IItemModule;
// //import ClassifierInterface;
#include "../backend/Classifer/ClassInterface.h"

// import Platform;

// //import TrainIO;
#include "../backend/TrainData.h"
// import MHashMap;


MEXPORT class BarlineClass : public IClassItem
{
public:
	// int id;
	bc::barline* line = nullptr;
	int depth = 0;
	int matrSize = 0;
	bc::barlinevector childer;

	// IClassItem* parent;
	BarlineClass(bc::barline* line = nullptr) : line(line)
	{
		if (line)
		{
			depth = line->getDeath();
			matrSize = line->getMatrSize();
			update();
		}
	}

	virtual size_t getId() const override
	{
		return line->id;
	}

	virtual size_t getParentId() const override
	{
		return line->parentId;
	}

	virtual int getDeath() const override
	{
		return depth;
	}

	virtual Barscalar start() const override
	{
		return line->start;
	}

	virtual Barscalar end() const override
	{
		return line->end();
	}

	virtual const bc::barvector& getMatrix() const override
	{
		return line->matr;
	}

	virtual const size_t getMatrixSize() const override
	{
		return matrSize;
	}

	void update()
	{
		line->getChilredAsList(childer, true, true, false);
	}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		if (state->isReading())
			line = new bc::barline();

		state->beginItem();
		state->pType(line->start.type); // Only set

		line->start = state->pBarscalar(line->start);
		line->m_end = state->pBarscalar(line->end());
		matrSize = state->pInt(matrSize);
		depth = state->pInt(depth);

		int csize = childer.size();
		csize = state->pInt(csize);
		state->beginArray(childer, csize);
		for (size_t i = 0; i < csize; i++)
		{
			if (state->isReading())
				childer[i] = new bc::barline();

			auto cline = childer[i];

			cline->start = state->pBarscalar(cline->start);
			cline->m_end = state->pBarscalar(cline->end());
		}
	}

	~BarlineClass()
	{
		for (size_t i = 0; i < childer.size(); i++)
		{
			delete childer[i];
		}
	}
};


MEXPORT template <typename TClassItem>
class AntBaritemHolder : public IDataClassItemHolder<TClassItem>
{
	using Base = IDataClassItemHolder<TClassItem>;
protected:

	std::shared_ptr<bc::Baritem> item;
public:

	void create(bc::DatagridProvider* img, const bc::barstruct& constr, const Base::ItemCallback& callback) override
	{
		bc::BarcodeCreator creator;
		item.reset(creator.createBarcode(img, constr));

		int size = (int)item->barlines.size();
		for (int i = 0; i < size; i++)
		{
			TClassItem id(item->barlines[i]);
			callback(&id);
		}
	}

	//void create(bc::DatagridProvider* img, const bc::BarConstructor& constr)
	//{
	//	bc::BarcodeCreator creator;
	//	std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));

	//	item.reset(ret->exractItem(0));

	//	int size = item->barlines.size();
	//	for (int i = 0; i < size; i++)
	//	{
	//		IClassItem* id = new BarlineClass(item->barlines[i]);
	//		items.push_back(id);
	//	}
	//}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		const bool isReading = state->isReading();

		if (isReading)
			item.reset(new bc::Baritem());

		bc::barlinevector& vec = item->barlines;

		// index = state->pInt(index); // Index
		item->setType((BarType)state->pType(item->getType())); // BarType
		size_t linesCount = state->pArray(static_cast<buint>(vec.size()));

		// Parent read/write stuff
		const bool use32index = state->pInt(sizeof(bc::BIndex) == 4 ? 1 : 0) == 1;

		// Begin
		state->beginArray(vec, linesCount);
		vec.resize(linesCount);

		for (size_t i = 0; i < linesCount; ++i)
		{
			bc::barline* line;
			if (!isReading)
			{
				// Write
				line = vec[i];

				// buint couId = line->id;
				state->pInt(line->id);
				state->pInt(line->parentId);
			}
			else
			{
				// Read

				// Main line
				buint couId = state->pInt(0);
				line = new bc::barline();
				line->id = couId;
				line->root = item.get();

				buint parentId = state->pInt(0);
				auto* parentLine = vec[line->parentId];
				if (parentLine == nullptr)
				{
					auto parline = new bc::barline();
					parentLine = parline;
					parline->id = parentId;
				}

				parentLine->addChild(line);
			}

			line->start = state->pBarscalar(line->start);
			line->m_end = state->pBarscalar(line->end());

			//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
			buint matrSize = state->pArray(line->matr.size());
			state->beginArray(line->matr, matrSize);

			if (use32index)
			{
				for (buint j = 0; j < matrSize; ++j)
				{
					bc::barvalue& v = line->matr[j];
					v.setX(state->pShort(v.getX()));
					v.setY(state->pShort(v.getY()));
					v.value = state->pBarscalar(v.value);
				}
			}
			else
			{
				for (buint j = 0; j < matrSize; ++j)
				{
					bc::barvalue& v = line->matr[j];
					v.setX(state->pInt(v.getX()));
					v.setY(state->pInt(v.getY()));
					v.value = state->pBarscalar(v.value);
				}
			}


			if (isReading)
			{
				TClassItem* id = new TClassItem(line);
				Base::items.push_back(id);
			}
		}
		state->endItem();
	}
};

MEXPORT class BaritemHolder : public AntBaritemHolder<BarlineClass>
{
public:
	IClassItem* getItem(size_t id)
	{
		return items[id];
	}

	const IClassItem* getItem(size_t id) const
	{
		return items[id];
	}
};

MEXPORT class barclassificator : public TCacheClassifier<BarlineClass, bc::Barcontainer>
{
public:
	const BackString name() const
	{
		return "CLASSIC";
	}

	void prepareBeforeAddInner(BarlineClass *raw)
	{
		bc::Baritem item;
		item.barlines = std::move(raw->childer);
		item.relength();
		item.setType();
		raw->childer = std::move(item.barlines);
	}

	void addToContainer(bc::Barcontainer& container, BarlineClass* raw)
	{
		bc::Baritem* item = new bc::Baritem();
		item->barlines = std::move(raw->childer);
		item->setType();
		container.addItem(item);
	}

	int predictInner(const BarlineClass* raw)
	{
		bc::Baritem newOne;
		raw->line->getChilredAsList(newOne.barlines, true, true, false);
		newOne.relength();

		auto cp = bc::CompareStrategy::CommonToLen;
		float res = 0;

		int maxInd = -1;
		float maxP = res;
		for (size_t i = 0; i < classes.size(); i++)
		{
			float ps = classes[i]->container.compireBest(&newOne, cp);
			if (ps > maxP)
			{
				maxP = ps;
				maxInd = i;
			}
		}

		//		bar0.barlines.clear();
		//		delete bar0;

		assert(maxP <= 1.0);
		return maxP > 0.5 ? maxInd : -1;
	}
};


MEXPORT class BettylineClass : public IClassItem
{
public:
	// int id;
	bc::barline* line = nullptr;
	int depth = 0;
	int matrSize = 0;
	std::array<int, 256> betty;

	BettylineClass(bc::barline* iline = nullptr) : line(iline)
	{
		if (iline)
		{
			depth = line->getDeath();
			// matrSize = line->getMatrSize();
			matrSize = line->matr.size();
			update();
		}
	}

	virtual size_t getId() const override
	{
		return line->id;
	}

	virtual size_t getParentId() const override
	{
		return line->parentId;
	}

	virtual int getDeath() const override
	{
		return depth;
	}

	virtual Barscalar start() const override
	{
		return line->start;
	}

	virtual Barscalar end() const override
	{
		return line->end();
	}

	virtual const bc::barvector& getMatrix() const override
	{
		return line->matr;
	}

	virtual const size_t getMatrixSize() const override
	{
		return matrSize;
	}

	void update()
	{
		betty = line->getBettyNumbers();
	}

	virtual void saveLoadState(StateBinFile::BinState* state) override
	{
		if (state->isReading())
			line = new bc::barline();

		state->beginItem();
		state->pType(line->start.type); // Only set

		line->start = state->pBarscalar(line->start);
		line->m_end = state->pBarscalar(line->end());
		matrSize = state->pInt(matrSize);
		depth = state->pInt(depth);

		for (short i = 0; i < 255; i++)
		{
			betty[i] = state->pInt(betty[i]);
		}
	}
};


MEXPORT class BettyItemHolder : public AntBaritemHolder<BettylineClass>
{
public:

	void operator=(BettyItemHolder&& o)
	{
		// move
		item = std::move(o.item);
		items = std::move(o.items);
	}

	IClassItem* getItem(size_t id)
	{
		return items[id];
	}

	const IClassItem* getItem(size_t id) const
	{
		return items[id];
	}
};


struct BettyVal
{
	int vals[256];
	int offset;

	BettyVal(const int* const input, int off)
	{
		memcpy(vals, input, 256 * sizeof(int));
		offset = off;
	}

	// Computes the cosine similarity between two arrays
	float compaire(const int* const ovals, int off)
	{
		size_t dotProduct = 0;
		size_t magA = 0;
		size_t magB = 0;
		for (unsigned short i = 0; i < 256; i++)
		{
			// int v = (i - offset) < 0 ? 0 : vals[i - offset];
			// int o = (i - off) < 0 ? 0 : ovals[i - off];
			int v = vals[i];
			int o = ovals[i];
			dotProduct += v * o;
			magA += v * v;
			magB += o * o;
		}
		// magA = sqrt(magA);
		// magB = sqrt(magB);
		return static_cast<double>(dotProduct) / (sqrt(magA) * sqrt(magB));
	}
};

class BattyClass
{
	std::vector<BettyVal*> data;

public:
	void add(const int* const src, int off)
	{
		data.push_back(new BettyVal(src, off));
	}

	BettyVal* exractItem(int i)
	{
		BettyVal* val = data[i];
		data[i] = nullptr;
		return val;
	}

	size_t count() const
	{
		return data.size();
	}

	float compire(const int* const other, int off)
	{
		float maxc = 0;
		// int maxId = 0;
		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i] == nullptr)
				continue;

			float locd = data[i]->compaire(other, off);
			// sum += locd;
			if (locd > maxc)
			{
				maxc = locd;
				// maxId = i;
			}
		}

		return maxc;
		// return sum / data.size();
	}

	~BattyClass()
	{
		for (size_t i = 0; i < data.size(); ++i)
		{
			delete data[i];
		}
		data.clear();
	}
};


MEXPORT class BettyClassificator : public TCacheClassifier<BettylineClass, BattyClass>
{
public:
	const BackString name() const
	{
		return "BETTY";
	}

	TClassData* getClass(int id)
	{
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i]->classId == id)
			{
				return classes[i].get();
			}
		}

		throw;
	}

	void prepareBeforeAddInner(BettylineClass*)
	{
	}

	void addToContainer(BattyClass& container, BettylineClass* raw)
	{
		short st = raw->line->start.getAvgUchar();
		short ed = raw->line->end().getAvgUchar();
		if (st > ed)
			st = ed;

		container.add(raw->betty.data(), st);
	}

	int predictInner(const BettylineClass* newOne)
	{
		short st = newOne->line->start.getAvgUchar();
		short ed = newOne->line->end().getAvgUchar();
		if (st > ed)
			st = ed;

		float res = 0;

		int maxInd = -1;
		float maxP = res;
		for (size_t i = 0; i < classes.size(); i++)
		{
			float ps = classes[i]->container.compire(newOne->betty.data(), st);
			if (ps > maxP)
			{
				maxP = ps;
				maxInd = i;
			}
		}

		assert(maxP <= 1.0);
		return maxP > 0.7 ? classes[maxInd]->classId : -1;
	}
};
