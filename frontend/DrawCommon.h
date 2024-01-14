#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../Bind/Common.h"

#include "imgui.h"
#include <imgui_internal.h>

#include <algorithm>

#include "IconsFontAwesome5.h"

import MatrModule;
import BarcodeModule;
import CSBind;

typedef unsigned int GLuint;

const int ImGuiWindowFlags_NoDocking = 0;

void ResizeImage(int& width, int& height, int max_width, int max_height);
void ResizeImage(ImVec2& size, const ImVec2& maxSize);
void ResizeImage(BackPoint& size, const BackPoint& maxSize);


using ApplicationVec2 = ImVec2;
using WindowVec2 = ImVec2; // The top-left is a begin window
using ItemVec2 = ImVec2; // The top-left is an Item (image) coords

struct TextureId
{
	GLuint textureId;
	~TextureId();
};

inline ImVec2 toIV(const BackPoint& p)
{
	return ImVec2(p.x, p.y);
}

inline BackPoint toBP(const ImVec2& p)
{
	return BackPoint(p.x, p.y);
}

class GuiImage
{
public:
	GuiImage() : textureId(nullptr), width(0), height(0), scaleFactor(1)
	{ }
	// Copy constructor
	GuiImage(const GuiImage& other) noexcept
	{
		width = other.width;
		height = other.height;
		textureId = other.textureId;
		scaleFactor = other.scaleFactor;
	}

	// Copy assignment operator
	GuiImage& operator=(const GuiImage& other) noexcept
	{
		width = other.width;
		height = other.height;
		textureId = other.textureId;
		scaleFactor = other.scaleFactor;

		return *this;
	}

	// Move constructor
	GuiImage(GuiImage&& other) noexcept
	{
		width = std::exchange(other.width, 0);
		height = std::exchange(other.height, 0);
		textureId = std::move(other.textureId);
		scaleFactor = std::exchange(other.scaleFactor, 0.f);
	}

	// Move assignment operator
	GuiImage& operator=(GuiImage&& other) noexcept
	{
		width = std::exchange(other.width, 0);
		height = std::exchange(other.height, 0);
		textureId = std::move(other.textureId);
		scaleFactor = std::exchange(other.scaleFactor, 0);

		return *this;
	}

	virtual ~GuiImage()
	{
		release();
	}

	// Load from file
	int width = 0;
	int height = 0;

	//int displayWidht = 0;
	//int displayHeight = 0;

	float scaleFactor = 1;


	bool setSource(const BackPathStr& path, bool smooth = true);

	void setImage(const MatrImg& img, bool smooth = true)
	{
		width = img.width();
		height = img.height();
		makeTexture(img.data, img.channels(), smooth);
	}

	void setImage(const MatrImg& img, int wid, int hei, bool smooth = true)
	{
		width = wid;
		height = hei;
		MatrImg newImg(wid, hei, img.channels());

		const float x_ratio = static_cast<float>(img.width()) / (wid);
		const float y_ratio = static_cast<float>(img.height()) / (hei);
		for (int y = 0; y < hei; ++y)
		{
			for (int x = 0; x < wid; ++x)
			{
				// Compute the corresponding pixel coordinates in the original image
				int x_floor = static_cast<int>(x * x_ratio);
				int y_floor = static_cast<int>(y * y_ratio);
				int x_ceil = (std::min)(static_cast<int>((x + 1) * x_ratio - 1), img.width() - 1);
				int y_ceil = (std::min)(static_cast<int>((y + 1) * y_ratio - 1), img.height() - 1);

				Barscalar p1s = img.get(x_floor, y_floor);
				Barscalar p2s = img.get(x_ceil, y_floor);
				Barscalar p3s = img.get(x_floor, y_ceil);
				Barscalar p4s = img.get(x_ceil, y_ceil);

				const auto ewf = [](int p1, int p2, int p3, int p4)
				{
					int a = static_cast<int>((float)p1 * 0.25 + p2 * 0.25 + p3 * 0.25 + p4 * 0.25);
					return static_cast<uchar>(MAX(MIN(a, 255), 0));
				};

				Barscalar p_interp;
				if (img.channels() == 1)
				{
					p_interp = ewf(p1s[0], p2s[0], p3s[0], p4s[0]);
				}
				else
				{
					p_interp = {
						ewf(p1s[0], p2s[0], p3s[0], p4s[0]),
						ewf(p1s[1], p2s[1], p3s[1], p4s[1]),
						ewf(p1s[2], p2s[2], p3s[2], p4s[2]),
					};
				}

				newImg.set(x, y, p_interp);
			}
		}

		makeTexture(newImg.data, newImg.channels(), smooth);
	}

	void release();

	GLuint getTextureId()
	{
		auto ptr = textureId.get();
		return ptr ? ptr->textureId: 0;
	}

	void* getTexturePtr()
	{
		return (void*)(intptr_t)getTextureId();
	}

private:
	std::shared_ptr<TextureId> textureId;
	void makeTexture(unsigned char* image_data, int comp, bool smooth = true);
};

class GuiItem
{

};

