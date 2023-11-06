module;
#include "../../Bind/Common.h"
#include <assert.h>
#include <functional>

export module IItemModule;

import BarcodeModule;
import MetadataIOCore;

export class IClassItem : public IBffIO
{
public:
	virtual size_t getId() const = 0;
	virtual size_t getParentId() const = 0;
	// virtual const bc::barlinevector& getLines() const = 0;

	virtual const bc::barvector& getMatrix() const = 0;
	virtual const size_t getMatrixSize() const = 0;
	bc::barvalue getMatrValue(int id) const
	{
		return getMatrix()[id];
	}

	virtual int getDeath() const = 0;
	virtual Barscalar start() const = 0;
	virtual Barscalar end() const = 0;
	// virtual IClassItem* parent() const = 0;

	virtual ~IClassItem()
	{
	}
};


// Holder - a container to store items
export class IClassItemHolder : public IBffIO
{
public:
	virtual IClassItem* getItem(size_t id) = 0;
	virtual const IClassItem* getItem(size_t id) const = 0;
	virtual size_t getItemsCount() const = 0;

	using ItemCallback = std::function<void(IClassItem* item)>;

	virtual void create(bc::DatagridProvider* img, const bc::BarConstructor& constr, const ItemCallback& callback) = 0;
	virtual ~IClassItemHolder() {}
};

export template<typename T>
class IDataClassItemHolder : public IClassItemHolder
{
protected:
	std::vector<T*> items;

public:

	size_t getItemsCount() const
	{
		return items.size();
	}

	virtual ~IDataClassItemHolder()
	{
		for (size_t var = 0; var < items.size(); ++var)
		{
			delete items[var];
		}
		items.clear();
	}
};


export class ItemHolderCache
{
protected:
	std::unique_ptr<StateBinFile::BinState> state;

public:
	void openRead(const BackPathStr& str)
	{
		state.reset(new StateBinFile::BinStateReader());
		if (!state->open(str.string()))
			throw;
	}

	void openWrite(const BackPathStr& str)
	{
		state.reset(new StateBinFile::BinStateWriter());
		if (!state->open(str.string()))
			throw;
	}

	// void openRead(std::istringstream& str)
	//{
	//	state.reset(new StateBinFile::BinStateReader(str));
	// }

	// void openWrite(std::ostringstream& str)
	//{
	//	state.reset(new StateBinFile::BinStateWriter(str));
	// }

	int readIndex(int id)
	{
		assert(state->isReading());

		auto* reader = dynamic_cast<StateBinFile::BinStateReader*>(state.get());
		reader->moveIndex(id);
		state->beginItem();

		return reader->pInt(0);
	}

	int getItemsCount()
	{
		assert(state->isReading());

		auto* reader = dynamic_cast<StateBinFile::BinStateReader*>(state.get());
		return reader->getIndexSize();
	}

	IClassItemHolder* load(int& index, IClassItemHolder* t)
	{
		assert(state->isReading());

		index = state->pInt(0);
		t->read(state.get());
		return t;
	}

	IClassItemHolder* loadSpecific(int index, IClassItemHolder* t)
	{
		assert(state->isReading());
		auto* reader = dynamic_cast<StateBinFile::BinStateReader*>(state.get());

		size_t i = 0, total = reader->getIndexSize();
		for (; i < total; ++i)
		{
			reader->moveIndex(i);
			int rindex = state->pInt(0);
			if (rindex == index)
				break;
		}
		assert(i != total);

		state->beginItem();
		t->read(state.get());
		return t;
	}

	void save(IClassItemHolder* item, int index)
	{
		assert(!state->isReading());

		state->beginItem();
		state->pInt(index);
		item->write(state.get());
	}

	bool canRead() const
	{
		return !state->ended();
	}

	void close()
	{
		state.reset(nullptr);
	}
};


export struct IItemFilter
{
	int imgLen = 0;

	virtual bool pass(const IClassItem* line) const = 0;
};

export struct RangeItemFilter : public IItemFilter
{
	struct FRange
	{
		int first;
		int second;

		// template<class T>
		// bool inRange(const T& val) const
		//{
		//	return val >= first && val <= second;
		// }

		bool inRange(int val) const
		{
			return val >= first && val <= second;
		}

		bool inRange(const Barscalar& val) const
		{
			return val >= first && val <= second;
		}

		bool notInRange(int val) const
		{
			return val < first || val > second;
		}

		bool notInRange(const Barscalar& val) const
		{
			return val < first && val > second;
		}
	};

	FRange start{ 0, 255 };
	FRange len{ 0, 255 };
	FRange matrSizeProc{ 0, 100 };
	FRange depth{ 0, 1000 };

	int minPixelsSize = 0;

	bool pass(const IClassItem* line) const
	{
		size_t matrSize = line->getMatrixSize();
		auto linlen = line->end() - line->start();
		return start.inRange(line->start()) &&
			len.inRange(linlen) &&
			matrSizeProc.inRange(matrSize * 100 / imgLen) &&
			matrSize > minPixelsSize &&
			depth.inRange(line->getDeath());
	}


	//virtual bool passFilter(const IItemFilter& filter) const
	//{
	//	return filter.start.inRange(line->start) &&
	//		filter.len.inRange(line->len()) &&
	//		filter.matrSizeProc.inRange(line->matr.size() * 100 / filter.imgLen) &&
	//		filter.depth.inRange(line->getDeath());
	//}
};
