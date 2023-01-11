#pragma once
#include "MatrImg.h"
#include "Common.h"

using BackImage = MatrImg;
//
//class BackImage // : public MatrImg
//{
//public:
//	GLuint textureId;
//	int width, height, chls;
//
//	BackImage()
//	{
//		width = 0;
//		height = 0;
//		chls = 0;
//		textureId = 0;
//	}
//
//	BackImage(const MatrImg& inner)
//	{
//		load(inner);
//	}
//
//	BackImage(const BackString& path)
//	{
//		load(path);
//	}
//
//	bool load(const MatrImg& inner)
//	{
//		glGenTextures(1, &textureId);
//		glBindTexture(GL_TEXTURE_2D, textureId);
//
//		// Setup filtering parameters for display
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
//		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
//
//		// Upload pixels into texture
//#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
//		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
//#endif
//
//		int textGl = 0;
//		switch (inner.getType())
//		{
//		case BarType::BYTE8_1:
//			textGl = GL_DEPTH_COMPONENT;
//			break;
//		case BarType::BYTE8_3:
//			textGl = GL_RGB;
//		default:
//			break;
//		}
//
//		glTexImage2D(GL_TEXTURE_2D, 0, textGl, inner.wid(), inner.hei(), 0, textGl, GL_UNSIGNED_BYTE, inner.data);
//
//		return true;
//	}
//
//	void release()
//	{
//		glDeleteTextures(1, &textureId);
//	}
//
//	~BackImage()
//	{
//		release();
//	}
//};
