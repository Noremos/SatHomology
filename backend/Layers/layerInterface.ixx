module;
#include <algorithm>
#include <memory>
#include <list>

#include "../../Bind/Common.h"

export module LayersCore;

import IOCore;
import BarcodeModule;
import Platform;
import JsonCore;
import MetadataIOCore;

export const int DEF_TILE_SIZE = 300;
export const int DEF_TILE_OFFSET = 150;

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
public:
	int width; // realWid
	int tileSize = DEF_TILE_SIZE;
	float displayFactor;
	bc::point layerOffset;

	//LayerProvider(int realWid, int displayWid, int tileSize) :
	//	width(width), tileSize(tileSize),
	//	layerOffset(0,0), displayFactor(_displayFactor)
	//{
	//}

	int getTilesInRow() const
	{
		return getCon(width, tileSize);
	}

	int getCon(int total, int part) const
	{
		return total / part + (total % part == 0 ? 0 : 1);
	}

	void init(int realWid, int displayWid, int tileSize)
	{
		update(realWid, displayWid);
		this->tileSize = tileSize;
	}

	void update(int realWid, int displayWid)
	{
		this->displayFactor = realWid == displayWid ? 1.0f : static_cast<float>(realWid) / displayWid;
		this->width = realWid;
	}

	TileProvider tileByIndex(uint tileIndex) const
	{
		int tilesInRow = getTilesInRow();
		TileProvider p(tileIndex);
		p.factor = displayFactor;
		p.offset.x = (tileIndex % tilesInRow) * tileSize;
		p.offset.y = (tileIndex / tilesInRow) * tileSize;
		return p;
	}

	TileProvider tileByOffset(uint offX, uint offY) const
	{
		int tilesInRow = getTilesInRow();

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


//export struct IGuiLayer
//{
//	virtual void draw() = 0;
//};

int getCon(int total, int part)
{
	return total / part + (total % part == 0 ? 0 : 1);
}

export class ILayer : public IJsonIO
{
protected:
	enum class Type
	{
		Raster = 1,
		Vector = 2,
		Compinted = Raster | Vector
	};

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

export using SubImgInf = BackSize;

export class IRasterLayer : public ILayer
{
public:
	LayerProvider prov;
	int tileOffset = DEF_TILE_OFFSET;


	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;
	virtual int realWidth() const = 0;
	virtual int realHeight() const = 0;
	virtual BackImage getRect(int stX, int stRow, int wid, int hei) = 0;

	virtual BackImage getImage(const int max) const = 0;
	virtual const BackImage* getCachedImage() const = 0;

	Type getType()
	{
		return Type::Raster;
	}

	virtual void setSubImage(int imgIndex) = 0;
	virtual int getSubImage() = 0;
	virtual int getFirstSmallIndex(const int maxSize = 2000) = 0;
	virtual std::vector<SubImgInf> getSubImageInfos() = 0;
};


export class IVectorLayer : public ILayer
{
public:
	Type getType()
	{
		return Type::Vector;
	}
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
			if ((*it)->getSysId() == oldId)
			{
				t1 = it;
				if (foundFirst)
					break;
				foundFirst = true;
			}
			else if ((*it)->getSysId() == newId)
			{
				t2 = it;
				if (foundFirst)
					break;
				foundFirst = true;
			}
		}

		if (t1 != layers.end() && t2 != layers.end())
		{
			// auto ptr = std::move(*t1);
			// layers.remove(t1);
			// layers.insert(t2, std::move(ptr));
			std::swap(*t1, *t2);
		}
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
