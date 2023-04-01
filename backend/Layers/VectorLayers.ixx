module;
#include <memory>
#include <unordered_set>
#include <algorithm>

#include "../../Bind/Common.h"

export module VectorLayers;
//import std.core;
import LayersCore;

import IOCore;
import Platform;

import MetadataIOCore;
import BarcodeModule;

using LayerMetaProvider = MetadataProvider;


export class DrawPrimitive
{
public:
	BackColor color;
	bool setted = false;
	std::vector<BackPoint> points;

	DrawPrimitive(const BackColor& bp = BackColor()) : color(bp)
	{
	}

	void addPoint(BackPoint p)
	{
		points.push_back(p);
	}
};

export class VectorLayer : public IVectorLayer
{
public:
	enum class VecType
	{
		points,
		polygons,
		multipolygons
	};

	VecType vecType;
	BackColor color;
	std::vector<DrawPrimitive> primitives;

	DrawPrimitive& addPrimitive(const BackColor& col)
	{
		primitives.push_back(DrawPrimitive(col));
		return primitives.back();
	}

	virtual const LFID getFactoryId() const
	{
		return VECTOR_LAYER_FID;
	}
};

export class MultiVectorLayer : public IVectorLayer
{
public:
	std::vector<DrawPrimitive> primitives;
	virtual const LFID getFactoryId() const
	{
		return MULTIPOLY_VECTOR_LAYER_FID;
	}
};
