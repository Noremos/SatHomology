#pragma once
#include "../backend/Common.h"
#include "../backend/BackImage.h"


namespace Plarform
{
};

BackImage imread(const BackString& path);
BackImage imread(const BackPathStr& path);

void imwrite(const BackString& path, const BackImage& mat);
void imwrite(const BackPathStr& path, const BackImage& mat);
BackPathStr getSavePath(std::initializer_list<std::string> exts);
BackPathStr getDicumnetPath();

BackPathStr openImageOrProject();
BackPathStr openProject();
BackPathStr openImage();
