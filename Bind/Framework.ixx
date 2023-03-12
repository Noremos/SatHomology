module;

#include "../Bind/Common.h"

// Side
#include "PortFileDialog.h"
#include "sago/platform_folders.h"

export module Platform;
import SideBind;
import IOCore;
import BackTypes;

export
{
	BackImage imread(const BackString& path);
	BackImage imread(const BackPathStr& path);
	BackImage imreadFromMemory(const uchar* data, size_t size);

	void imwrite(const BackString& path, const BackImage& mat);
	void imwrite(const BackPathStr& path, const BackImage& mat);
	vbuffer imwriteToMemory(const BackImage& mat);

	BackPathStr getSavePath(std::initializer_list<std::string> exts);
	BackPathStr getDicumnetPath();

	BackPathStr openImageOrProject();
	BackPathStr openProject();
	BackPathStr openImage();
}


//#define GL_RGB                            0x1907

BackImage imread(const BackString& path)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load(path.c_str(), &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	if (chls == 4)
	{
		const int req = 3;
		unsigned int length = width * height;
		unsigned char* fixedData = new uchar[length * req];
		//memcpy(fixedData, image_data, length * req);
		for (size_t i = 0, destinationIndex = 0; i < length * chls; i += chls, destinationIndex += req)
		{
			fixedData[destinationIndex + 0] = image_data[i + 0];
			fixedData[destinationIndex + 1] = image_data[i + 1];
			fixedData[destinationIndex + 2] = image_data[i + 2];

			/*for (size_t j = 0; j < i + req; ++j)
			{
				fixedData[j] = image_data[destinationIndexj];
			}*/
		}
		delete[] image_data;
		image_data = fixedData;
		chls = req;
	}
	return BackImage(width, height, chls, image_data, false, true);
}

BackImage imread(const BackPathStr& path)
{
	return imread(path.string());
}

void imwrite(const BackString& path, const BackImage& mat)
{
	stbi_write_png(path.c_str(), mat.width(), mat.height(), mat.channels(), mat.data, 0);
}

void imwrite(const BackPathStr& path, const BackImage& mat)
{
	imwrite(path.string(), mat);
}


BackImage imreadFromMemory(const uchar* data, size_t size)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load_from_memory(data, size, & width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	if (chls == 4)
	{
		const int req = 3;
		unsigned int length = width * height;
		unsigned char* fixedData = new uchar[length * req];
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

vbuffer imwriteToMemory(const BackImage& mat)
{
	int outLen = 0;
	vbuffer buff;
	auto data = stbi_write_png_to_mem(mat.getData(), 0, mat.width(), mat.height(), mat.channels(), &outLen);
	assert(outLen != 0);
	buff.setData(data, outLen);
	return buff;
}

BackPathStr openImageOrProject()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{ "All Files", "*",
		"Images (.png .jpg, .tif)", "*.png *.jpg *.tiff *tif",
		"Project (.qwr)", "*.qwr",
		},
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}

BackPathStr openImage()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{
			"Images (.png .jpg, .tif)", "*.png *.jpg *.tiff *tif",
			"All Files", "*"
		},
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}


BackPathStr openProject()
{
	// File open
	auto f = pfd::open_file("Choose files to read", pfd::path::home(),
		{
			"Project (.qwr)", "*.qwr",
			"All Files", "*"
		},
		pfd::opt::none);

	std::cout << "Selected files:";
	for (auto const& name : f.result())
		std::cout << " " + name;
	std::cout << "\n";

	return f.result().size() == 0 ? "" : f.result()[0];
}

BackPathStr getSavePath(std::initializer_list<std::string> exts)
{
	auto fs = pfd::save_file("Choose file to save", pfd::path::home(), exts, pfd::opt::none);

	std::cout << "File path:" << " " + fs.result() << std::endl;

	return fs.result().size() == 0 ? "" : fs.result();
}

BackPathStr getDicumnetPath()
{
	return sago::getDocumentsFolder();
}
