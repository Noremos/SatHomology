module;

#include <filesystem>
#include <string>
#include <string_view>
//#include <random>
#include <fstream>
export module BackBind;

export using BackStringView = std::string_view;
export using BackString = std::string;
export using BackDirStr = std::filesystem::path;
export using BackPathStr = std::filesystem::path;
export using BackFileReader = std::ifstream;
export using BackFileWriter = std::ofstream;

export using uint = unsigned int;
export using uchar = unsigned char;
export using ushort = unsigned short;



export BackString operator+(const BackString& left, BackStringView right)
{
	return left + right;
}

export BackString operator+(const BackString& left, const BackString& right)
{
	return left + right;
}

export bool StrEquals(const BackString& str, BackStringView view)
{
	return str == view;
}

export void WriteFile(const BackPathStr& fileName, const BackString& content)
{
	BackFileWriter w;
	w.open(fileName, std::ios::out | std::ios::trunc);
	w.write(content.c_str(), content.length());
	w.close();
}

export constexpr int DEFAULT_PROJECTION = 4326;
export constexpr const char* const DEFAULT_PROJECTION_STR = "4326";

export struct BackSize
{
	int wid, hei;
	BackSize(int _wid, int _hei)
	{
		wid = _wid;
		hei = _hei;
	}
};


export struct BackColor
{
	uchar r = 0;
	uchar g = 0;
	uchar b = 0;
	BackColor operator=(BackColor other)
	{
		r = other.r;
		g = other.g;
		b = other.b;
		return *this;
	}

	static BackColor random()
	{
		BackColor rc;
		rc.r = rand() % 255;
		rc.g = rand() % 255;
		rc.b = rand() % 255;
		return rc;
	}
};

export template<typename T>
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


	TPoint operator*(double scalar) const {
		return TPoint(static_cast<T>(x * scalar), static_cast<T>(y * scalar));
	}

	TPoint operator/(double scalar) const {
		return TPoint(static_cast<T>(x / scalar), static_cast<T>(y / scalar));
	}

	TPoint operator*(float scalar) const {
		return TPoint(static_cast<T>(x * scalar), static_cast<T>(y * scalar));
	}

	TPoint operator/(float scalar) const {
		return TPoint(static_cast<T>(x / scalar), static_cast<T>(y / scalar));
	}

	TPoint operator*(int scalar) const {
		return TPoint(static_cast<T>(x * scalar), static_cast<T>(y * scalar));
	}

	TPoint operator/(int scalar) const {
		return TPoint(static_cast<T>(x / scalar), static_cast<T>(y / scalar));
	}

	TPoint operator-(T scalar) const {
		return TPoint(static_cast<T>(x - scalar), static_cast<T>(y - scalar));
	}

	//// TPoint dot product operator
	//int operator*(const TPoint& other) const {
	//	return x * other.x + y * other.y;
	//}

	//// TPoint cross product operator
	//int operator^(const TPoint& other) const {
	//	return x * other.y - y * other.x;
	//}

	TPoint abs() const
	{
		return TPoint(std::abs(x), std::abs(y));
	}

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

export using BackPixelPoint = TPoint<int>;
export using BackPoint = TPoint<double>;

export class Variables
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

	static BackDirStr getFontsDir()
	{
		return rootPath / "fonts";
	}

	static BackPathStr getDefaultFontPath()
	{
		return getFontsDir() / "Arial.ttf";
	}
};

BackPathStr Variables::rootPath;
BackPathStr Variables::metaPath;
BackString Variables::prodDbPath;


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
	uint locIndex;

	TileIterator(uint start, uint tileSize, uint offset, uint maxLen) :
		start(start), tileSize(tileSize), fullTileSize(tileSize + offset), maxLen(maxLen),
		locIndex(0)
	{ }

	void reset(uint st = 0)
	{
		start = st;
		locIndex = start / tileSize;
	}

	uint pos()
	{
		return start;
	}

	uint accum()
	{
		++locIndex;
		start += tileSize;
		return start;
	}

	uint tilesInLine() const
	{
		return maxLen / tileSize + (maxLen % tileSize > (fullTileSize - tileSize) ? 1 : 0);
	}


	uint getFullTileSize()
	{
		if (start + fullTileSize <= maxLen)
		{
			return fullTileSize;
		}
		else if (maxLen > start)
		{
			return maxLen - start;
		}
		else
			return 0;
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


export bool pathExists(const BackPathStr& filePath)
{
	return std::filesystem::exists(filePath);
}

export template<class StrT>
bool mkdir(const StrT& path)
{
	BackDirStr filePath(path);
	return std::filesystem::create_directory(filePath);
}


export void mkDirIfNotExists(const BackDirStr& dirPath)
{
	if (!pathExists(dirPath))
	{
		mkdir(dirPath);
	}
}

export void dropDir(const BackDirStr& dirPath)
{
	std::filesystem::remove_all(dirPath);
}

export void dropDirIfExists(const BackDirStr& dirPath)
{
	if (pathExists(dirPath))
	{
		dropDir(dirPath);
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
