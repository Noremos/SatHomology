module;
#include <algorithm>
#include <functional>
#include <memory>
#include <list>

#include "../../Bind/Common.h"
#include <cassert>

export module LayersCore;

import IOCore;
import BarcodeModule;
import Platform;
import JsonCore;
import MetadataIOCore;
import CSBind;
import MHashMap;

export const int DEF_TILE_SIZE = 1000;
export const int DEF_TILE_OFFSET = 300;

export struct TileProvider
{
public:
	float factor;
	int index;
	bc::point offset; // In rastar coords

	TileProvider(float factor, int index) : factor(factor), index(index), offset(0, 0)
	{ }
	TileProvider(float factor, int x, int y) : factor(factor), index(0), offset(x, y)
	{ }

	BackPixelPoint tileToPreview(uint locX, uint locY) const
	{
		return BackPixelPoint((locX + offset.x) / factor, (locY + offset.y) / factor);
	}

	BackPixelPoint tileToFull(uint locX, uint locY) const
	{
		return BackPixelPoint(locX + offset.x, locY + offset.y);
	}

	TileProvider& operator=(const TileProvider& other)
	{
		factor = other.factor;
		index = other.index;
		offset = other.offset;
		return *this;
	}
};


export class LayerProvider
{
public:
	int width; // realWid
	int height;
	int tileSize = DEF_TILE_SIZE;
	float displayFactor;
	bc::point layerOffset;


	LayerProvider() :
		width(0), height(0),
		tileSize(DEF_TILE_SIZE),
		displayFactor(1.f),
		layerOffset(0,0)
	{
	}

	//LayerProvider(int realWid, int displayWid, int tileSize) :
	//	width(width), tileSize(tileSize),
	//	layerOffset(0,0), displayFactor(_displayFactor)
	//{
	//}

	int getOptTilesInRow(int offset) const
	{
		return width / tileSize + ((width % tileSize > offset) ? 1 : 0);
	}

	int getOptTilesInCol(int offset) const
	{
		return height / tileSize + ((height % tileSize > offset) ? 1 : 0);
	}

	int getTilesInRow() const
	{
		return getCon(width, tileSize);
	}

	int getCon(int total, int part) const
	{
		return total / part + (total % part == 0 ? 0 : 1);
	}

	void init(int realWid, int realHeight, int displayWid, int tileSize)
	{
		update(realWid, realHeight, displayWid);
		this->tileSize = tileSize;
	}

	void update(int realWid, int realHei, int displayWid)
	{
		this->displayFactor = realWid == displayWid ? 1.0f : static_cast<float>(realWid) / displayWid;
		this->width = realWid;
		this->height = realHei;
		if (tileSize > width)
			tileSize = width;
	}

	float getDisplayWidth()
	{
		return static_cast<float>(width) / displayFactor;
	}

	float getDisplayHeight()
	{
		return static_cast<float>(height) / displayFactor;
	}

	TileProvider tileByIndex(uint tileIndex) const
	{
		int tilesInRow = getTilesInRow();
		TileProvider p(displayFactor, tileIndex);
		p.offset.x = (tileIndex % tilesInRow) * tileSize;
		p.offset.y = (tileIndex / tilesInRow) * tileSize;
		return p;
	}

