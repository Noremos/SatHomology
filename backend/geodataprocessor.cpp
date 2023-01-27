#include "geodataprocessor.h"

#include "project.h"

#include <stack>
#include <ranges>

double getPsa(const bc::barvector &matr)
{
	std::unordered_map<uint, bool> map;

	int minX = 10000, maxX = 0, minY = 1000, maxY = 0;
	for (const auto &pm : matr)
	{
		map[pm.index] = true;
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



void getMaskRes(const MatrImg &matres, std::vector<Cound *> &veas, Cound **resmap)
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
			Cound *b = resmap[ind];
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
		auto &d = veas[i];
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
	std::cout << "Res bad: " << proc - ew  << std::endl;
}

void getMaskRes(const MatrImg &mat, MatrImg &maskMat)
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
			else if(a == 0 && b == 0)
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
	mcountor &contur;
	std::unordered_map<uint, bool> points;

	enum StartPos : char { LeftMid = 0, LeftTop = 1, TopMid = 2, RigthTop = 3, RigthMid = 4, RigthBottom = 5, BottomMid = 6, LeftBottom = 7 };

	StartPos dirct = RigthMid;
	std::stack<StartPos> dirs;
	std::stack<uint> pointsStack;

public:
	MapCountur(mcountor &contur) : contur(contur) {}

	void run(const bool aproxim = false)
	{
		static int poss[16][2] = {{-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}};

		dirct = RigthMid;
		while (true)
		{
			char start = (dirct + 6) % 8; // Safe minus 2
			char end = start + 5;
			// Check
			// 1 2 3
			// 0 X 4
			int prevS = getIndex();

			for (; start < end; ++start)
			{
				int *off = poss[(int) start];
				if (tryVal(off[0], off[1]))
				{
					break;
				}
			}

			if (start != end)
			{
				int s = getIndex();
				StartPos old = dirct;
				dirs.push(dirct);
				dirct = (StartPos)(start % 8);

				// Check new disk with the old one
				if (dirct != old || !aproxim)
				{
					contur.push_back(prevS);
//					contur.push_back(s);
				}

				if (s == stIndex)
				{
					break;
				}
			}
			else
			{
				unset();

				if (pointsStack.size() < 1)
					break;
				//				if (addIndex < 1)
				//					break;

				StartPos old = dirct;
				dirct = dirs.top();
				dirs.pop();

				if (dirct != old || !aproxim)
					contur.pop_back();

				auto p = bc::barvalue::getStatPoint(pointsStack.top());
				pointsStack.pop();
				//contur[addIndex]);
				x = p.x;
				y = p.y;
			}
		}
	}

	void set(const bc::barvalue &p) { points[p.getIndex()] = true; }

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

	uint getIndex()
	{
		return bc::barvalue::getStatInd(x, y);
	}
};


