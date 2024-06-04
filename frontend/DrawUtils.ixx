module;
#include <algorithm>
#include <memory>
#include <cmath>
#include <vector>

#include "DrawCommon.h"
#include <glew.h>

#include "stb_image.h"

#include "Barcode/PrjBarlib/include/barscalar.h"


#include "../backend/CSBind.h"
#include "../backend/MatrImg.h"
#include "../Bind/Framework.h"
export module DrawUtils;

// import Platform;
// import MatrModule;
// import CSBind;
//import BackBind;
// import BarScalarModule;

export void ResizeImage(int& width, int& height, int max_width, int max_height)
{
	// Calculate the aspect ratio of the image
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

	// Calculate the maximum width and height that maintain the aspect ratio
	int max_aspect_width = std::round(max_height * aspect_ratio);
	int max_aspect_height = std::round(max_width / aspect_ratio);

	// Use the maximum width or height that maintains the aspect ratio, whichever is smaller
	width = std::min(max_width, max_aspect_width);
	height = std::min(max_height, max_aspect_height);
}

export void ResizeImage(ImVec2& size, const ImVec2& maxSize)
{
	// Calculate the aspect ratio of the image
	float aspect_ratio = size.x / size.y;
	float max_aspect_width = maxSize.y * aspect_ratio;
	float max_aspect_height = maxSize.x / aspect_ratio;
	size.x = std::min(maxSize.x, max_aspect_width);
	size.y = std::min(maxSize.y, max_aspect_height);
}

export void ResizeImage(BackPoint& size, const BackPoint& maxSize)
{
	// Calculate the aspect ratio of the image
	double aspect_ratio = size.x / size.y;
	double max_aspect_width = maxSize.y * aspect_ratio;
	double max_aspect_height = maxSize.x / aspect_ratio;
	size.x = std::min(maxSize.x, max_aspect_width);
	size.y = std::min(maxSize.y, max_aspect_height);
}


export using ApplicationVec2 = ImVec2;
export using WindowVec2 = ImVec2; // The top-left is a begin window
export using ItemVec2 = ImVec2; // The top-left is an Item (image) coords

export struct TextureId
{
	GLuint textureId;
	~TextureId()
	{
		glDeleteTextures(1, &textureId);
	}
};

export inline ImVec2 toIV(const BackPoint& p)
{
	return ImVec2(p.x, p.y);
}

export BackPoint toBP(const ImVec2& p)
{
	return BackPoint(p.x, p.y);
}



export class GuiImage
{
public:
	GuiImage() : textureId(nullptr)
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
		scaleFactor = std::exchange(other.scaleFactor, 0.f);

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


	bool setSource(const BackPathStr& path, bool smooth = true)
	{
		int comp = 4;
		unsigned char* image_data = stbi_load(path.string().c_str(), &width, &height, NULL, 4);
		if (image_data == NULL)
			return false;

		makeTexture(image_data, comp, smooth);
		stbi_image_free(image_data);

		return true;
	}

	void setImage(const BackImage& img, bool smooth = true)
	{
		width = img.width();
		height = img.height();
		makeTexture(img.data, img.channels(), smooth);
	}

	void setImage(const BackImage& img, int wid, int hei, bool smooth = true)
	{
		width = wid;
		height = hei;
		BackImage newImg(wid, hei, img.channels());

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
					return static_cast<buchar>(std::max(std::min(a, 255), 0));
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

	void release()
	{
		textureId.reset();
	}

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
	void makeTexture(unsigned char* image_data, int comp, bool smooth = true)
	{
		// Create a OpenGL texture identifier
		GLuint newtext;
		glGenTextures(1, &newtext);
		glBindTexture(GL_TEXTURE_2D, newtext);
		textureId = std::make_shared<TextureId>(newtext);

		if (smooth)
		{
			// Setup filtering parameters for display
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		std::vector<unsigned char> rgba_image_data(width * height * 4);

		if (comp == 1)
		{
			size_t length = width * height;
			rgba_image_data.resize(length * 4);
			for (size_t i = 0; i < length; i++){
				const auto& val = image_data[i * 3];
				rgba_image_data[i * 4 + 0] = val;
				rgba_image_data[i * 4 + 1] = val;
				rgba_image_data[i * 4 + 2] = val;
				rgba_image_data[i * 4 + 3] = 255;
			}

			image_data = rgba_image_data.data();
		}
		else if (comp == 3)
		{
			size_t length = width * height;
			rgba_image_data.resize(length * 4);
			for (size_t i = 0; i < length; i++) {
				rgba_image_data[i * 4 + 0] = image_data[i * 3 + 0];
				rgba_image_data[i * 4 + 1] = image_data[i * 3 + 1];
				rgba_image_data[i * 4 + 2] = image_data[i * 3 + 2];
				rgba_image_data[i * 4 + 3] = 255;
			}

			image_data = rgba_image_data.data();
		}

		// Upload pixels into texture
	#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	}

};

export class GuiItem
{

};

export class GuiDisplaySystem
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
