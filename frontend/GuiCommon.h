#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../backend/BackImage.h"
 
typedef unsigned int GLuint;

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

	void setImage(const BackImage& img, bool smooth = true)
	{
		width = img.width();
		height = img.height();
		makeTexture(img.data, img.channels(), smooth);
	}

	void release();

    GLuint getTextureId()
    {
        auto ptr = textureId.get();
        return ptr ? *ptr : 0;
    }

    void* getTexturePtr()
    {
        return (void*)(intptr_t)getTextureId();
    }

private:
    std::shared_ptr<GLuint> textureId;
	void makeTexture(unsigned char* image_data, int comp, bool smooth = true);
};

class GuiItem
{

};
