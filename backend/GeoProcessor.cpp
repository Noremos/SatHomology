#include "GeoProcessor.h"


struct Cound
{
	int total = 0;
	int srcTotal = 0;
	int index = 0;
};

struct ProcPoint
{
	float x, y;
};

using std::string;

double getPsa(const bc::barvector& matr)
{
	MMMAP<buint, bool> map;

	int minX = 10000, maxX = 0, minY = 1000, maxY = 0;
	for (const auto& pm : matr)
	{
		map[pm.getIndex()] = true;
		int xa = pm.getX();
		int ya = pm.getY();
		if (xa > maxX)
			maxX = xa;
		if (xa < minX)
			minX = xa;

		if (ya > maxY)
			maxY = ya;
		if (ya < minY)
			minY = ya;
	}

	double entr = 0;
	bool foundPrev = map.find(bc::barvalue::getStatInd(minX, minY)) != map.end();
	for (int x = minX; x < maxX; ++x)
	{
		double locEntr = 1.0;
		for (int y = minY; y < maxY; ++y)
		{
			bool foundCur = map.find(bc::barvalue::getStatInd(x, y)) != map.end();

			locEntr *= 0.5;
			if (foundPrev != foundCur)
			{
				entr += locEntr;
				locEntr = 1.0;
			}
		}

		if (locEntr != 1.0)
			entr += locEntr;
	}

	for (int y = minY; y < maxY; ++y)
	{
		double locEntr = 1.0;
		for (int x = minX; x < maxX; ++x)
		{
			bool foundCur = map.find(bc::barvalue::getStatInd(x, y)) != map.end();
			locEntr *= 0.5;
			if (foundPrev != foundCur)
			{
				entr += locEntr;
				locEntr = 1.0;
			}
		}

		if (locEntr != 1.0)
			entr += locEntr;
	}

	return entr;
}



void getMaskRes(const BackImage& matres, std::vector<Cound*>& veas, Cound** resmap)
{
	if (veas.size() == 0)
		return;

	for (int i = 0, total = veas.size(); i < total; ++i)
		veas[i]->total = 0;

	int bad = 0;
	for (int y = 0; y < matres.hei(); ++y)
	{
		for (int x = 0; x < matres.wid(); ++x)
		{
			int ind = y * matres.wid() + x;
			auto a = matres.get(x, y);
			Cound* b = resmap[ind];
			if (b)
			{
				if (a > 0)
					b->total++;
			}
			else if (a > 0)
				++bad;
		}
	}

	int fdo = 0;
	float proc = 0;
	for (int i = 0, total = veas.size(); i < total; ++i)
	{
		auto& d = veas[i];
		float p = static_cast<float>(d->total) / d->srcTotal;
		if (p >= 0.6)
			p = 1.0;

		proc += p;
		//*static_cast<float>(d->srcTotal) / matres.length();
		d->total = 0;
		float sadfe = 100.f * static_cast<float>(d->srcTotal) / matres.length();
		if (sadfe > 0.015)
			++fdo;
	}
	proc /= fdo;

	float ew = static_cast<float>(bad) / matres.length();
	std::cout << "Res src: " << proc << std::endl;
	std::cout << "Res bad: " << proc - ew << std::endl;
}

void getMaskRes(const BackImage& mat, BackImage& maskMat)
{
	if (maskMat.wid() == 1)
		return;

	int edd = 0;
	int edd1 = 0;
	int edd2 = 0;
	int tot1 = 0;
	int tot2 = 0;

	for (int y = 0; y < mat.hei(); ++y)
	{
		for (int x = 0; x < mat.wid(); ++x)
		{
			auto a = mat.get(x, y);
			auto b = maskMat.get(x, y);
			if (b > 0)
				++tot1;
			else if (b == 0)
				++tot2;

			if ((a > 0 && b > 0))
			{
				++edd;
				++edd1;
			}
			else if (a == 0 && b == 0)
			{
				++edd;
				++edd2;
			}
			else
			{
				--edd;
			}
		}
	}
	float proc = (static_cast<float>(mat.length()) + static_cast<float>(edd)) / (mat.length() * 2);
	std::cout << "Res counter: " << edd << "; porc: " << proc << std::endl;
	std::cout << "Res Hit: " << edd1 << "; porc: " << static_cast<float>(edd1) / tot1 << std::endl;
	std::cout << "Res Clear: " << edd2 << "; porc: " << static_cast<float>(edd2) / tot2 << std::endl;
}

