module;

#include <cassert>
#include "../../Bind/Common.h"

export module ClassifierInterface;
// import std.core;

import IOCore;
import Platform;

import BarcodeModule;

import MetadataIOCore;
import TrainIO;

export struct FilterInfo
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

		bool inRange(const Barscalar &val) const
		{
			return val >= first && val <= second;
		}

		bool notInRange(int val) const
		{
			return val < first || val > second;
		}

		bool notInRange(const Barscalar &val) const
		{
			return val < first && val > second;
		}
	};

	FRange start{0, 255};
	FRange len{0, 255};
	FRange matrSizeProc{0, 100};
	FRange depth{0, 1000};
	int imgLen = 0;

	// bool needSkip(bc::barline* line) const
	// {
	// 	return start.notInRange(line->start) ||
	// 		len.notInRange(line->len()) ||
	// 		matrSizeProc.notInRange(line->matr.size() * 100 / imgLen) ||
	// 		depth.notInRange(line->getDeath());
	// }
};

export class IClassItem : public IBffIO
{
public:
	virtual size_t getId() const = 0;
	virtual size_t getParentId() const = 0;
	// virtual const bc::barlinevector& getLines() const = 0;
	virtual const bc::barvector &getMatrix() const = 0;
	virtual const size_t getMatrixSize() const = 0;
	virtual int getDeath() const = 0;
	virtual Barscalar start() const = 0;
	virtual Barscalar end() const = 0;
	// virtual IClassItem* parent() const = 0;

	virtual bool passFilter(const FilterInfo &filter) const = 0;

	virtual ~IClassItem()
	{
	}
};

export class IClassItemHolder : public IBffIO
{
public:
	virtual const std::vector<IClassItem*> &getItems() const = 0;

	using ItemCallback = std::function<void(IClassItem *item)>;

	virtual void create(bc::DatagridProvider *img, const bc::BarConstructor &constr, const ItemCallback& callback) = 0;
};

export template<typename T>
class IDataClassItemHolder : public IClassItemHolder
{
protected:
	std::vector<T*> items;
public:
	virtual const std::vector<IClassItem*> &getItems() const
	{
		return static_cast<const std::vector<T*>&>(items);
	}

	virtual ~IDataClassItemHolder()
	{ }
};


export class ItemHolderCache
{
protected:
	std::unique_ptr<StateBinFile::BinState> state;

public:
	void openRead(const BackPathStr &str)
	{
		state.reset(new StateBinFile::BinStateReader());
		if (!state->open(str.string()))
			throw;
	}

	void openWrite(const BackPathStr &str)
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

	IClassItemHolder *load(int &index, IClassItemHolder *t)
	{
		assert(state->isReading());

		index = state->pInt(0);
		t->read(state.get());
		return t;
	}

	IClassItemHolder *loadSpecific(int index, IClassItemHolder *t)
	{
		assert(state->isReading());
		dynamic_cast<StateBinFile::BinStateReader *>(state.get())->moveIndex(index);

		state->beginItem();
		index = state->pInt(0);
		t->read(state.get());
		return t;
	}

	void save(IClassItemHolder *item, int index)
	{
		assert(!state->isReading());

		state->beginItem();
		index = state->pInt(index);
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

// Classifer

export class IBarClassifier
{
public:
	BackPathStr dbPath;

	void open(const BackPathStr& metaPath)
	{
		this->dbPath = metaPath / name();
	}

	virtual const BackString name() const = 0;

	virtual void loadData(const BarCategories &categs) = 0;
	virtual void addClass(int id) = 0;
	virtual void removeClass(int id) = 0;
	virtual void prepareBeforeAdd(IClassItem *raw) = 0;

	size_t addData(int classInd, IClassItem *raw, BackImage *icon, bool extract = false)
	{
		prepareBeforeAdd(raw);

		std::ostringstream st;
		raw->write(st);

		ClassDataIO io;
		io.openWrite(dbPath);
		vbuffer temp;
		auto str = st.str();
		char* data = str.data();
		temp.setData((uchar*)data, str.length(), false);
		size_t id = io.save(classInd, temp, icon);
		addDataInnerT(classInd, raw, id, extract);

		return id;
	}

	virtual void addDataInnerT(int classInd, IClassItem *raw, size_t dataId, bool extractLine = false) = 0;

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
	virtual int predict(const IClassItem *raw) = 0;

	virtual ~IBarClassifier()
	{
	}
};

export template<typename T>
class IDataBarClassifier : public IBarClassifier
{
public:
	virtual int predictInner(const T* raw) = 0;
	virtual int predict(const IClassItem *raw)
	{
		return predictInner(static_cast<const T*>(raw));
	}

	virtual void prepareBeforeAddInner(T *raw) = 0;
	virtual void prepareBeforeAdd(IClassItem *raw)
	{
		prepareBeforeAddInner(static_cast<T*>(raw));
	}

	virtual void addDataInner(int classInd, T *raw, size_t dataId, bool extractLine = false) = 0;
	virtual void addDataInnerT(int classInd, IClassItem* raw, size_t dataId, bool extractLine = false)
	{
		addDataInner(classInd, static_cast<T*>(raw), dataId, extractLine);
	}

	virtual ~IDataBarClassifier()
	{ }
};


export class ClassFactory
{
private:
	template<class Interface>
	using BaseCreator = std::function<std::unique_ptr<Interface>()>;

	// using ItemCreator = BaseCreator<IClassItem>;
	// using HolderCreator = BaseCreator<IClassItemHolder>;
	// using ClassifierCreator = BaseCreator<IBarClassifier>;

	template<class IF>
	using FunctionHolder = MMMAP<int, BaseCreator<IF>>;

	static FunctionHolder<IClassItem> itemCreators;
	static FunctionHolder<IClassItemHolder> holderCreators;
	static FunctionHolder<IBarClassifier> classifierCreators;

	template<class IF>
	static std::unique_ptr<IF> CreateItem(int id, FunctionHolder<IF>& creators)
	{
		auto it = creators.find(id);
		if (it != creators.end())
			return it->second();
		else
			return nullptr;
	}

public:
	static std::unique_ptr<IClassItem> CreateItem(int id)
	{
		return CreateItem<IClassItem>(id, itemCreators);
	}

	static std::unique_ptr<IClassItemHolder> CreateItemHolder(int id)
	{
		return CreateItem<IClassItemHolder>(id, holderCreators);
	}

	static std::unique_ptr<IBarClassifier> CreateClassifier(int id)
	{
		return CreateItem<IBarClassifier>(id, classifierCreators);
	}

	template <typename Item, typename Holder, typename Classifier>
	static void RegisterFactory(int id)
	{
		itemCreators[id] = []() { return std::make_unique<Item>(); };
		holderCreators[id] = []() { return std::make_unique<Holder>(); };
		classifierCreators[id] = []() { return std::make_unique<Classifier>(); };
	}
};

ClassFactory::FunctionHolder<IClassItem> ClassFactory::itemCreators;
ClassFactory::FunctionHolder<IClassItemHolder> ClassFactory::holderCreators;
ClassFactory::FunctionHolder<IBarClassifier> ClassFactory::classifierCreators;
