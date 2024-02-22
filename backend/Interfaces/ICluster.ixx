module;
#include <cassert>
#include "Barcode/PrjBarlib/include/barline.h"
#include "Usings.h"
export module ClusterInterface;

import BackBind;
import IItemModule;
import CachedBarcode;
import MLSettings;
import StateBinIO;
// import BarTypes;
//export class IClasterItem
//{
//	IClasterItem(CachedBarline*)
//	{}
//};


export class ICluster : public IClassItem // : public IBffIO
{
public:
	//virtual const bc::barvector& getMatrix() const = 0;

	virtual size_t getId() const
	{
		return -1;
	}

	virtual size_t getParentId() const
	{
		return -1;
	}
	// virtual const bc::barlinevector& getLines() const = 0;

	virtual void saveLoadState(StateBinFile::BinState*)
	{ }

	virtual const size_t getMatrixSize() const
	{
		return getMatrix().size();
	}

	bc::barvalue getMatrValue(int id) const
	{
		return getMatrix()[id];
	}

	virtual int getDeath() const
	{
		return 0;
	}
	virtual Barscalar start() const
	{
		return 0;
	}
	virtual Barscalar end() const
	{
		return 0;
	}

	virtual ~ICluster()
	{}
};

// Holder - a container to store items
export class IClusterItemHolder : public IClassItemHolder
{
public:
	IClusterItemHolder(bool isRootBased) : settings(), rootBased(isRootBased)
	{ }

	MLSettings settings; // Prepare
	bool rootBased;

	//virtual ICluster* getItem(size_t id) = 0;
	virtual const ICluster* getCItem(size_t id) const = 0;
	virtual void addItem(const CachedBarline& item) = 0;
	virtual void clear() = 0;

	virtual ~IClusterItemHolder() {}

private:
	const IClassItem* getItem(size_t id) const override
	{
		return getCItem(id);
	}

	IClassItem* getItem(size_t id) override
	{
		assert(false);
		throw;
		return nullptr;
	}

	void saveLoadState(StateBinFile::BinState* state) override
	{
		assert(false);
		throw;
	}

	void create(bc::DatagridProvider* img, const bc::barstruct& constr, const ItemCallback& callback) override
	{
		assert(false);
		throw;
	}

	void addItem(const IClassItem& line) override
	{
		auto* r = dynamic_cast<const CachedBarline*>(&line);
		if (r)
			addItem(*r);
		else
		{
			assert(false);
			throw;
		}
	}

};


export template<typename T>
class IClusterItemValuesHolder : public IClusterItemHolder
{
protected:
	std::vector<T> items;

public:

	IClusterItemValuesHolder(bool isRootBased) : IClusterItemHolder(isRootBased)
	{ }

	T* getRItem(size_t id)
	{
		return &items[id];
	}

	const T& getRItem(size_t id) const
	{
		return items[id];
	}

	const ICluster* getCItem(size_t id) const override
	{
		return dynamic_cast<const ICluster*>(&items[id]);
	}

	size_t getItemsCount() const override
	{
		return items.size();
	}

	virtual void clear() override
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

struct Dummy
{
	static ClusterFactory clusterFactory;
};
ClusterFactory Dummy::clusterFactory;

export ClusterFactory& getClusterFactory()
{
	return Dummy::clusterFactory;
}

export template<class TClass, class TClassHolder, class TClassifier>
class GlobalClusterRegister : public GlobalRegister<ClusterFactory, TClass, TClassHolder, TClassifier>
{
public:
	GlobalClusterRegister(BackStringView name = "") : GlobalRegister<ClusterFactory, TClass, TClassHolder, TClassifier>(getClusterFactory(), name)
	{
		printf(" for cluster\n");
	}
};


//using GlobalClusterRegister = GlobalRegister<TClass, TClassHolder, TClassifier, ClusterFactory>;