#define DICT_EXISTS(DICT, X, Y) (DICT.find(bc::barvalue::getStatInd(X, Y)) != DICT.end())

class MapCountur
{
	int x = 0, y = 0;
	int stIndex = 0;
	mcountor& contur;
	MMMAP<buint, bool> points;

	enum StartPos : char { LeftMid = 0, LeftTop = 1, TopMid = 2, RigthTop = 3, RigthMid = 4, RigthBottom = 5, BottomMid = 6, LeftBottom = 7 };

	StartPos dirct = RigthMid;
	std::stack<StartPos> dirs;
	std::stack<buint> pointsStack;

	int accum = 2;
	int poss[16][2] = { {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1} };

public:
	MapCountur(mcountor& contur) : contur(contur)
	{ }

	void setFull()
	{
		accum = 2;
	}

	void setShort()
	{
		accum = 1;
	}

	// There is a bug somewhere that lead to infinite loop. Try to find and fix it

	void run(const bool aproxim = false)
	{
		int prevS;
		dirct = RigthMid;

		if (aproxim)
		{
			contur.push_back(stIndex);
		}

		while (!pointsStack.empty() || getIndex() != stIndex)
		{
			char start = (dirct + 6) % 8; // Safe minus 2
			char end = start + 5;
			// Check
			// 1 2 3
			// 0 X 4
			prevS = getIndex();

			bool foundMove = false;
			for (; start < end; start += accum)
			{
				int* off = poss[(int)start];
				if (tryVal(off[0], off[1]))
				{
					foundMove = true;
					break;
				}
			}

			if (foundMove)
			{
				int s = getIndex();
				StartPos old = dirct;
				dirs.push(dirct);
				dirct = (StartPos)(start % 8);

				// Check new disk with the old one
				if (dirct != old || !aproxim)
				{
					contur.push_back(prevS);
				}

				if (s == stIndex)
				{
					break;
				}
			}
			else
			{
				unset();

				if (pointsStack.empty())
					break;

				StartPos old = dirct;
				dirct = dirs.top();
				dirs.pop();

				if (dirct != old || !aproxim)
					contur.pop_back();

				auto p = bc::barvalue::getStatPoint(pointsStack.top());
				pointsStack.pop();
				x = p.x;
				y = p.y;
			}
		}

		//if (aproxim)
		//{
		//	contur.push_back(prevS);
		//	//contur.push_back(getIndex());
		//	//					contur.push_back(s);
		//}
	}

	void set(const bc::barvalue& p) { points[p.getIndex()] = true; }
	void set(int x, int y) { points[bc::barvalue::getStatInd(x, y)] = true; }

	void setStart(int x, int y)
	{
		this->x = x;
		this->y = y;
		stIndex = bc::barvalue::getStatInd(x, y);
	}

private:

	void unset() { points[bc::barvalue::getStatInd(x, y)] = false; }

	bool exists(int xl, int yl)
	{
		auto ds = points.find(bc::barvalue::getStatInd(xl, yl));
		if (ds == points.end())
			return false;

		return ds->second;
	}

	bool tryVal(int oX, int oY)
	{
		if (exists(x + oX, y + oY))
		{
			pointsStack.push(getIndex());
			x += oX;
			y += oY;
			return true;
		}
		return false;
	}

	buint getIndex()
	{
		return bc::barvalue::getStatInd(x, y);
	}
};


