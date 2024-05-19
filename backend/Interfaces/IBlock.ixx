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
	virtual RetLayers execute(InOutLayer iol) = 0;

	virtual const BackString name() const = 0;

	virtual ~IBlock()
	{
	}
};


//using GlobalClusterRegister = GlobalRegister<TClass, TClassHolder, TClassifier, ClusterFactory>;


// Ml for machine learning
export class BlockFactory
{
private:
	using BaseHolder = std::unique_ptr<IBlock>;

	// using ItemCreator = BaseCreator<IClassItem>;
	// using HolderCreator = BaseCreator<IClassItemHolder>;
	// using ClassifierCreator = BaseCreator<IBarClassifier>;

	template<class IF>
	using FunctionHolder = std::vector<BaseHolder>;
	FunctionHolder<IBlock> blockHolder;
	std::vector<BackString> names;

public:
	IBlock* get(int id)
	{
		if (id >= 0 && id < blockHolder.size())
			return blockHolder[id].get();
		else
			return nullptr;
	}

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
	int Register()
	{
		blockHolder.push_back(std::make_unique<Block>());
		names.push_back(blockHolder.back()->name());
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


struct Dummy2
{
	static BlockFactory blockFactory;
};
BlockFactory Dummy2::blockFactory;

export BlockFactory& getBlockFactory()
{
	return Dummy2::blockFactory;
}

export template<class TClass>
class BlockRegister
{
	int id;
public:
	BlockRegister()
	{
		id = Dummy2::blockFactory.Register<TClass>();
	}
	int getId() const
	{
		return id;
	}
};
