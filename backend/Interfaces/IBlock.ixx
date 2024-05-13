module;
#include <cassert>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Usings.h"

export module IBlock;

import BackBind;
import ProjectSettings;
import LayersCore;
import RefSettings;
// import BarTypes;
//export class IClasterItem
//{
//	IClasterItem(CachedBarline*)
//	{}
//};

export class IBlock
{
public:
	BackPathStr dbPath;
	RefSettings settings; // Clustering settings

	void open(const BackPathStr& metaPath)
	{
		this->dbPath = metaPath / name();
	}

	virtual void addInput(InOutLayer iol) = 0;
	virtual void execute() = 0;

	virtual const BackString name() const = 0;

	virtual ~IBlock()
	{
	}
};


//using GlobalClusterRegister = GlobalRegister<TClass, TClassHolder, TClassifier, ClusterFactory>;


// Ml for machine learning
class BlockFactory
{
private:
	template<class Interface>
	using BaseCreator = std::function<std::unique_ptr<Interface>()>;

	// using ItemCreator = BaseCreator<IClassItem>;
	// using HolderCreator = BaseCreator<IClassItemHolder>;
	// using ClassifierCreator = BaseCreator<IBarClassifier>;

	template<class IF>
	using FunctionHolder = std::vector<BaseCreator<IF>>;
	FunctionHolder<IBlock> blockCreators;
	std::vector<BackString> names;

	template<class IF>
	std::unique_ptr<IF> Create(int id, FunctionHolder<IF>& creators)
	{
		//auto it = creators.find(id);
		//if (it != creators.end())
		//	return it->second();
		//else
		//	return nullptr;
		assert(id >= 0 && id < creators.size());
		return creators[id]();
	}

public:
	const std::vector<BackString>& getNames() const
	{
		return names;
	}

	size_t getCount() const
	{
		return idCounter;
	}

	int idCounter = 0;

	template <typename Block>
	int Register(BackStringView name)
	{
		blockCreators.push_back([]() { return std::make_unique<Block>(); });
		names.push_back(BackString(name.data(), name.length()));
		return idCounter++;
	}
};


export template<class TFactory, class TClass, class TClassHolder, class TML>
class GlobalRegister
{
	int id;
public:
	GlobalRegister(TFactory& factory, BackStringView name)
	{
		id = factory.template Register<TClass, TClassHolder, TML>(name);
		printf("%s class registered", name.data());
	}
	int getId() const
	{
		return id;
	}
};


struct Dummy
{
	static BlockFactory clusterFactory;
};
BlockFactory Dummy::clusterFactory;

export BlockFactory& getClusterFactory()
{
	return Dummy::clusterFactory;
}

export template<class TClass>
class BlockRegister
{
	int id;
public:
	BlockRegister(BackStringView name)
	{
		id = Dummy::clusterFactory.Register<TClass>(name);
	}
	int getId() const
	{
		return id;
	}
};
