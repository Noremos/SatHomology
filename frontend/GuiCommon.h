#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../Bind/Common.h"

#include "imgui.h"
#include <imgui_internal.h>

#include <algorithm>

#include "../side/IconsFontAwesome5.h"

import MatrModule;
import BarcodeModule;

typedef unsigned int GLuint;

const int ImGuiWindowFlags_NoDocking = 0;

void ResizeImage(int& width, int& height, int max_width, int max_height);

struct TextureId
{
	GLuint textureId;
	~TextureId();
};

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
		scaleFactor = std::exchange(other.scaleFactor, 0);
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

		const float x_ratio = static_cast<float>(img.width() - 1) / (wid - 1);
		const float y_ratio = static_cast<float>(img.height() - 1) / (hei - 1);
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

				auto ewf = [](int p1, int p2, int p3, int p4)
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
