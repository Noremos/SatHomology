#pragma once
#include <filesystem>
#include  <iostream>
#include <fstream>
#include "../backend/MatrImg.h"

#include "barline.h"

#include "../side/json.hpp"



using BackImage = MatrImg;
using BackString = std::string;
using BackDirStr = std::filesystem::path;
using BackPathStr = std::filesystem::path;
using BackFileReader = std::ifstream;
using BackFileWrited = std::ofstream;


using BackJson = nlohmann::json;
using JsonObject = BackJson;
using JsonArray = BackJson;
static BackJson jsonFromFile(const BackPathStr& path)
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

static void jsonToFile(const BackJson& json, const BackPathStr& path)
{
	BackString temp;

	BackFileWrited file;
	file.open(path, std::ios::trunc);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open a file.";
		return;
	}
	file << json.dump();
	file.close();
}


template<class T>
using BackVector = std::vector<T>;

struct Cound;


static int strToInt(const BackString& string, bool &ready)
{
	char* endptr;
	int numI = std::strtol(string.c_str(), &endptr, 10);
	ready = *endptr == '\0';

	return numI;
}

static int strToInt(const BackString& string)
{
	char* endptr;
	return std::strtol(string.c_str(), &endptr, 10);
}

template<class T>
static BackString intToStr(T value)
{
	return std::to_string(value);
}

template<class StrT>
static bool pathExists(const StrT& path)
{
	std::filesystem::path filePath(path);
	return std::filesystem::exists(filePath);
}

template<class StrT>
static bool mkdir(const StrT& path)
{
	std::filesystem::path filePath(path);
	return std::filesystem::create_directory(filePath);
}


static void mkDirIfNotExists(const BackDirStr& dirPath)
{
	if (!pathExists(dirPath))
	{
		mkdir(dirPath);
	}
}

static bool endsWith(const BackString& string, const BackString endl)
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


struct SimpleLine
{
	int id, barlineIndex;
	SimpleLine(int id = 0, int barlineIndex = 0) : id(id), barlineIndex(barlineIndex), parent(nullptr), start(0), end(0), depth(0)
	{}
	bc::barvector matr;
	SimpleLine* parent;
	//	ushort counter = 0;

	Barscalar start, end;

	int depth;
	int matrSrcSize;

	int getDeath()
	{
		return depth;
	}
};


struct BackSize
{
	int wid, hei;
	BackSize(int _wid, int _hei)
	{
		wid = _wid;
		hei = _hei;
	}
};

struct FilterInfo
{
	struct FRange
	{
		int first;
		int second;

		//template<class T>
		//bool inRange(const T& val) const
		//{
		//	return val >= first && val <= second;
		//}

		bool inRange(int val) const
		{
			return val >= first && val <= second;
		}

		bool inRange(const Barscalar& val) const
		{
			return val >= first && val <= second;
		}

		bool notInRange(int val) const
		{
			return val < first || val > second;
		}

		bool notInRange(const Barscalar& val) const
		{
			return val < first && val > second;
		}
	};

	FRange start{0,255};
	FRange len{ 0,255 };
	FRange matrSizeProc{ 0,100 };
	FRange depth{0,1000};
	int imgLen = 0;

	bool needSkip(bc::barline* line) const
	{
		return start.notInRange(line->start) ||
				len.notInRange(line->len()) || 
				matrSizeProc.notInRange(line->matr.size() * 100 / imgLen) ||
				depth.notInRange(line->getDeath());
	}
};

//struct FileBuffer
//{
//private:
//	BackString buffer;
//	QFile outfile;
//	QTextStream* stream = nullptr;
//	int maxBufferSize = 10000;
//public:
//	bool openFileStream(BackString path, int maxBufferSize = 10000, QFile::OpenMode openFlags = QFile::WriteOnly | QFile::Truncate)
//	{
//		outfile.setFileName(path);
//
//		//		if (outfile.exists())
//		//		{
//		//			QFile::rename(path,  path.replace("bds.lst", "bds old.lst"));
//		//			//			outfile.setFileName(path);
//		//		}
//
//		if (!outfile.open(openFlags))
//			return false;
//
//		if (stream != nullptr)
//			delete stream;
//		stream = new QTextStream(&outfile);
//		this->maxBufferSize = maxBufferSize;
//
//		return true;
//	}
//
//	void write(const BackString& part)
//	{
//		if (stream == nullptr)
//			return;
//
//		buffer.append(part);
//		if (buffer.size() > maxBufferSize)
//		{
//			dowrite();
//		}
//	}
//
//	void writeLine(const BackString& part = "")
//	{
//		if (stream == nullptr)
//			return;
//
//		buffer.append(part);
//		buffer.append(nl);
//		if (buffer.size() > maxBufferSize)
//		{
//			dowrite();
//		}
//	}
//
//	void dowrite()
//	{
//		stream->operator<<(buffer);
//		buffer.clear();
//	}
//	void close()
//	{
//		if (stream != nullptr)
//		{
//			if (outfile.isOpen())
//			{
//				dowrite();
//				outfile.close();
//			}
//
//			delete stream;
//			stream = nullptr;
//		}
//	}
//	~FileBuffer()
//	{
//		close();
//	}
//};

