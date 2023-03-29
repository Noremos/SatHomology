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


export class DrawPrimetive
{
public:
	BackColor color;
	bool setted = false;
	std::vector<BackPoint> draws;
};

export class VectorLayer : public IVectorLayer
{
public:
	enum class VecType
	{
		points,
		polygon,
		multipolygon
	};

	VecType vecType;
	BackColor color;
	DrawPrimetive primetive;
	virtual const LFID getFactoryId() const
	{
		return VECTOR_LAYER_FID;
	}
};

export class MultiVectorLayer : public IVectorLayer
{
public:
	std::vector<DrawPrimetive> primetives;
	virtual const LFID getFactoryId() const
	{
		return MULTIPOLY_VECTOR_LAYER_FID;
	}
};
