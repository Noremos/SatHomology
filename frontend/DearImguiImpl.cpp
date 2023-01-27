#include "Framework.h"
#include "GuiCommon.h"

// Side
#include "../side/PortFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../side/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../side/stb_image_write.h"
//#include <imgui_impl_opengl3_loader.h>
#define GLEW_STATIC
#include "../side/glew/GL/glew.h"
#include "../side/sago/platform_folders.h"

//#define GL_RGB                            0x1907

using namespace Plarform;

BackImage imread(const BackString& path)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load(path.c_str(), &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	if (chls == 4)
	{
		const int req = 3;
		uint length = width * height;
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
	return BackImage(width, height, chls, image_data, false, true);;
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


// -----

bool GuiImage::setSource(const BackPathStr& path)
{
	int comp = 4;
	unsigned char* image_data = stbi_load(path.string().c_str(), &width, &height, NULL, 4);
	if (image_data == NULL)
		return false;

	makeTexture(image_data, comp);
	stbi_image_free(image_data);

	return true;
}

void GuiImage::release()
{
	textureId.reset();
}

TextureId::~TextureId()
{
	glDeleteTextures(1, &textureId);
}

void GuiImage::makeTexture(unsigned char* image_data, int comp)
{
	// Create a OpenGL texture identifier
	GLuint newtext;
	glGenTextures(1, &newtext);
	glBindTexture(GL_TEXTURE_2D, newtext);
	textureId = std::make_shared<GLuint>(newtext);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	std::vector<unsigned char> rgba_image_data(width * height * 4);
	if (comp == 3)
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
