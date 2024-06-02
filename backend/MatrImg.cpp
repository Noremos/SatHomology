#include "MatrImg.h"

#include "fpng/fpng.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include "stb_image_write.h"



void FrameworkInit()
{
	fpng::fpng_init();
}

MEXPORT BackImage imread(const BackString& path)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load(path.c_str(), &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	// if (chls == 4)
	// {
	// 	const int req = 3;
	// 	unsigned int length = width * height;
	// 	unsigned char* fixedData = new buchar[length * req];
	// 	//memcpy(fixedData, image_data, length * req);
	// 	for (size_t i = 0, destinationIndex = 0; i < length * chls; i += chls, destinationIndex += req)
	// 	{
	// 		fixedData[destinationIndex + 0] = image_data[i + 0];
	// 		fixedData[destinationIndex + 1] = image_data[i + 1];
	// 		fixedData[destinationIndex + 2] = image_data[i + 2];

	// 		/*for (size_t j = 0; j < i + req; ++j)
	// 		{
	// 			fixedData[j] = image_data[destinationIndexj];
	// 		}*/
	// 	}
	// 	delete[] image_data;
	// 	image_data = fixedData;
	// 	chls = req;
	// }
	return BackImage(width, height, chls, image_data, false, true);
}

BackImage imread(const BackPathStr& path)
{
	return imread(path.string());
}

void imwrite(const BackString& path, const BackImage& mat)
{
	fpng::fpng_encode_image_to_file(path.c_str(), mat.data, mat.width(), mat.height(), mat.channels());
	// stbi_write_png(path.c_str(), mat.width(), mat.height(), mat.channels(), mat.data, 0); // so slow...
}

void imwrite(const BackPathStr& path, const BackImage& mat)
{
	imwrite(path.string(), mat);
}


BackImage imreadFromMemory(const buchar* data, size_t size)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load_from_memory(data, size, &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	if (chls == 4)
	{
		const int req = 3;
		unsigned int length = width * height;
		unsigned char* fixedData = new buchar[length * req];
		//memcpy(fixedData, image_data, length * req);
		for (size_t i = 0, destinationIndex = 0; i < length * chls; i += chls, destinationIndex += req)
		{
			fixedData[destinationIndex + 0] = image_data[i + 0];
			fixedData[destinationIndex + 1] = image_data[i + 1];
			fixedData[destinationIndex + 2] = image_data[i + 2];
		}
		delete[] image_data;
		image_data = fixedData;
		chls = req;
	}
	return BackImage(width, height, chls, image_data, false, true);
}

MemImgData imwriteToMemory(const BackImage& mat)
{
	MemImgData out_buf;
	[[maybe_unused]]
	bool r = fpng::fpng_encode_image_to_memory(mat.getData(), mat.width(), mat.height(), mat.channels(), out_buf);
	assert(r);
	return out_buf;
}