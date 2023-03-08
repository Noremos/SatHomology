module;
#include <vector>
#include <memory>

#include "../Bind/Common.h"


export module BarholdersModule;

import BarcodeModule;
import ClassifierInterface;

namespace bc
{
	using barlinevector = std::vector<bc::barline*>;
	using barvector = std::vector<bc::barvalue>;
}

export class ClassItemHolder
{
public:
	std::vector<IClassItem*> items;

	~ClassItemHolder()
	{
		for (size_t var = 0; var < items.size(); ++var)
		{
			delete items[var];
		}
		items.clear();
	}
};


export class BarcodeHolder
{
public:
	bc::barlinevector lines;
	bc::barvector matrix;
	int depth;
	int getDeath()
	{
		return depth;
	}
	void cloneLines(bc::barlinevector& out) const
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			out.push_back(lines[var]->clone(false));
		}
	}
	~BarcodeHolder()
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			delete lines[var];
		}
		lines.clear();
	}
};



export class BarcodesHolder
{
public:
	std::vector<BarcodeHolder*> lines;

	~BarcodesHolder()
	{
		for (size_t var = 0; var < lines.size(); ++var)
		{
			delete lines[var];
		}
		lines.clear();
	}
};


export
{
	using CloudItem = BarcodesHolder;
	using CloudBarcodeHolder = BarcodesHolder;
}