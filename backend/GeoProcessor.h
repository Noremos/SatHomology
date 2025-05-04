// module;
// Std
#pragma once
#include <random>
#include <vector>
#include <iostream>
#include <stack>
#include <ranges>
#include <unordered_set>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include "shapefil.h"


#include "Barcode/PrjBarlib/include/barline.h"
#include "Usings.h"

#include "MatrImg.h"
#include "../Bind/MHashMap.h"
#include "../Bind/Framework.h"
// export module GeoprocessorModule;

//import BackBind;
// import Platform;
// import BarTypes;
// import MHashMap;
//import BackBind;
// import MatrModule;



MEXPORT struct CounturRect
{
	BackPoint topLeft;
	BackPoint bottomRight;

	BackPoint topRight()
	{
		return {bottomRight.x + 1, topLeft.y};
	}

	BackPoint bottomLeft()
	{
		return {topLeft.x, bottomRight.y + 1};
	}

	BackPoint getBottomRight()
	{
		return {bottomRight.x + 1, bottomRight.y + 1};
	}
};


MEXPORT struct DictWrap
{
	std::unordered_set<buint> points;
	int wid = 0, hei = 0;

	void set(buint i)
	{
		points.insert(i);
	}

	bool has(buint i) const
	{
		return points.find(i) != points.end();
	}

	bool has(int x, int y) const
	{
		if (x < 0 || y < 0)
			return false;
		return points.find(bc::barvalue::getStatInd(x, y)) != points.end();
	}

	bool hasCorners(int x, int y) const
	{
		return (has(x - 1, y) && has(x - 1, y - 1) && has(x, y - 1) && has(x, y + 1) && has(x + 1, y + 1));
	}
};

MEXPORT using mcountor = std::vector<buint>;

MEXPORT inline CounturRect getCountour(const bc::barvector& points, mcountor& contur, bool aproximate = false);
CounturRect getCountour(const bc::barvector& points, mcountor& contur, bool aproximate);
CounturRect getCountourOder(const bc::barvector& points, mcountor& contur, bool aproximate);

void getCountourSimple(const bc::barvector& points, bc::barvector& contur, float changeFactor);

void getCircle(const bc::barvector& points, BackPoint& center, float& r);
void getRect(const bc::barvector& points, BackPoint& topLeft, BackPoint& bottomRight);


class ShapeFile
{
	SHPHandle shpHandle;
	DBFHandle dbfHandle;

public:
	ShapeFile(std::string_view path);

	void writePolygonRecord(const bc::barline& polygon);

	void close();

	~ShapeFile();
};
