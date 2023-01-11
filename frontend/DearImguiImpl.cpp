#include "Framework.h"
#include "GuiCommon.h"

// Side
#include "../side/PortFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../side/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../side/stb_image_write.h"
#include <imgui_impl_opengl3_loader.h>

// ----------------

#define GL_RGB                            0x1907

using namespace Plarform;

BackImage imread(BackString path)
{
	int width, height, chls;
	unsigned char* image_data = stbi_load(path.c_str(), &width, &height, &chls, 0);
	if (image_data == NULL)
		return BackImage(0, 0, 0, NULL, false, false);

	return BackImage(width, height, chls, image_data, false, true);;
}

void imwrite(const BackString& path, const BackImage& mat)
{
	stbi_write_png(path.c_str(), mat.width(), mat.height(), mat.channels(), mat.data, mat.width() * mat.channels());
}

void imwrite(const BackPathStr& path, const BackImage& mat)
{
	stbi_write_png((char*)path.c_str(), mat.width(), mat.height(), mat.channels(), mat.data, mat.width() * mat.channels());
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


bool GuiImage::setSource(const BackPathStr& path)
{
	unsigned char* image_data = stbi_load((const char*)path.c_str(), &width, &height, NULL, 4);
	if (image_data == NULL)
		return false;

	makeTexture(image_data);
	stbi_image_free(image_data);

	return true;
}

void GuiImage::makeTexture(unsigned char* image_data)
{
	// Create a OpenGL texture identifier
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
}
