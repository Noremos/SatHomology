#pragma once
#include <filesystem>
#include <fstream>

#include "../side/emhash/hash_table5.hpp"
#define MMMAP emhash5::HashMap


#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

using BackString = std::string;
using BackDirStr = std::filesystem::path;
using BackPathStr = std::filesystem::path;
using BackFileReader = std::ifstream;
using BackFileWriter = std::ofstream;

using uint = unsigned int;

//template<class T>
//using BackVector = std::vector<T>;
//

struct BackSize
{
	int wid, hei;
	BackSize(int _wid, int _hei)
	{
		wid = _wid;
		hei = _hei;
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

