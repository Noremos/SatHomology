module;
#include "GuiWidgets.h"

export module GuiOverlap;

import IOCore;
import LayersCore;

struct vec3
{
	short x, y, z;

	vec3(const Barscalar& col)
	{
		x = col.data.b3[0];
		y = col.data.b3[1];
		z = col.data.b3[2];
	}

	vec3(short _x, short _y, short _z) : x(_x), y(_y), z(_z)
	{ }

	vec3 operator+(const vec3& b) const
	{
		return vec3(x + b.x, y + b.y, z + b.z);
	}

	vec3 operator-(const vec3& b) const
	{
		return vec3(x - b.x, y - b.y, z - b.z);
	}

	vec3 operator*(float v) const
	{
		return vec3(x * v, y * v, z * v);
	}

	Barscalar getSc()
	{
		assert(x >= 0);
		assert(x < 256);
		assert(y >= 0);
		assert(y < 256);
		assert(z >= 0);
		assert(z < 256);
		return Barscalar(x, y, z);
	}
	float length()
	{
		return std::sqrtf(x * x + y * y + z * z);
	}
};


export class IOverlapProcess
{
public:
	bool enable = true;
	//virtual void init() = 0;
	virtual void draw(ImVec2 pos, ImVec2 size) = 0;
};

export class HeimapOverlap : public IOverlapProcess
{
	GuiDrawImage heimap;
public:

	void init(const BackImage& mainMat)
	{
		Barscalar mi, ma;
		mainMat.maxAndMin(mi, ma);

		vec3 mixc{ mi.data.b3[0], mi.data.b3[1], mi.data.b3[2] };
		vec3 maxc{ ma.data.b3[0], ma.data.b3[1], ma.data.b3[2] };
		vec3 cDiff = maxc - mixc;

		vec3 minHeiCol{ 0, 0, 255 };
		vec3 maxHeiCol{ 255, 0, 0 };
		vec3 colDiff = maxHeiCol - minHeiCol;

		BackImage heim(mainMat.width(), mainMat.height(), 3);
		for (size_t i = 0; i < mainMat.length(); i++)
		{
			vec3 col = mainMat.getLiner(i);
			float t = (col - mixc).length() / cDiff.length();
			vec3 newCol = minHeiCol + colDiff * t;
			heim.setLiner(i, newCol.getSc());
		}

		heimap.setImage(heim);
	}
	virtual void draw(ImVec2 pos, ImVec2 size)
	{
		if (!enable)
			return;

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowViewport(viewport->ID);
		heimap.drawImage("Heimap", true);
	}
};


export class TilemapOverlap : public IOverlapProcess
{
	GuiDrawImage* img;
	int tileSize = 0;

public:

	void init(GuiDrawImage* src, int tileSize)
	{
		img = src;
		this->tileSize = tileSize;
	}

	void setTilesize(int tileSize)
	{
		this->tileSize = tileSize;
	}

	virtual void draw(ImVec2 pos, ImVec2)
	{
		if (tileSize == 0 || !enable)
			return;

		ImVec2 cont = ImGui::GetCursorPos();
		ImDrawList* list = ImGui::GetWindowDrawList();

		ImVec2 drawTileSize(tileSize / img->scaleFactor, tileSize / img->scaleFactor);
		ImVec2 maxPos = pos + img->localDisplayPos;

		int newWid = img->displaySize.x;
		int newHei = img->displaySize.y;
		ImColor lineColor(115, 115, 115);
		for (float w = 0; w < newWid; w += drawTileSize.x)
		{
			ImVec2 ts(maxPos.x + w, maxPos.y);
			ImVec2 te(maxPos.x + w, maxPos.y + newHei);

			list->AddLine(ts, te, lineColor, 3);
		}
		ImVec2 ts(maxPos.x + newWid, maxPos.y);
		ImVec2 te(maxPos.x + newWid, maxPos.y + newHei);
		list->AddLine(ts, te, lineColor, 3);

		for (float h = 0; h < newHei; h += drawTileSize.y)
		{
			ImVec2 ts(maxPos.x, maxPos.y + h);
			ImVec2 te(maxPos.x + newWid, maxPos.y + h);
			list->AddLine(ts, te, lineColor, 3);
		}
		ImVec2 tse(maxPos.x, maxPos.y + newHei);
		ImVec2 tee(maxPos.x + newWid, maxPos.y + newHei);
		list->AddLine(tse, tee, lineColor, 3);
	}
};
