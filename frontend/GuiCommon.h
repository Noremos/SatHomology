#pragma once
#include "../backend/BackImage.h"
 
typedef unsigned int GLuint;

void ResizeImage(int& width, int& height, int max_width, int max_height);

class GuiImage
{
public:
	virtual ~GuiImage()
	{
		release();
	}

	// Load from file  
	int width = 0;
	int height = 0;
	GLuint textureId;

	//int displayWidht = 0;
	//int displayHeight = 0;

	float scaleFactor = 1;

	bool setSource(const BackPathStr& path);

	void setImage(const BackImage& img)
	{
		width = img.width();
		height = img.height();
		makeTexture(img.data, img.channels());
	}

	int getRealX(int x)
	{
		return x * scaleFactor;
	}
	int getRealY(int y)
	{
		return y * scaleFactor;
	}

	void release();
private:
	void makeTexture(unsigned char* image_data, int comp);
};

class GuiItem
{

};
