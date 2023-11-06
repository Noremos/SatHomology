module;

#include <cassert>
#include <functional>

#include "../../Bind/Common.h"

export module ClassifierInterface;
// import std.core;

import IOCore;
import Platform;

import BarcodeModule;

import MetadataIOCore;
import TrainIO;
import MHashMap;
import IItemModule;


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


export template<class TItem, class TContainer>
class TCacheClassifier : public IDataBarClassifier<TItem>
{
public:
	template<class TContainer>
	struct ClassData
	{
		int classId;
		TContainer container;
		MMMAP<size_t, int> cacheIndex;
	};

	using TClassData = ClassData<TContainer>;

	std::vector<std::unique_ptr<TClassData>> classes;

	const BackString name() const
	{
		return "CLASSIC";
	}

	void loadData(const BarCategories& categs)
	{
		ClassDataIO io;
		io.open(IDataBarClassifier<TItem>::dbPath);

		classes.clear();
		for (const BarClassCategor& categ : categs.categs)
		{
			addClass(categ.id);
			loadClassData(io, categ.id);
		}
	}

	void loadClassData(ClassDataIO& io, int classId)
	{
		ClassDataIO::TrainCallback cla = [this](int clId, vbuffer& buf, BackImage, size_t dbLocalId)
		{
			std::stringstream stream;
			stream.write(reinterpret_cast<const char*>(buf.data()), buf.size());
			stream.seekg(0, std::ios::beg);
			// std::istringstream stream(buf.data(), buf.size());

			TItem raw{};
			raw.read(stream); // Already Prepared
			addDataInner(clId, &raw, dbLocalId);
		};

		io.loadAll(cla, classId, ClassDataIO::LF_ALL);
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

	void addClass(int id)
	{
		auto nd = std::make_unique<TClassData>();
		nd->classId = id;
		classes.push_back(std::move(nd));
	}

	void removeClass(int id)
	{
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i]->classId == id)
			{
				classes.erase(classes.begin() + i);
				break;
			}
		}
	}

	virtual void addToContainer(TContainer& holder, TItem* raw) = 0;

	void addDataInner(int classInd, TItem* item, size_t dataId, bool extractLine = false)
	{
		assert(classes.size() != 0);
		auto* classHolder = getClass(classInd);
		classHolder->cacheIndex.insert(std::make_pair(dataId, classHolder->container.count()));
		addToContainer(classHolder->container, item);
	}

	bool removeDataInner(int classId, size_t id)
	{
		TClassData& classHolder = *getClass(classId);
		auto it = classHolder.cacheIndex.find(id);
		if (it != classHolder.cacheIndex.end())
		{
			delete classHolder.container.exractItem(it->second);
			classHolder.cacheIndex.erase(it);
			return true;
		}
		return false;
	}
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

	static int idCounter;

	template <typename Item, typename Holder, typename Classifier>
	static int RegisterFactory()
	{
		int id = idCounter++;
		itemCreators[id] = []() { return std::make_unique<Item>(); };
		holderCreators[id] = []() { return std::make_unique<Holder>(); };
		classifierCreators[id] = []() { return std::make_unique<Classifier>(); };

		return idCounter;
	}
};

int ClassFactory::idCounter = 0;

ClassFactory::FunctionHolder<IClassItem> ClassFactory::itemCreators;
ClassFactory::FunctionHolder<IClassItemHolder> ClassFactory::holderCreators;
ClassFactory::FunctionHolder<IBarClassifier> ClassFactory::classifierCreators;


export template<class TClass, class TClassHolder, class TClassifier>
class GlobalRegister
{
	int id;
public:
	GlobalRegister()
	{
		id = ClassFactory::RegisterFactory<TClass, TClassHolder, TClassifier>();
	}
	int getId()
	{
		return id;
	}
};
