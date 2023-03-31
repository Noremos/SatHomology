#include "GuiCommon.h"

import SideBind;

#define GLEW_STATIC
#include "glew/GL/glew.h"


void ResizeImage(int& width, int& height, int max_width, int max_height)
{
	// Calculate the aspect ratio of the image
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

	// Calculate the maximum width and height that maintain the aspect ratio
	int max_aspect_width = std::round(max_height * aspect_ratio);
	int max_aspect_height = std::round(max_width / aspect_ratio);

	// Use the maximum width or height that maintains the aspect ratio, whichever is smaller
	width = std::min(max_width, max_aspect_width);
	height = std::min(max_height, max_aspect_height);
}

void ResizeImage(ImVec2& size, const ImVec2& maxSize)
{
	// Calculate the aspect ratio of the image
	float aspect_ratio = size.x / size.y;
	float max_aspect_width = maxSize.y * aspect_ratio;
	float max_aspect_height = maxSize.x / aspect_ratio;
	size.x = std::min(maxSize.x, max_aspect_width);
	size.y = std::min(maxSize.y, max_aspect_height);
}


bool GuiImage::setSource(const BackPathStr& path, bool smooth)
{
	int comp = 4;
	unsigned char* image_data = stbi_load(path.string().c_str(), &width, &height, NULL, 4);
	if (image_data == NULL)
		return false;

	makeTexture(image_data, comp, smooth);
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

void GuiImage::makeTexture(unsigned char* image_data, int comp, bool smooth)
{
	// Create a OpenGL texture identifier
	GLuint newtext;
	glGenTextures(1, &newtext);
	glBindTexture(GL_TEXTURE_2D, newtext);
	textureId = std::make_shared<TextureId>(newtext);

	if (smooth)
	{
		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	std::vector<unsigned char> rgba_image_data(width * height * 4);

	if (comp == 1)
	{
		size_t length = width * height;
		rgba_image_data.resize(length * 4);
		for (size_t i = 0; i < length; i++) {
			rgba_image_data[i * 4 + 0] = image_data[i * 3];
			rgba_image_data[i * 4 + 1] = image_data[i * 3];
			rgba_image_data[i * 4 + 2] = image_data[i * 3];
			rgba_image_data[i * 4 + 3] = 255;
		}

		image_data = rgba_image_data.data();
	}
	else if (comp == 3)
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
