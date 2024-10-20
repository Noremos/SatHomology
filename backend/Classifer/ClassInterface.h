// module;

#pragma once
#include <cassert>
#include <sstream>
#include <vector>
#include "Barcode/PrjBarlib/include/barcodeCreator.h"
#include "Usings.h"

#include "../MetadataIO.h"
#include "../MatrImg.h"


#include "../Bind/MHashMap.h"
#include "../backend/Interfaces/IItem.h"
#include "../Bind/Framework.h"
// export module ClassifierInterface;
// import std.core;

// import Platform;

// import BarcodeModule;

// //import BackTypes;
#include "../backend/BackTypes.h"
// //import TrainIO;
#include "../backend/TrainData.h"

#include "../BackTypes.h"
#include "../TrainData.h"

// import MHashMap;
// import IItemModule;
//import BackBind;
// import MatrModule;


// Classifer

MEXPORT class IBarClassifier
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
		ubuffer temp;
		auto str = st.str();
		char* data = str.data();
		temp.setData((buchar*)data, str.length(), false);
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

MEXPORT template<typename T>
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


MEXPORT template<class TItem, class TContainer>
class TCacheClassifier : public IDataBarClassifier<TItem>
{
public:
	struct ClassData
	{
		int classId;
		TContainer container;
		MMMAP<size_t, int> cacheIndex;
	};

	using TClassData = ClassData;

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
		ClassDataIO::TrainCallback cla = [this](int clId, ubuffer& buf, BackImage, size_t dbLocalId)
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


MEXPORT using ClassFactory = ImlFactory<IClassItem, IClassItemHolder, IBarClassifier>;

MEXPORT inline static ClassFactory classFactory;

MEXPORT template<class TClass, class TClassHolder, class TClassifier>
class GlobalClassRegister : public GlobalRegister<ClassFactory, TClass, TClassHolder, TClassifier>
{
public:
	GlobalClassRegister(std::string_view name = "") : GlobalRegister<ClassFactory, TClass, TClassHolder, TClassifier>(classFactory, name)
	{
		printf(" for class\n");
	}
};

//MEXPORT template<class TClass, class TClassHolder, class TClassifier>
//using GlobalClassRegister = GlobalRegister<TClass, TClassHolder, TClassifier, ClassFactory>;
