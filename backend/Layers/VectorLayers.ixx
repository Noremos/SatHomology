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


template<class FR>
static void saveLoadChar(JsonObjectIOState* state, const char* name, FR& val)
{
	int valInt = static_cast<int>(val);
	state->scInt(name, valInt);
	val = static_cast<FR>(valInt);
}


export class DrawPrimitive
{
	bool setted = false;
public:
	BackColor color;
	std::vector<BackPoint> points;

	DrawPrimitive(const BackColor& bp = BackColor()) : color(bp)
	{
	}

	void addPoint(BackPoint p)
	{
		points.push_back(p);
	}

	void clear()
	{
		points.clear();
	}

	//IClassItemHolder *load(int &index, IClassItemHolder *t)
	//{
	//	assert(state->isReading());

	//	index = state->pInt(0);
	//	t->read(state.get());
	//	return t;
	//}
	BackString pointsAsGeojson()
	{
		BackString safsd = "\"geometry\": {\"type\":\"Polygon\", \"coordinates\":[[[ ";
		asGeojsonCommon(safsd);
		safsd += "]]]";

		return safsd;
	}

	BackString polygonAsGeojson()
	{
		BackString safsd = "\"geometry\": {\"type\":\"Polygon\", \"coordinates\":[[[ ";

		asGeojsonCommon(safsd);
		safsd += "]]]";

		return safsd;
	}

	void asGeojsonCommon(BackString& prefix) const
	{
		for (const BackPoint& p : points)
		{
			prefix += std::format("[{}, {}],", p.y, p.x);
		}
		prefix[prefix.length() - 1] = ' ';
	}

	void saveLoadState(StateBinFile::BinState* state)
	{
		state->beginItem();

		color.r = state->pInt(color.r);
		color.g = state->pInt(color.g);
		color.b = state->pInt(color.b);

		int size = state->pInt(points.size());
		if (state->isReading())
		{
			points.resize(size);
		}

		for (int i = 0; i < size; i++)
		{
			state->pInt(points[i].x);
			state->pInt(points[i].y);
		}

		state->endItem();
	}
};

export class VectorLayer : public IVectorLayer
{
	int binId = -1;

public:
	enum class VecType : short
	{
		points,
		polygons,
		multipolygons
	};

	VecType vecType;
	BackColor color;
	std::vector<DrawPrimitive> primitives;

	void clear()
	{
		primitives.clear();
	}

	DrawPrimitive& addPrimitive(const BackColor& col)
	{
		primitives.push_back(DrawPrimitive(col));
		return primitives.back();
	}

	virtual const LFID getFactoryId() const
	{
		return VECTOR_LAYER_FID;
	}

	void savePolygonsAsGeojson(const BackPathStr& savePath) const
	{
		BackString json = "{\"type\":\"FeatureCollection\","
			"\"name\":\"Roofs\","
			"\"crs\": { \"type\": \"name\", \"properties\":{\"name\": \"urn:ogc:def:crs:EPSG::";
		json += intToStr(cs.getProjId()); //3857
		json += "\" } },";
		json +=  "\"features\":[ ";

		for (int i = 0, total = primitives.size(); i < total; ++i)
		{
			BackString safsd = "{ \"type\": \"Feature\",";
			safsd += "\"properties\":{\"id\":"; // TODO! Check
			safsd += intToStr(i + 1);
			safsd += "}, ";
			switch (type)
			{
			case VecType::points:
				safsd += primitives[i].pointsAsGeojson();
				break;
			case VecType::polygons:
				safsd += primitives[i].pointsAsGeojson();
				break;
			}
			safsd += "}},";
		}


		json[json.length() - 1] = ']';
		json += "}";

		std::ofstream file(savePath, std::ios::trunc);
		if (file.is_open())
		{
			file << json;
			file.close();
		}
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		IVectorLayer::saveLoadState(state, metaFolder);

		saveLoadChar(state, "vectype", vecType);
		saveLoadChar(state, "col_r", color.r);
		saveLoadChar(state, "col_g", color.g);
		saveLoadChar(state, "col_b", color.b);

		// int imgId = metaFolder.getUniqueId();
		// state->scInt("mat_id", imgId);
		int size = primitives.size();
		state->scInt("primitivesSize", size);


		if (state->isReading())
		{
			primitives.resize(size);
			StateBinFile::BinStateReader binstate;
			state->scInt("binId", binId);

			binstate.open(metaFolder.getFile(intToStr(binId)).string());
			for (int i = 0; i < size; i++)
			{
				primitives[i].saveLoadState(&binstate);
			}
			binstate.close();
		}
		else
		{
			if (binId == -1)
			{
				binId = metaFolder.getUniqueId();
			}
			state->scInt("binId", binId);

			StateBinFile::BinStateWriter binstate;
			binstate.open(metaFolder.getFile(intToStr(binId)).string());
			for (int i = 0; i < size; i++)
			{
				primitives[i].saveLoadState(&binstate);
			}
			binstate.close();
		}
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