void getCountour(const bc::barvector &points, mcountor &contur, bool aproximate)
{
	contur.clear();

	int rect[4]{99999999, 99999999, 0, 0};
	int stY;
	MapCountur dictPoints(contur);
	for (auto &p : points)
	{
		dictPoints.set(p);
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

	if (wid < 5 || hei < 5)// || (wid > 1000 && hei > 1000))
		return;

	dictPoints.setStart(rect[0], stY);
	dictPoints.run(aproximate);
}


void saveJson(const string &text, int st)
{
	BackFileWrited file(std::format("D:\\{}.json", st));
	if (file.is_open())
	{
		file << text;
		file.close();
	}
}

void saveAllJsons(bc::barlinevector *geojson, int num, const BackDirStr& dir)
{
	ProcPoint start;
	int size;

	std::ifstream inputFile("D:\\Programs\\C++\\Barcode\\PrjBarlib\\researching\\coords.txt");
	if (inputFile.is_open())
	{
		BackFileReader inputFile("file.txt");
		BackString line;
		int size;
		int i = 0;

		while (std::getline(inputFile, line))
		{
			if (i == num)
				break;

			size_t commm = line.find("#");
			line = line.substr(0, commm);
			std::vector<std::string> ist;

			for (const auto word : std::views::split(line, "|"))
				ist.push_back(BackString(word.begin(), word.end()));

			if (ist.size() != 3)
				continue;

			start.x = std::stof(ist[0]);
			start.y = std::stof(ist[1]);
			size = std::stoi(ist[2]);

			i++;
		}

		inputFile.close();
	}

	int wid = 500;
	double coof = 0.00001;

	start.x += (wid / 2) * coof;
	start.x -= (wid / 2) * coof;

	saveAsGeojson(geojson[1], dir / "2skat.geojson", start, coof);
	saveAsGeojson(geojson[2], dir / "1skat.geojson", start, coof);
}


void saveAsGeojson(const bc::barlinevector &lines, const BackPathStr& savePath, ProcPoint startOffset, double coof)
{
	BackString json = "{\"type\":\"FeatureCollection\","
				   "\"name\":\"Roofs\","
				   "\"crs\": { \"type\": \"name\", \"properties\":{\"name\": \"urn:ogc:def:crs:EPSG::3857\" } }," //3857
				   "\"features\":[ ";

	json += "";

	for (int i = 0, total = lines.size(); i < total; ++i)
	{
		BackString safsd = "{ \"type\": \"Feature\",";
		safsd += "\"properties\":{\"id\":"; // TODO! Check
		safsd += intToStr(i + 1);
		safsd += "}, \"geometry\": {\"type\":\"MultiPolygon\", \"coordinates\":[[[ ";

		auto &allPoints = lines[i]->getMatrix();
		mcountor cont;
		getCountour(allPoints, cont, true);

		int totalc = cont.size();
		for (int j = 0; j < totalc; ++j)
		{
			const auto &p = bc::barvalue::getStatPoint(cont[j]);
			double x = startOffset.x - p.y * coof;
			double y = startOffset.y + p.x * coof;
			safsd += std::format("[{}, {}],", y, x);
		}

		if (totalc > 0)
		{
			safsd[safsd.length() - 1] = ']';
			safsd += "]]}},";
		}
		else
		{
			safsd += "]]]}},";
		}

		json += safsd;
	}

	json[json.length() - 1] = ']';
	json += "}";

	std::ofstream file(savePath, std::ios::trunc);
	if (file.is_open()) 
	{
		file << json;
		file.close();
	}
	//	widget->importedMakrers->release();
	//	//	Size2 size = imgsrch.getTileSize();
}


void BarClassifierCache::loadCategories(std::function<void(int classId, const BackString& name)> callback)
{
	barclassificator barclas;
	BackJson loadDoc = jsonFromFile(Project::getPathSt(BackPath::classifier));

	JsonArray list = loadDoc["categories"];
	for (size_t i = 0; i < list.size(); i++)
	{
		JsonObject catId = list.at(i);
		callback(catId["id"], catId["name"].get<std::string>());
	}
}

BarCategories BarClassifierCache::loadCategories()
{
	BackPathStr path = Project::getProject()->getPath(BackPath::classifier);

	BarCategories categ;
	if (!pathExists(path))
	{
		categ.value.push_back(0);
		categ.name.push_back("Bad");
		categ.value.push_back(1);
		categ.name.push_back("Kurgan");
		return categ;
	}

	auto loadColback = [&categ](int classId, const BackString& name)
	{
		categ.value.push_back(classId);
		categ.name.push_back(name);
	};
	loadCategories(loadColback);

	return categ;
}

void BarClassifierCache::saveCategories(BarCategories& barclas)
{
	BackPathStr path = Project::getProject()->getPath(BackPath::classifier);

	JsonArray arr;
	for (size_t i = 0; i < barclas.name.size(); i++)
	{
		JsonObject catId;
		catId["id"] = barclas.value[i];
		catId["name"] = barclas.name[i];
		arr.push_back(catId);
	}

	BackJson loadDoc;
	loadDoc["categories"] = arr;
	jsonToFile(loadDoc, path);
}

void BarClassifierCache::loadImgs(std::function<void(int classId, const BackPathStr& path)> callback, int* categorues, int size)
{
	BackPathStr pathp = Project::getPathSt(BackPath::classfiles);
	GeoBarHolderCache creator;
	for (size_t i = 0; i < size; i++)
	{
		int categ = categorues[i];
		BackDirStr dirl = pathp / (BackPathStr)intToStr(categ);
		if (!pathExists(dirl))
			continue;

		for (auto const& entry : std::filesystem::directory_iterator(dirl))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			BackPathStr filename = entry.path().string();
			auto ext = entry.path().extension();
			if (ext == ".jpg")
			{
				callback(categ, filename);
			}
		}
	}
}

void BarClassifierCache::save(BarcodeHolder* curBar, int classIndex, BackImage* img)
{
	BackPathStr path = Project::getProject()->getPath(BackPath::classfiles);
	BackDirStr doorPat = path / intToStr(classIndex);
	mkDirIfNotExists(doorPat);

	std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(1, 1000);
	BackString name;
	do
	{
		name = std::format("{}.bbf", distribution(generator));
	} while (pathExists(doorPat / name));
	doorPat /= name;

	GeoBarHolderCache writer;
	writer.openWrite(doorPat.string());
	writer.save(0, curBar);

	if (img)
	{
		doorPat = doorPat.replace_extension("jpg");
		imwrite(doorPat, *img);
	}
}

void GeoBarHolderCache::openRead()
{
	state.reset(new StateBinFile::BinStateReader());
	state->open(Project::getPathSt(BackPath::binbar).string());
}

void GeoBarHolderCache::openWrite()
{
	state.reset(new StateBinFile::BinStateWriter());
	state->open(Project::getPathSt(BackPath::binbar).string());
}
