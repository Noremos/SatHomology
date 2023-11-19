module;
#include "../../Bind/Common.h"

export module ClusterInterface;

import TrainIO;
import IItemModule;
import CachedBarcode;

export class IClasterItem
{
	IClasterItem(CachedBarline*)
	{}
};

export class IBarClusterizer
{
public:
	BackPathStr dbPath;

	void open(const BackPathStr& metaPath)
	{
		this->dbPath = metaPath / name();
	}

	virtual const BackString name() const = 0;

	//virtual void loadData() = 0;
	virtual void setClassesCount(int size) = 0;

	virtual void predict(const IClassItemHolder& allItems) = 0;

	virtual int test(size_t itemId) = 0;
	virtual ~IBarClusterizer()
	{
	}
};



using ClusterFactory = ImlFactory<IBarClusterizer>;

export ClusterFactory& getClusterFactory()
{
	static ClusterFactory clusterFactory;
	return clusterFactory;
}

export template<class TClass, class TClassHolder, class TClassifier>
class GlobalClusterRegister : public GlobalRegister<ClusterFactory, TClass, TClassHolder, TClassifier>
{
public:
	GlobalClusterRegister(std::string_view name = "") : GlobalRegister<ClusterFactory, TClass, TClassHolder, TClassifier>(getClusterFactory(), name)
	{ }
};
//using GlobalClusterRegister = GlobalRegister<TClass, TClassHolder, TClassifier, ClusterFactory>;