CounturRect getCountour(const bc::barvector& points, mcountor& contur, bool aproximate)
{
	contur.clear();

	int rect[4]{ 99999999, 99999999, 0, 0 };
	int stY = 99999;
	MapCountur dictPoints(contur);
	dictPoints.setFull();
	for (auto& p : points)
	{
		dictPoints.set(p);
		int x = p.getX();
		int y = p.getY();
		if (x <= rect[0])
		{
			rect[0] = x;
			if (stY > y)
			{
				stY = y;
			}
		}
		if (x > rect[2])
		{
			rect[2] = x;
		}
		if (y < rect[1])
		{
			rect[1] = y;
		}
		if (y > rect[3])
		{
			rect[3] = y;
		}
	}

	int wid = rect[2] - rect[0];
	int hei = rect[3] - rect[1];

	if (wid < 3 || hei < 3)// || (wid > 1000 && hei > 1000))
		return {BackPoint(rect[0], rect[1]), BackPoint(rect[2], rect[3])};

	dictPoints.setStart(rect[0], stY);
	dictPoints.run(aproximate);

	return {BackPoint(rect[0], rect[1]), BackPoint(rect[2], rect[3])};
}




CounturRect getCountourOder(const bc::barvector& points, mcountor& contur, bool aproximate)
{
	contur.clear();

	int rect[4]{ 99999999, 99999999, 0, 0 };
	int stY;
	MapCountur dictPoints(contur);
	dictPoints.setShort();
	for (auto& p : points)
	{
		dictPoints.set(p);
		dictPoints.set(p.x, p.y + 1);
		dictPoints.set(p.x + 1, p.y);
		dictPoints.set(p.x + 1, p.y + 1);
		int x = p.getX();
		int y = p.getY();
		if (x < rect[0])
		{
			rect[0] = x;
			stY = y;
		}
		if (x > rect[2])
		{
			rect[2] = x;
		}
		if (y < rect[1])
		{
			rect[1] = y;
		}
		if (y > rect[3])
		{
			rect[3] = y;
		}
	}

	int wid = rect[2] - rect[0];
	int hei = rect[3] - rect[1];

	dictPoints.setStart(rect[0], stY);
	dictPoints.run(aproximate);

	return { BackPoint(rect[0], rect[1]), BackPoint(rect[2], rect[3]) };
}

void getCountourSimple(const bc::barvector& points, bc::barvector& contur, float changeFactor)
{
	contur.clear();

	DictWrap dictPoints;
	for (auto& p : points)
	{
		dictPoints.set(p.getIndex());
		int x = p.getX();
		int y = p.getY();
		if (x > dictPoints.hei)
		{
			dictPoints.hei = x;
		}
		if (y > dictPoints.wid)
		{
			dictPoints.wid = y;
		}
	}

	for (auto p : points)
	{
		int x = p.getX();
		int y = p.getY();
		if (!dictPoints.hasCorners(x, y))
		{
			p.setXY(x * changeFactor, y * changeFactor);
			contur.push_back(p);
		}
	}
}


void getCircle(const bc::barvector& points, BackPoint& center, float& r)
{
	int rect[4]{ 99999999, 99999999, 0, 0 };
	for (const auto& p : points)
	{
		int x = p.getX();
		int y = p.getY();

		if (x < rect[0])
			rect[0] = x;
		if (x > rect[2])
			rect[2] = x;

		if (y < rect[1])
			rect[1] = y;
		if (y > rect[3])
			rect[3] = y;
	}


	const int wid = rect[2] - rect[0];
	const int hei = rect[3] - rect[1];
	if (wid < 3 || hei < 3)// || (wid > 1000 && hei > 1000))
		return;

	center.x = rect[0] + wid / 2;
	center.y = rect[1] + hei / 2;
	r = (center.x > center.y ? wid : hei) / 2;
}

void getRect(const bc::barvector& points, BackPoint& topLeft, BackPoint& bottomRight)
{
	topLeft.x = 99999999;
	topLeft.y = 99999999;
	bottomRight.x = 0;
	bottomRight.y = 0;

	for (const auto& p : points)
	{
		int x = p.getX();
		int y = p.getY();

		if (x < topLeft.x)
			topLeft.x = x;
		if (x > bottomRight.x)
			bottomRight.x = x;

		if (y < topLeft.y)
			topLeft.y = y;
		if (y > bottomRight.y)
			bottomRight.y = y;
	}
}



// void ShapeFile::writeShapefileHeader()
// {
// 	int numRecordsDummy = 0;

// 	const int fileCode = 9994;
// 	const int unused = 0;
// 	const int version = 1000;
// 	const int shapeType = 1; // Point
// 	const double minX = -180.0, minY = -90.0, maxX = 180.0, maxY = 90.0;

