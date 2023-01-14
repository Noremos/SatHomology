#pragma once
#include "../backend/Common.h"
#include "../backend/BackImage.h"


namespace Plarform
{
};
BackImage imread(BackString path);

void imwrite(const BackString& path, const BackImage& mat);
void imwrite(const BackPathStr& path, const BackImage& mat);
BackPathStr openImageOrProject();
BackPathStr getSavePath(std::initializer_list<std::string> exts);

