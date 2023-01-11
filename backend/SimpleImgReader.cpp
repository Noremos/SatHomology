#include "SimpleImgReader.h"
#include "../frontend/Framework.h"

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

bool SimpleImgReader::open(const char *path)
{
	main = imread(path);
	samples = main.channels();
	switch (samples)
	{
	case 1:
		type = ImageType::int8;
		break;
	case 3:
		type = ImageType::rdb8;
		break;
	default:
		assert(false);
		break;
	}

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

int SimpleImgReader::widght()
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
	int rectOffset = 0;
	for (int y = 0; y < hei; ++y, rectOffset += wid)
	{
		rect.setInRow(rectOffset, 0, main.lineOffet(y) + stX, wid);
	}

	return rect;
}