// 	// Write .shp header
// 	shp.write(reinterpret_cast<const char*>(&fileCode), sizeof(fileCode));
// 	for (int i = 0; i < 5; ++i) shp.write(reinterpret_cast<const char*>(&unused), sizeof(unused));
// 	int fileLength = 50 + numRecordsDummy * 10; // 50 header + 10 bytes per record
// 	fileLength = fileLength * 2; // Convert to 16-bit words

// 	shpPosToWriteSize = shp.tellp();
// 	shp.write(reinterpret_cast<const char*>(&fileLength), sizeof(fileLength));
// 	shp.write(reinterpret_cast<const char*>(&version), sizeof(version));
// 	shp.write(reinterpret_cast<const char*>(&shapeType), sizeof(shapeType));
// 	shp.write(reinterpret_cast<const char*>(&minX), sizeof(minX));
// 	shp.write(reinterpret_cast<const char*>(&minY), sizeof(minY));
// 	shp.write(reinterpret_cast<const char*>(&maxX), sizeof(maxX));
// 	shp.write(reinterpret_cast<const char*>(&maxY), sizeof(maxY));
// 	for (int i = 0; i < 4; ++i)
// 		shp.write(reinterpret_cast<const char*>(&unused), sizeof(unused));

// 	// Write .shx header (same as .shp)
// 	shx.write(reinterpret_cast<const char*>(&fileCode), sizeof(fileCode));
// 	for (int i = 0; i < 5; ++i)
// 		shx.write(reinterpret_cast<const char*>(&unused), sizeof(unused));
// 	int shxFileLength = 50 + numRecordsDummy * 4; // 50 header + 4 bytes per record
// 	shxFileLength = shxFileLength * 2; // Convert to 16-bit words

// 	shxPosToWriteSize = shp.tellp();
// 	shx.write(reinterpret_cast<const char*>(&shxFileLength), sizeof(shxFileLength));
// 	shx.write(reinterpret_cast<const char*>(&version), sizeof(version));
// 	shx.write(reinterpret_cast<const char*>(&shapeType), sizeof(shapeType));
// 	shx.write(reinterpret_cast<const char*>(&minX), sizeof(minX));
// 	shx.write(reinterpret_cast<const char*>(&minY), sizeof(minY));
// 	shx.write(reinterpret_cast<const char*>(&maxX), sizeof(maxX));
// 	shx.write(reinterpret_cast<const char*>(&maxY), sizeof(maxY));
// 	for (int i = 0; i < 4; ++i) shx.write(reinterpret_cast<const char*>(&unused), sizeof(unused));
// }

// void ShapeFile::writeShapefileRecord(const Point& point) {
// 	int recordNumber = ++recordCounter;

// 	int contentLength = 10; // 10 bytes for a point record
// 	int recordOffset = 50 + (recordNumber - 1) * 10; // Offset in 16-bit words

// 	// Write to .shx
// 	shx.write(reinterpret_cast<const char*>(&recordOffset), sizeof(recordOffset));
// 	shx.write(reinterpret_cast<const char*>(&contentLength), sizeof(contentLength));

// 	// Write to .shp
// 	shp.write(reinterpret_cast<const char*>(&recordNumber), sizeof(recordNumber));
// 	shp.write(reinterpret_cast<const char*>(&contentLength), sizeof(contentLength));
// 	int shapeType = 1; // Point
// 	shp.write(reinterpret_cast<const char*>(&shapeType), sizeof(shapeType));
// 	shp.write(reinterpret_cast<const char*>(&point.x), sizeof(point.x));
// 	shp.write(reinterpret_cast<const char*>(&point.y), sizeof(point.y));
// }

// ShapeFile::ShapeFile(std::string_view path)
// {
// 	std::string shpPath = std::string(path) + ".shp";
// 	std::string shxPath = std::string(path) + ".shx";

// 	shp.open(shpPath, std::ios::binary);
// 	shx.open(shxPath, std::ios::binary);

// 	if (!shp || !shx) {
// 		std::cerr << "Error opening shapefile for writing: " << shpPath << std::endl;
// 	}

// 	writeShapefileHeader();
// }

// void ShapeFile::writePolygonRecord(const bc::barline& polygon)
// {
// 	int recordNumber = ++recordCounter;

