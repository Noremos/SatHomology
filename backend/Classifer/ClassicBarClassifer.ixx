module;
#include <memory>
#include <random>
#include <iostream>
#include <functional>
#include <vector>
#include <assert.h>

#include "../../Bind/Common.h"

export module Classifiers;

import BarcodeModule;
import ClassifierInterface;

import IOCore;
import Platform;

import BarcodeModule;
import TrainIO;
import MHashMap;


export class BarlineClass : public IClassItem
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
			matrSize = line->matr.size();
			update();
		}
	}

	virtual size_t getId() const
	{
		return (size_t)line;
	}

	virtual size_t getParentId() const
	{
		return (size_t)line->parent;
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

	virtual const bc::barvector& getMatrix() const
	{
		return line->matr;
	}

	virtual const size_t getMatrixSize() const
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


export template <typename TClassItem>
class AntBaritemHolder : public IDataClassItemHolder<TClassItem>
{
	std::shared_ptr<bc::Baritem> item;
	using Base = IDataClassItemHolder<TClassItem>;
public:

	void create(bc::DatagridProvider* img, const bc::BarConstructor& constr, const Base::ItemCallback& callback)
	{
		bc::BarcodeCreator creator;
		std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(img, constr));

		item.reset(ret->exractItem(0));
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
		size_t linesCount = state->pArray(vec.size());

		// Parent read/write stuff
		uint counterId = 0;
		std::vector<bc::barline*> ids;
		ids.resize(linesCount * 2);
		std::fill(ids.begin(), ids.end(), nullptr);
		MMMAP<size_t, uint> writeIds;

		// Begin
		state->beginArray(vec, linesCount);
		for (size_t i = 0; i < linesCount; ++i)
		{
			bc::barline* line;
			if (!isReading)
			{
				// Write
				line = vec[i];

				uint couId;
				auto p = writeIds.find((size_t)line);
				if (p != writeIds.end())
				{
					couId = p->second;
				}
				else
				{
					couId = counterId++;
					writeIds.insert({ (size_t)line, couId });
				}

				state->pInt(couId);

				uint parId;
				p = writeIds.find((size_t)line->parent);
				if (p != writeIds.end())
				{
					parId = p->second;
				}
				else
				{
					parId = counterId++;
					writeIds.insert({ (size_t)line->parent, parId });
				}

				state->pInt(parId);
			}
			else
			{
				// Read

				// Main line
				uint couId = state->pInt(0);
				line = ids[couId];
				if (line == nullptr)
				{
					line = new bc::barline();
					ids[couId] = line;
				}
				vec[i] = line;

				// Parent
				uint parId = state->pInt(0);
				bc::barline* par = ids[parId];
				if (par == nullptr)
				{
					par = new bc::barline();
					ids[parId] = par;
				}

				line->setparent(par);
			}

			line->start = state->pBarscalar(line->start);
			line->m_end = state->pBarscalar(line->end());

			//act matrSize = state->pFixedArray(line->matr, 4 + typeSize);
			uint matrSize = state->pArray(line->matr.size());
			state->beginArray(line->matr, matrSize);
			for (uint j = 0; j < matrSize; ++j)
			{
				bc::barvalue& v = line->matr[j];
				v.index = state->pInt(v.index);
				v.value = state->pBarscalar(v.value);
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

export class BaritemHolder : public AntBaritemHolder<BarlineClass>
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

export class barclassificator : public TCacheClassifier<BarlineClass, bc::Barcontainer>
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
		item.relen();
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
		newOne.relen();

		auto cp = bc::CompireStrategy::CommonToLen;
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


export class BettylineClass : public IClassItem
{
public:
	// int id;
	bc::barline* line = nullptr;
	int depth = 0;
	int matrSize = 0;
	int betty[256];

	BettylineClass(bc::barline* iline = nullptr) : line(iline)
	{
		if (iline)
		{
			depth = line->getDeath();
			matrSize = line->matr.size();
			update();
		}
	}

	virtual size_t getId() const
	{
		return (size_t)line;
	}

	virtual size_t getParentId() const
	{
		return (size_t)line->parent;
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

	virtual const bc::barvector& getMatrix() const
	{
		return line->matr;
	}

	virtual const size_t getMatrixSize() const
	{
		return matrSize;
	}

	void update()
	{
		line->getBettyNumbers(betty);
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


export class BettyItemHolder : public AntBaritemHolder<BettylineClass>
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


struct BettyVal
{
	int vals[256];

	BettyVal(const int* const input)
	{
		memcpy(vals, input, 256 * sizeof(int));
	}

	// Computes the cosine similarity between two arrays
	float compaire(const int* const ovals)
	{
		double dotProduct = 0.0;
		double magA = 0.0;
		double magB = 0.0;
		for (unsigned short i = 0; i < 256; i++)
		{
			dotProduct += vals[i] * ovals[i];
			magA += vals[i] * vals[i];
			magB += ovals[i] * ovals[i];
		}
		magA = sqrt(magA);
		magB = sqrt(magB);
		return dotProduct / (magA * magB);
	}
};

class BattyClass
{
	std::vector<BettyVal*> data;

public:
	void add(const int* const src)
	{
		data.push_back(new BettyVal(src));
	}

	BettyVal* exractItem(int i)
	{
		BettyVal* val = data[i];
		data[i] = nullptr;
		return data[i];
	}

	size_t count()
	{
		return data.size();
	}

	float compire(const int* const other)
	{
		float maxc = 0;
		// int maxId = 0;
		for (size_t i = 0; i < data.size(); i++)
		{
			float locd = data[i]->compaire(other);
			if (locd > maxc)
			{
				maxc = locd;
				// maxId = i;
			}
		}

		return maxc;
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


export class BettyClassificator : public TCacheClassifier<BettylineClass, BattyClass>
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
		container.add(raw->betty);
	}

	int predictInner(const BettylineClass* newOne)
	{
		float res = 0;

		int maxInd = -1;
		float maxP = res;
		for (size_t i = 0; i < classes.size(); i++)
		{
			float ps = classes[i]->container.compire(newOne->betty);
			if (ps > maxP)
			{
				maxP = ps;
				maxInd = i;
			}
		}

		assert(maxP <= 1.0);
		return maxP > 0.5 ? maxInd : -1;
	}
};


void registerClassic()
{
	ClassFactory::RegisterFactory<BarlineClass, BaritemHolder, barclassificator>(1);
	ClassFactory::RegisterFactory<BettylineClass, BettyItemHolder, BettyClassificator>(2);
}
