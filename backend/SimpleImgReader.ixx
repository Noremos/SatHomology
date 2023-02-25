module;

#include "../Bind/Common.h"

export module SimpleImgReaderModule;

import Platform;
import ImgReader;
import IOCore;


export class SimpleImgReader : public ImageReader
{
	BackImage main;
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
	bool open(const std::string& path) override;
	void close() override;
	ImageType getType() override;
	int getSamples() override;
	int width() override;
	int height() override;
	float getNullValue() override;
	DataRect getRect(int stX, int stRow, int wid, int hei) override;
};


rowptr SimpleImgReader::getRowData(int ri)
{
	rowptr ptr;
	ptr.samples = samples;
	ptr.count = main.width();
	ptr.deletePtr = false;
	ptr.type = type;
	ptr.ptr.b = main.lineOffet(ri);

	return ptr;
}

bool SimpleImgReader::open(const std::string& path)
{
	main = imread(path);
	samples = main.channels();
	type = ImageType::int8;

	//switch (samples)
	//{
	//case 1:
	//	type = ImageType::int8;
	//	break;
	//case 3:
	//	type = ImageType::rgb8;
	//	break;
	//default:
	//	assert(false);
	//	break;
	//}

	ready = true;
	return true;
//	MatrImg e(img.width(), img.height(), i);

//	int lineLen = e.bytesInLine();
//	for (int y = 0; y < img.height(); y++)
//	{
//		memcpy(e.lineOffet(y), img.scanLine(y), lineLen);
//	}
//	return e;
}

void SimpleImgReader::close()
{
	//main = BackImage();
}

ImageType SimpleImgReader::getType()
{
	return type;
}

int SimpleImgReader::getSamples()
{
	return samples;
}

int SimpleImgReader::width()
{
	return main.width();
}

int SimpleImgReader::height()
{
	return main.height();
}

float SimpleImgReader::getNullValue()
{
	return 0;
}

DataRect SimpleImgReader::getRect(int stX, int stRow, int wid, int hei)
{
	if (stX + wid > main.width())
		wid = main.width() - stX;
	if (stRow + hei > main.height())
		hei = main.height() - stRow;

	DataRect rect(wid, hei, type, samples);
	for (int y = 0; y < hei; ++y)
	{
		rect.setInRow(y, 0, main.lineOffet(stRow + y, stX), wid);
	}

	return rect;
}
