module;
#include "../../Bind/Common.h"

export module ClusterInterface;

import TrainIO;
import IItemModule;



export class IBarClusterizer
{
public:
	BackPathStr dbPath;

	void open(const BackPathStr& metaPath)
	{
		this->dbPath = metaPath / name();
	}

	virtual const BackString name() const = 0;

	virtual void loadData(const BarCategories& categs) = 0;
	virtual void setClassesCount(int size) = 0;

	virtual void predict(const IClassItemHolder& allItems) = 0;

	virtual int test(size_t itemId) = 0;
	virtual ~IBarClusterizer()
	{
	}
};
