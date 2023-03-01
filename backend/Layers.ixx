module;
#include <memory>
#include "../bind/common.h"
#include <algorithm>
#include <list>
#include <memory>

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

//export struct IGuiLayer
//{
//	virtual void draw() = 0;
//};

export class ILayer
{
public:
	int id = -1;

	BackString name;
	int iconId = -1;

	int getSysId()
	{
		return id;
	}

	//IGuiLayer* toGuiLayer();
};


export template <class T>
class LayersList
{
	std::list<std::unique_ptr<T>> layers;

public:
	T* at(uint id)
	{
		for (auto& layer : layers)
		{
			if (layer->getSysId() == id)
			{
				return layer.get();
			}
		}

		return nullptr;
		//assert(id < layers.size());
		//auto t = layers.begin();
		//std::advance(t, id);
		//return t->get();
	}

	auto begin()
	{
		return layers.begin();
	}

	auto end()
	{
		return layers.end();
	}

	template<class TReal>
	TReal* add()
	{
		auto t = std::make_unique<TReal>();
		auto tptr = t.get();
		layers.push_back(std::move(t));
		return tptr;
	}

	template<class TReal, class TArg>
	TReal* add(TArg arg)
	{
		auto t = std::make_unique<TReal>(arg);
		auto tptr = t.get();
		layers.push_back(std::move(t));
		return tptr;
	}

	bool set(uint id, T* val)
	{
		auto t = layers.begin();

		for (auto& layer : layers)
		{
			if (layer->getSysId() == id)
			{
				layer.reset(val);
				return true;
			}
		}

		return false;
	}

	bool set(uint id, std::unique_ptr<T>& val)
	{
		auto t = layers.begin();

		for (auto& layer : layers)
		{
			if (layer->getSysId() == id)
			{
				layer = std::move(val);
				return true;
			}
		}
		return false;
	}

	void move(uint oldId, uint newId)
	{
		auto t1 = layers.end();
		auto t2 = layers.end();
		bool foundFirst = false;
		for (auto it = layers.begin(); it != layers.end(); it++)
		{
			if (it->getSysId() == oldId)
			{
				t1 = it;
				if (foundFirst)
					break;
				foundFirst = true;
			}
			else if (it->getSysId() == newId)
			{
				t2 = it;
				if (foundFirst)
					break;
				foundFirst = true;
			}
		}
		//auto* ptr = t1.get().release();
		//layers.remove(t1);
		//layers.insert(t2, std::unique_ptr(ptr));
		std::swap(t1, t2);
	}

	void clear()
	{
		layers.clear();
	}

	size_t size()
	{
		return layers.size();
	}
	//template<class TReal>
	//TReal* getOrAdd(int id)
	//{
	//	if (id == -1)
	//	{

	//	}

	//	auto t = std::make_unique<TReal>();
	//	auto tptr = t.get();
	//	layers.push_back(std::move(t));
	//	return tptr;
	//}


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


	void init(const BackImage& src)
	{
		clear();
		mat.assignCopyOf(src);
		clickResponser.resize(mat.length());
	}

	void init(int wid, int hei)
	{
		clear();
		mat.reinit(wid, hei, 4);
		clickResponser.resize(mat.length());
	}

	void clear()
	{
		clickResponser.clear();
	}


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

	LayerProvider(float _displayFactor) :
		tilesInRow(0), tileSize(0),
		layerOffset(0,0), displayFactor(_displayFactor)
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
		p.factor = displayFactor;
		p.offset.x = (tileIndex / tilesInRow) * tileSize;
		p.offset.y = (tileIndex % tilesInRow) * tileSize;
		return p;
	}

	TileProvider tileByOffset(uint offX, uint offY) const
	{
		TileProvider p(offX, offY);
		p.factor = displayFactor;
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
