module;

#include <cassert>
#include "../../Bind/Common.h"

export module ClassifierInterface;
//import std.core;


import BarholdersModule;
import IOCore;
import Platform;
import CacheFilesModule;

import LayersCore;
import BarcodeModule;
import GeoprocessorModule;
import JsonCore;

import TrainIO;
import MetadataIOCore;


export struct FilterInfo
{
	struct FRange
	{
		int first;
		int second;

		//template<class T>
		//bool inRange(const T& val) const
		//{
		//	return val >= first && val <= second;
		//}

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

	FRange start{ 0,255 };
	FRange len{ 0,255 };
	FRange matrSizeProc{ 0,100 };
	FRange depth{ 0,1000 };
	int imgLen = 0;

	// bool needSkip(bc::barline* line) const
	// {
	// 	return start.notInRange(line->start) ||
	// 		len.notInRange(line->len()) ||
	// 		matrSizeProc.notInRange(line->matr.size() * 100 / imgLen) ||
	// 		depth.notInRange(line->getDeath());
	// }
};

export class IClassItem //: public IBffIO
{
public:
	virtual const bc::barlinevector& getLines() const = 0;
	virtual const bc::barvector& getMatrix() const = 0;
	virtual int getDeath() const = 0;
	virtual Barscalar start() const = 0;
	virtual Barscalar end() const = 0;
	virtual IClassItem* parent() const = 0;

	virtual bool passFilter(const FilterInfo& filter) const = 0;
};


export class ClassItemHolder
{
public:
	std::vector<IClassItem*> lines;

	~ClassItemHolder()
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			delete lines[var];
		}
		lines.clear();
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

	//void openRead(std::istringstream& str)
	//{
	//	state.reset(new StateBinFile::BinStateReader(str));
	//}

	//void openWrite(std::ostringstream& str)
	//{
	//	state.reset(new StateBinFile::BinStateWriter(str));
	//}

	IClassItem* load(int& index, IClassItem* t)
	{
		assert(state->isReading());

		index = state->pInt(0);
		//t->read(state.get());
		return t;
	}

	IClassItem* loadSpecific(int index, IClassItem* t)
	{
		assert(state->isReading());
		dynamic_cast<StateBinFile::BinStateReader*>(state.get())->moveIndex(index);

		index = state->pInt(0);
		//t->read(state.get());
		return t;
	}

	void save(IClassItem* item, int index)
	{
		assert(!state->isReading());
		index = state->pInt(index);
		//item->write(state.get());
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

// Classifer

export class IBarClassifier
{
public:
	BackPathStr dbPath;

	virtual const BackString& className() = 0;

	virtual void loadClasses(const BarCategories& categs, const BackPathStr& path) = 0;
	virtual void addClass(int id) = 0;
	virtual void removeClass(int id) = 0;

	size_t addData(int classInd, IClassItem* raw, BackImage* icon, bool extract = false)
	{
		addDataInner(classInd, raw, extract);
		std::ostringstream st;
		//raw->write(st);

		ClassDataIO io;
		io.openWrite(dbPath);
		vbuffer temp;
		temp.setData(reinterpret_cast<uchar*>(st.str().data()), st.str().length(), false);
		size_t id = io.save(classInd, temp, icon);
		return id;
	}

	virtual void addDataInner(int classInd, IClassItem* raw, bool extractLine = false) = 0;

	void removeData(int classId, size_t id)
	{
		if (removeDataInner(classId, id))
		{
			ClassDataIO io;
			io.openWrite(dbPath);
			io.remove(id);
		}
	}

	virtual bool removeDataInner(int classId, size_t id) = 0;
	virtual int predict(IClassItem* raw) = 0;
	virtual ~IBarClassifier() = 0;
};