	TileProvider tileByOffset(uint offX, uint offY) const
	{
		int tilesInRow = getTilesInRow();

		TileProvider p(displayFactor, offX, offY);
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


	// WindowVec2 localDisplayPos;
	// ImVec2 displaySize;

	// int getRealX(int x)
	// {
	// 	return static_cast<float>(x - localDisplayPos.x) * (width / displaySize.x);
	// }
	// int getRealY(int y)
	// {
	// 	return static_cast<float>(y - localDisplayPos.y) * (height / displaySize.y);
	// }

	// int toDisplayX(int x)
	// {
	// 	return static_cast<float>(x) * (displaySize.x / width) + localDisplayPos.x;
	// }
	// int toDisplayY(int y)
	// {
	// 	return static_cast<float>(y) * (displaySize.y / height) + localDisplayPos.y;
	// }

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

export using LFID = unsigned int;

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
	static LFID counter;
	static LFID getCountId()
	{
		return counter++;
	}

	BackString getMetaLayerName()
	{
		return std::format("{}_layer", id);
	}

	MetadataProvider getLayerMeta(const MetadataProvider& metaFolder)
	{
		BackString lname = getMetaLayerName();
		return metaFolder.getSubMeta(lname);
	}

public:
	int id = -1;
	BackString name;
	CSBinding cs;
	bool isSystem = false;

	void init(int proj)
	{
		cs.init(proj);
	}

	void initCSFrom(const CSBinding& csf)
	{
		assert(cs.proj.isInited());
		cs.globOrigin = csf.globOrigin;
		std::copy(csf.img_transform, csf.img_transform + 6, cs.img_transform);
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		state->scInt("coreId", id);
		state->scStr("name", name);

		int d = isSystem ? 1 : 0;
		state->scInt("isSystem", d);
		isSystem = d == 1;

		cs.saveLoadState(state, metaFolder);
	}

	virtual Type getType() const = 0;
	virtual const LFID getFactoryId() const = 0;
	virtual void release(const MetadataProvider&)
	{
		// BackString lname = getMetaLayerName();
		// metaFolder.update(metaFolder.getSubMeta(lname));
	}

	virtual float displayWidth() const = 0;
	virtual float displayHeight() const = 0;

	BackPoint getDisplaySize() const
	{
		return BackPoint(displayWidth(), displayHeight());
	}

	BackPoint getRealSize() const
	{
		return BackPoint(realWidth(), realHeight());
	}

	virtual int realWidth() const = 0;
	virtual int realHeight() const = 0;

	int getSysId() const
	{
		return id;
	}

	bool hasCS() const
	{
		return cs.proj.isInited();
	}

	BackPoint getNormGlobStart() const
	{
		BackPoint st = getGlobStart();
		BackPoint ed = getGlobEnd();

		if (st.x > ed.x)
		{
			std::swap(st.x, ed.x);
		}

		if (st.y > ed.y)
		{
			std::swap(st.y, ed.y);
		}

		return st;
	}

	BackPoint getNormGlobSize() const
	{
		return cs.getScaled(getRealSize()).abs();
	}

	BackPoint getGlobStart() const
	{
		return cs.globOrigin;
	}

	BackPoint getGlobEnd() const
	{
		return getGlobStart() + getGlobSize();
	}

	BackPoint getGlobSize() const
	{
		return cs.getScaled(getDisplaySize());
	}

	virtual ~ILayer()
	{ }
};

LFID ILayer::counter = 0;
export const LFID RASTER_LAYER_FID = 0;
export const LFID RASTER_LINE_LAYER_FID = 1;
export const LFID RASTER_DISK_LAYER_FID = 2;
export const LFID VECTOR_LAYER_FID = 3;
export const LFID MULTIPOLY_VECTOR_LAYER_FID = 4;// ILayer::getCountId();

export class CoreLayerFactory
{
protected:
	template<class IF>
	using FunctionCoreHolder = MMMAP<LFID, std::function<IF*()> >;

	template<class IF>
	using FunctionGuiHolder = MMMAP<LFID, std::function<IF*(ILayer* core)> >;

	static FunctionCoreHolder<ILayer> coreLayersCreators;

public:
	static ILayer* CreateCoreLayer(int id)
	{
		auto it = coreLayersCreators.find(id);
		if (it != coreLayersCreators.end())
			return it->second();
		else
			return nullptr;
	}
};

CoreLayerFactory::FunctionCoreHolder<ILayer> CoreLayerFactory::coreLayersCreators;



export using SubImgInf = BackSize;

export class IRasterLayer : public ILayer
{
public:
	LayerProvider prov;
	int tileOffset = DEF_TILE_OFFSET;

	Type getType() const
	{
		return Type::Raster;
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		ILayer::saveLoadState(state, metaFolder);
		JsonObjectIOState* jobj = state->objectBegin("provider");

		jobj->scInt("width", prov.width);
		jobj->scInt("height", prov.height);
		jobj->scInt("tileSize", prov.tileSize);
		jobj->scFloat("displayFactor", prov.displayFactor);
		jobj->scInt("layerOffsetX", prov.layerOffset.x);
		jobj->scInt("layerOffsetY", prov.layerOffset.y);
		state->objectEnd();
	}

	virtual BackImage getRect(int stX, int stRow, int wid, int hei) = 0;

	virtual BackImage getImage(const int max) const = 0;
	virtual const BackImage* getCachedImage() const = 0;

	virtual void setCache(size_t) = 0;
	virtual void setSubImage(int imgIndex, bool overrideCs = false) = 0;
	virtual int getSubImage() = 0;
	virtual SubImgInf getSubImgInf() = 0;

	virtual int getFirstSmallIndex(const int maxSize = 2000) = 0;
	virtual std::vector<SubImgInf> getSubImageInfos() = 0;
};


export class IVectorLayer : public ILayer
{
public:
	Type getType() const
	{
		return Type::Vector;
	}

	virtual float displayWidth() const
	{
		return 0;
	}

	virtual float displayHeight() const
	{
		return 0;
	}

	virtual void release()
	{

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

	T* at(const BackString& name)
	{
		for (auto& layer : layers)
		{
			if (layer->name == name)
			{
				return layer.get();
			}
		}

		return nullptr;
	}

	void remove(uint id)
	{
		for (auto it = layers.begin(); it != layers.end(); it++)
		{
			if ((*it)->getSysId() == id)
			{
				layers.erase(it);
				break;
			}
		}
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


	void addMove(T* value)
	{
		std::unique_ptr<T> ptr(value);
		layers.push_back(std::move(ptr));
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
