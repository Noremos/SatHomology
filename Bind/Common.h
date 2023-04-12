#ifndef COMMON_H
#define COMMON_H

#include <filesystem>
//#include <random>
#include <fstream>


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

const int DEFAULT_PROJECTION = 4326;
const char* const DEFAULT_PROJECTION_STR = "4326";

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

template<typename T>
struct TPoint
{
	T x, y;

	TPoint(T _x = 0, T _y = 0) : x(_x), y(_y) {}

	bool operator==(const TPoint& other) const
	{
		return x == other.x && y == other.y;
	}

	// Point addition operator
	TPoint operator+(const TPoint& other) const {
		return TPoint(x + other.x, y + other.y);
	}

	// TPoint subtraction operator
	TPoint operator-(const TPoint& other) const {
		return TPoint(x - other.x, y - other.y);
	}

	TPoint operator*(const TPoint& other) const {
		return TPoint(x * other.x, y * other.y);
	}

	TPoint operator/(const TPoint& other) const {
		return TPoint(x / other.x, y / other.y);
	}

	// TPoint multiplication operator by scalar
	TPoint operator*(T scalar) const {
		return TPoint(x * scalar, y * scalar);
	}

	// TPoint division operator by scalar
	TPoint operator/(T scalar) const {
		return TPoint(x / scalar, y / scalar);
	}

	TPoint operator-(T scalar) const {
		return TPoint(x - scalar, y - scalar);
	}

	//// TPoint dot product operator
	//int operator*(const TPoint& other) const {
	//	return x * other.x + y * other.y;
	//}

	//// TPoint cross product operator
	//int operator^(const TPoint& other) const {
	//	return x * other.y - y * other.x;
	//}

	// TPoint magnitude
	double mag() const {
		return sqrt(x * x + y * y);
	}

	// Normalize the TPoint to unit magnitude
	TPoint norm() const {
		double m = mag();
		return TPoint((int)(x / m), (int)(y / m));
	}

	// Distance between two TPoints
	double dist(const TPoint& other) const {
		int dx = x - other.x;
		int dy = y - other.y;
		return sqrt(dx * dx + dy * dy);
	}
};

using BackPixelPoint = TPoint<int>;
using BackPoint = TPoint<double>;

class Variables
{
public:
	static BackPathStr rootPath;
	static BackPathStr metaPath;
	static BackString prodDbPath;
	static void setRoot(const char* arg)
	{
		rootPath = arg;
		rootPath = rootPath.parent_path();
		metaPath = rootPath / "meta";
		prodDbPath = (metaPath / "proj").string();
	}
};

#endif // !COMMON_H
