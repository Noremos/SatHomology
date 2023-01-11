#ifndef SIMPLEIMGREADER_H
#define SIMPLEIMGREADER_H

#include "tiffreader.h"
#include "BackImage.h"

class SimpleImgReader : public ImageReader
{
	MatrImg main;
	ImageType type;
	int typeSize;
	int samples;
//	MatrImg main;
	// ImageReader interface
public:
	SimpleImgReader() : ImageReader()
	{
		isTile = false;
	}
	rowptr getRowData(int ri) override;
	bool open(const char *path) override;
	void close() override;
	ImageType getType() override;
	int getSamples() override;
	int widght() override;
	int height() override;
	float getNullValue() override;
	DataRect getRect(int stX, int stRow, int wid, int hei) override;
};

#endif // SIMPLEIMGREADER_H
