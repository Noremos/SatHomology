module;
#include <algorithm>
#include <functional>
#include <memory>
#include <list>

#include "../../Bind/Common.h"
#include <cassert>

export module GroupLayer;

import IOCore;
import LayersCore;

export class GroupLayer : public ILayer
{

public:
	std::vector<int> subLayers;

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		int arrSize = subLayers.size();
		JsonArrayIOState* arr = state->arrayBegin("subIds", arrSize);
		if (state->isReading())
		{
			for (size_t i = 0; i < arrSize; i++)
			{
				int subLayerId;
				arr->scInt(i, subLayerId);
				subLayers.push_back(subLayerId);
			}
		}
		else
		{
			for (size_t i = 0; i < arrSize; i++)
				arr->scInt(i, subLayers[i]);
		}
	}

	virtual Type getType() const
	{
		return Type::Group;
	}

	virtual const LFID getFactoryId() const
	{
		return GROUP_LAYER;
	}

	virtual void release(const MetadataProvider&)
	{
		// BackString lname = getMetaLayerName();
		// metaFolder.update(metaFolder.getSubMeta(lname));
	}

	BackPoint getDisplaySize() const
	{
		//return BackPoint(displayWidth(), displayHeight());
		return BackPoint(0, 0);
	}

	BackPoint getRealSize() const
	{
		return BackPoint(0,0);
	}

	virtual int realWidth() const override
	{
		return 0;
	}
	virtual int realHeight() const override
	{
		return 0;
	}

	// Унаследовано через ILayer
	float displayWidth() const override
	{
		return 0.0f;
	}

	float displayHeight() const override
	{
		return 0.0f;
	}

};
