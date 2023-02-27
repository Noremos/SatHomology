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


export BackJson jsonFromFile(const BackPathStr& path)
{
	BackString temp;

	BackFileReader file;
	file.open(path);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open a file.";
		return BackJson();
	}
	file >> temp;
	file.close();

	return BackJson::parse(temp);
}
export void jsonToFile(const BackJson& json, const BackPathStr& path)
{
	BackString temp;

	BackFileWriter file;
	file.open(path, std::ios::trunc);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open a file.";
		return;
	}
	file << json.dump();
	file.close();
}

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
