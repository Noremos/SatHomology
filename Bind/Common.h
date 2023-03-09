#pragma once
#include <filesystem>
//#include <random>
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
using uchar = unsigned char;

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


struct BackColor
{
	uchar r = 0;
	uchar g = 0;
	uchar b = 0;

	static BackColor random()
	{
		BackColor rc;
		rc.r = rand() % 255;
		rc.g = rand() % 255;
		rc.b = rand() % 255;
		return rc;
	}
};