// 	mcountor count;
// 	CounturRect rect = getCountour(polygon.matr, count, true);

// 	// Calculate content length: 44 bytes for polygon header + 16 bytes per vertex
// 	int contentLength = 44 + (count.size() * 16);
// 	int recordOffset = 50 + (recordNumber - 1) * 10; // Offset in 16-bit words

// 	// Write to .shx
// 	shx.write(reinterpret_cast<const char*>(&recordOffset), sizeof(recordOffset));
// 	shx.write(reinterpret_cast<const char*>(&contentLength), sizeof(contentLength));

// 	// Write to .shp
// 	shp.write(reinterpret_cast<const char*>(&recordNumber), sizeof(recordNumber));
// 	shp.write(reinterpret_cast<const char*>(&contentLength), sizeof(contentLength));
// 	int shapeType = 5; // Polygon
// 	shp.write(reinterpret_cast<const char*>(&shapeType), sizeof(shapeType));

// 	// Write bounding box
// 	double minX = rect.bottomLeft().x;
// 	double minY = rect.bottomLeft().y;
// 	double maxX = rect.topRight().x;
// 	double maxY = rect.topRight().y;

// 	shp.write(reinterpret_cast<const char*>(&minX), sizeof(minX));
// 	shp.write(reinterpret_cast<const char*>(&minY), sizeof(minY));
// 	shp.write(reinterpret_cast<const char*>(&maxX), sizeof(maxX));
// 	shp.write(reinterpret_cast<const char*>(&maxY), sizeof(maxY));

// 	// Write number of parts and number of points
// 	int numParts = 1; // Single part polygon
// 	int numPoints = count.size();
// 	shp.write(reinterpret_cast<const char*>(&numParts), sizeof(numParts));
// 	shp.write(reinterpret_cast<const char*>(&numPoints), sizeof(numPoints));

// 	// Write part indices (only one part starting at index 0)
// 	int partStartIndex = 0;
// 	shp.write(reinterpret_cast<const char*>(&partStartIndex), sizeof(partStartIndex));

// 	// Write vertices
// 	for (const auto& vertex : count)
// 	{
// 		auto p = bc::barvalue::getStatPoint(vertex);
// 		double x = p.x;
// 		double y = p.y;
// 		shp.write(reinterpret_cast<const char*>(&x), sizeof(x));
// 		shp.write(reinterpret_cast<const char*>(&y), sizeof(y));
// 	}
// }

// void ShapeFile::close()
// {
// 	if (shp.is_open())
// 	{

// 		shp.close();
// 	}
// 	if (shx.is_open())
// 	{
// 		shx.close();
// 	}
// }


ShapeFile::ShapeFile(std::string_view path)
{
	std::string shpPath = std::string(path) + ".shp";
	std::string dbfPath = std::string(path) + ".dbf";

	shpHandle = SHPCreate(shpPath.c_str(), SHPT_POLYGON);
	dbfHandle = DBFCreate(dbfPath.c_str());

	if (!shpHandle || !dbfHandle) {
		throw std::runtime_error("Error creating shapefile: " + std::string(path));
	}
}

void ShapeFile::writePolygonRecord(const bc::barline& polygon)
{
	mcountor count;
	CounturRect rect = getCountour(polygon.matr, count, true);

	std::vector<double> xCoords, yCoords;
	for (const auto& vertex : count)
	{
		auto p = bc::barvalue::getStatPoint(vertex);
		xCoords.push_back(p.x);
		yCoords.push_back(p.y);
	}

	SHPObject* shpObject = SHPCreateSimpleObject(SHPT_POLYGON, xCoords.size(), xCoords.data(), yCoords.data(), nullptr);
	if (!shpObject) {
		throw std::runtime_error("Error creating SHPObject for polygon");
	}

	SHPWriteObject(shpHandle, -1, shpObject);
	SHPDestroyObject(shpObject);
}

void ShapeFile::close()
{
	if (shpHandle) {
		SHPClose(shpHandle);
		shpHandle = nullptr;
	}
	if (dbfHandle) {
		DBFClose(dbfHandle);
		dbfHandle = nullptr;
	}
}

ShapeFile::~ShapeFile() {
	close();
}
