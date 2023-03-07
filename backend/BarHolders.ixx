module;
#include <vector>

export module BarholdersModule;
import BarcodeModule;

namespace bc
{
	using barlinevector = std::vector<bc::barline*>;
	using barvector = std::vector<bc::barvalue>;
}



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