class GuiDisplaySystem
{
public:
	DisplaySystem& core;
	GuiDisplaySystem(DisplaySystem& core) :
		core(core)
	{ }

	BackPoint drawPos;
	BackPoint drawSize;
	BackPoint cursorPos;

	// S = D * scale
	// D = S / scale
	BackPoint getSysSize() const
	{
		return core.toSysGlobRelative(drawSize);
	}

	// T getDrawPos() const
	// {
	// 	return T(displayPos.x, displayPos.y);
	// }

	// template<class T>
	// T getDrawPos() const
	// {
	// 	return T(displaySize.x, displaySize.y);
	// }

	ImVec2 getWinPos() const
	{
		return ImVec2(static_cast<float>(drawPos.x), static_cast<float>(drawPos.y));
	}

	ImVec2 getDrawPos() const
	{
		return ImVec2(static_cast<float>(drawPos.x), static_cast<float>(drawPos.y));
	}

	ImVec2 getDrawSize() const
	{
		return ImVec2(static_cast<float>(drawSize.x), static_cast<float>(drawSize.y));
	}

	ImVec2 projItemGlobToDisplay(const CSBinding& itemCs, ImVec2 itemPos) const
	{
		return projItemGlobToDisplay(itemCs, BackPoint(itemPos.x, itemPos.y));
	}

	ImVec2 projItemGlobToDisplay(const CSBinding& itemCs, const BackPoint& itemPos) const
	{
		auto ps = core.sysProj.getThisProj(itemCs.proj, itemPos, true);
		return sysglobToDisplay(ps);
	}

	//BackPixelPoint projItemGlobToLoc(const CSBinding& itemCs, BackPoint itemPos) const
	//{
	//	auto ps = sysCs.getThisProj(itemCs, { itemPos.x, itemPos.y });
	//	return sysCs.toLocal(ps);
	//}

	BackPoint projItemLocalToSys(const CSBinding& itemCs, const BackPixelPoint& itemPos) const
	{
		BackPoint bp = itemCs.toGlobal(itemPos.x, itemPos.y);
		return core.sysProj.getThisProj(itemCs.proj, bp, true);
	}

	ImVec2 projItemLocalToDisplay(const CSBinding& itemCs, const BackPixelPoint& itemPos) const
	{
		BackPoint bp = itemCs.toGlobal(itemPos.x, itemPos.y);
		auto ps = core.sysProj.getThisProj(itemCs.proj, bp, true);
		return sysglobToDisplay(ps);
	}


	ImVec2 getDisplayStartPos() const
	{
		return sysglobToDisplay(core.csPos);// projItemLocalToDisplay(itemCs, BackPixelPoint(0, 0));
	}

	ImVec2 getDisplayEndPos() const
	{
		return sysglobToDisplay(core.csPos + getSysSize()); //projItemLocalToDisplay(itemCs, BackPixelPoint(drawSize.x, drawSize.y));
	}

	BackPoint getSysToItemStartPos(const CSBinding& itemCs) const
	{
		return itemCs.proj.getThisProj(core.sysProj, core.csPos, true);
	}

	BackPoint getSysToItemEndPos(const CSBinding& itemCs) const
	{
		return itemCs.proj.getThisProj(core.sysProj, core.csPos + getSysSize(), true);
	}

	ImVec2 sysglobToDisplay(const BackPoint& p) const
	{
		return toIV(core.toDisplay(p) + drawPos);
	}

	static bool inRange(const ImVec2& start, const ImVec2& end, const ImVec2& val)
	{
		if (val.x > end.x || val.y > end.y)
			return false;

		if (val.x < start.x || val.y < start.y)
			return false;

		return true;
	}

	static bool inRange(const BackPoint& start, const BackPoint& end, const BackPoint& val)
	{
		if (val.x > end.x || val.y > end.y)
			return false;

		if (val.x < start.x || val.y < start.y)
			return false;

		return true;
	}

	template <class T>
	static bool inRange(const T& start, const T& end, const T& valSt, const T& valEd)
	{
		if (valSt.x > end.x || valSt.y > end.y)
			return false;

		if (valEd.x < start.x || valEd.y < start.y)
			return false;

		return true;
	}

	bool inDisplayRange(const ImVec2& displayPos) const
	{
		auto start = getDisplayStartPos();
		auto end = getDisplayEndPos();

		return inRange(start, end, displayPos);
	}

	bool inSysRange(const BackPoint& poi) const
	{
		return inRange(core.csPos, core.csPos + getSysSize(), poi);
	}

	bool inSysRange(const BackPoint& st, const BackPoint& ed) const
	{
		return inRange(core.csPos, core.csPos + getSysSize(), st, ed);
	}

	//int getRealX(int x)
	//{
	//	return static_cast<float>(x - csPos) * (width / displaySize.x);
	//}


   //int toDisplayX(int x)
   //{
	  // return static_cast<float>(x) * (displaySize.x / width) + localDisplayPos.x;
   //}
   //int toDisplayY(int y)
   //{
   //	return static_cast<float>(y) * (displaySize.y / height) + localDisplayPos.y;
   //}
};