module;
#include "../../Bind/Common.h"

export module ClusterInterface;

import TrainIO;
import IItemModule;
import CachedBarcode;
import BarcodeModule;
import MLSettings;
//export class IClasterItem
//{
//	IClasterItem(CachedBarline*)
//	{}
//};


export class ICluster // : public IBffIO
{
public:
	virtual const bc::barvector& getMatrix() const = 0;
	virtual ~ICluster()
	{}
};

// Holder - a container to store items
export class IClusterItemHolder// : public IBffIO
{
public:
	IClusterItemHolder()
	{ }

	MLSettings settings; // Prepare

	//virtual ICluster* getItem(size_t id) = 0;
	virtual const ICluster* getItem(size_t id) const = 0;
	virtual size_t getItemsCount() const = 0;
	virtual void addItem(const CachedBarline& item) = 0;
	virtual void clear() = 0;

	virtual ~IClusterItemHolder() {}
};


export template<typename T>
class IClusterItemValuesHolder : public IClusterItemHolder
{
protected:
	std::vector<T> items;

public:

	IClusterItemValuesHolder() : IClusterItemHolder()
	{ }

	ICluster* getItem(size_t id)
	{
		return &items[id];
	}

	const ICluster* getItem(size_t id) const
	{
		return &items[id];
	}

	size_t getItemsCount() const
	{
		return items.size();
	}

	virtual void clear()
	{
		items.clear();
	}


	virtual ~IClusterItemValuesHolder()
	{ }
};


export class IBarClusterizer
{
public:
	BackPathStr dbPath;
	MLSettings settings; // Clustering settings

	void open(const BackPathStr& metaPath)
	{
		this->dbPath = metaPath / name();
	}

	virtual const BackString name() const = 0;

	//virtual void loadData() = 0;
	virtual void setClassesCount(int size) = 0;

	virtual bool predict(const IClusterItemHolder& allItems) = 0;

	virtual int test(size_t itemId) = 0;
	virtual int getClusters() = 0;
	virtual ~IBarClusterizer()
	{
	}
};



using ClusterFactory = ImlFactory<ICluster, IClusterItemHolder, IBarClusterizer>;

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
