module;

#include "Common.h"
#include <iostream>

export module IOCore;

import MatrModule;

export
{
	using BackImage = MatrImg;
}

export struct toStdStr
{
	using STRTYPE = std::string;

	template<class T>
	static STRTYPE toStr(T val)
	{
		return std::to_string(val);
	}
};


export struct TileIterator
{
	uint start;
	uint tileSize;
	uint fullTileSize;

	uint maxLen;

	TileIterator(uint start, uint tileSize, uint offset, uint maxLen) :
		start(start), tileSize(tileSize), fullTileSize(tileSize + offset), maxLen(maxLen)
	{ }

	void reset(uint st = 0)
	{
		start = st;
	}

	uint pos()
	{
		return start;
	}

	uint accum()
	{
		start += tileSize;
		return start;
	}


	uint getFullTileSize()
	{
		return (start + fullTileSize <= maxLen ? fullTileSize : maxLen - start);
	}

	bool shouldSkip(uint& len)
	{
		len = getFullTileSize();
		return len <= (fullTileSize - tileSize); // len < offset
	}

	bool notEnded()
	{
		return start < maxLen;
	}
};

export int strToInt(const BackString& string, bool& ready)
{
	char* endptr;
	int numI = std::strtol(string.c_str(), &endptr, 10);
	ready = *endptr == '\0';

	return numI;
}
export int strToInt(const BackString& string)
{
	char* endptr;
	return std::strtol(string.c_str(), &endptr, 10);
}

export template<class T>
BackString intToStr(T value)
{
	return std::to_string(value);
}

export template<class StrT>
bool pathExists(const StrT& path)
{
	std::filesystem::path filePath(path);
	return std::filesystem::exists(filePath);
}

export template<class StrT>
bool mkdir(const StrT& path)
{
	std::filesystem::path filePath(path);
	return std::filesystem::create_directory(filePath);
}


export void mkDirIfNotExists(const BackDirStr& dirPath)
{
	if (!pathExists(dirPath))
	{
		mkdir(dirPath);
	}
}

export bool endsWith(const BackString& string, const BackString endl)
{
	signed long long pos = string.length() - endl.length();
	if (pos <= 0)
		return false;

	for (size_t i = 0; i < endl.length(); i++)
	{
		if (string[pos + i] != endl[i])
			return false;
	}

	return true;
}
