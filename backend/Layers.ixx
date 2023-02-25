module;
#include <memory>
#include "../bind/common.h"
#include <algorithm>

export module LayersCore;

import IOCore;
import BarcodeModule;

struct SimpleLine;

export struct SimpleLine
{
	int id, barlineIndex;
	SimpleLine(int id = 0, int barlineIndex = 0) :
		id(id), barlineIndex(barlineIndex), start(0), end(0), depth(0), matrSrcSize(0), parent(nullptr)
	{}
	//	ushort counter = 0;
	Barscalar start, end;

	int depth;
	int matrSrcSize;

	std::shared_ptr<SimpleLine> parent;

	bc::barvector matr;

	int getDeath()
	{
		return depth;
	}
};

export class DrawPrimetive
{
public:
	//SimpleLine line;

	enum class Type
	{
		points,
		polygon,
		multipolygon
	};

	Type type;
	Barscalar color;
	std::vector<bc::point> draws;
};

export class ILayer
{
};

export class RasterLayer : public ILayer
{
public:
	BackImage mat;

	bc::point minmax(const bc::point& p) const
	{
		return { std::min(p.x, mat.width() - 1), std::min(p.y, mat.height() - 1) };
	}
};

export class PrimetiveLayer : public ILayer
{
public:
	std::vector<DrawPrimetive> primetives;
};

export class RasterLineLayer : public RasterLayer
{
public:
	std::vector<std::shared_ptr<SimpleLine>> clickResponser;

	void setMatrPoint(int x, int y, int curLineDepth, std::shared_ptr<SimpleLine>& newLine)
	{
		int indLocal = mat.getLineIndex(x, y);
		SimpleLine* existLine = clickResponser[indLocal].get();

		if (existLine == nullptr)
		{
			clickResponser[indLocal] = newLine;
		}
		else if (existLine->getDeath() < curLineDepth)
		{
			// main(depth) < child(depth)
			//newLine->parent = existLine;
			clickResponser[indLocal] = newLine;

			int ylek = 2;
			for (int i = MAX(x - ylek, 0); i < std::min(x + ylek, mat.wid()); i++)
			{
				for (int j = MAX(y - ylek, 0); j < std::min(y + ylek, mat.hei()); j++)
				{
					int indLocal2 = mat.getLineIndex(i, j);

					clickResponser[indLocal2] = newLine;
				}
			}
		}
	}
};

export struct TileProvider
{
public:
	float factor;
	int index;
	bc::point offset; // In rastar coords

	TileProvider(float factor, int index = 0) : factor(factor), index(index), offset(0, 0)
	{ }
	TileProvider(float factor, int x, int y) : factor(factor), index(0), offset(x, y)
	{ }

	bc::point toGlobal(uint locX, uint locY) const
	{
		return bc::point((locX + offset.x) / factor, (locY + offset.y) / factor);
	}
};

export class LayerProvider
{
//std::function calcOffset
	int tilesInRow;
	int tileSize;
public:
	float displayFactor;
	bc::point layerOffset;

	LayerProvider(float displayFactor) :
		layerOffset(0,0), displayFactor(displayFactor)
	{ }

	int getCon(int total, int part)
	{
		return total / part + (total % part == 0 ? 0 : 1);
	}

	void init(int tileSize, int width)
	{
		this->tileSize = tileSize;
		this->tilesInRow = getCon(width, tileSize);
	}

	TileProvider tileByIndex(uint tileIndex) const
	{
		TileProvider p(tileIndex);
		p.offset.x = (tileIndex / tilesInRow) * tileSize;
		p.offset.y = (tileIndex % tilesInRow) * tileSize;
		return p;
	}

	TileProvider tileByOffset(uint offX, uint offY) const
	{
		TileProvider p(offX, offY);
		p.index = (offY / tileSize) * tilesInRow + offX / tileSize;
		return p;
	}


	bc::point toGlobal(uint locX, uint locY) const
	{
		return bc::point((locX + layerOffset.x) / displayFactor, (locY + layerOffset.y) / displayFactor);
	}

	bc::point toGlobalFactor(uint locX, uint locY) const
	{
		return bc::point(locX / displayFactor, locY / displayFactor);
	}

	bc::point getTileOffset(int index)
	{
		return tileByIndex(index).offset;
	}

	//bc::point toLocal(uint x, uint y) const
	//{
	//	return { (x + offset.x) / displayFactor, (y + offset.y) / displayFactor };
	//}
};
