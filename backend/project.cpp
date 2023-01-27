#include "MatrImg.h"
#include "project.h"
#include "../frontend/Framework.h"

#include "../modules/StateBinFile.h"
#include <unordered_set>
#include "geodataprocessor.h"
#include <random>

bc::barlinevector geojson[3];

using std::min;
using std::vector;

Project *Project::proj = nullptr;
using namespace bc;

Project::Project()
{
	projectPath = "D:\\Programs\\Barcode\\_bar\\_p2\\";

	u_classCache = getDicumnetPath() / "GeoBar";
	mkDirIfNotExists(u_classCache);


	srand(300);
	colors.push_back(Barscalar(255, 0, 0));
	colors.push_back(Barscalar(0, 0, 0));
	colors.push_back(Barscalar(0, 255, 0));
	colors.push_back(Barscalar(0, 0, 255));
	for (int k = 0; k < 40; ++k)
		colors.push_back(Barscalar(5 + rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

	colors.push_back(Barscalar(255, 255, 255));
}

bool Project::saveProject()
{
	std::ofstream saveFile(getPath(BackPath::project), std::ios::trunc);
	//BackJson data = BackJson::parse(f);

	if (!saveFile.is_open())
	{
		std::cerr << "Couldn't open save file.";
		return false;
	}

	JsonObject gameObject;
	write(gameObject);
	saveFile << gameObject.dump();
	saveFile.close();

	mkDirIfNotExists(getPath(BackPath::tiles));
	BarClassifierCache ccb;
	ccb.saveCategories(classer.categs);
	return true;
}

void Project::closeImages()
{
	for (int i = 0; i < images.size(); ++i)
		delete images[i];

	images.clear();
}

float Project::getImgMinVal() const
{
	return u_imgMinVal;
}

float Project::getImgMaxVal() const
{
	return u_imgMaxVal;
}


int getCon(int total, int part)
{
	return total / part + (total % part == 0 ? 0 : 1);
}


void Project::getOffsertByTileIndex(int tileIndex, uint &offX, uint &offY)
{
//	reader->setPrevImage();

//	int rhei = getCon(reader->height(), tileSize);
	int rwid = getCon(reader->widght(), tileSize);
	offY = (tileIndex / rwid) * tileSize;
	offX = (tileIndex % rwid) * tileSize;
}

void saveLoadBarlines(StateBinFile::BinState& state)
{
	//state.begin();
	//state.end();
}


// ---------------------------

bc::Barcontainer* Project::createCacheBarcode(const bc::BarConstructor& constr, int imgIndex, const FilterInfo& info)
{
	if (block) return nullptr;

//	reader->setCurrentSubImage(1);
	if (imgType == ReadType::Tiff)
		dynamic_cast<TiffReader*>(reader)->setCurrentSubImage(imgIndex);

	GeoBarHolderCache creator;
	creator.openWrite(getPath(BackPath::binbar).string());

	Barcontainer *barconsrt = nullptr;
	Baritem *main = nullptr;
	const uint fullTile = tileSize + tileOffset;
	uint stH = 0, stW = 0;

	uint rwid = reader->widght();
	uint rhei = reader->height();

	int ke = 0;
	for (uint i = 0; i < rhei; i += tileSize)
	{
		uint ihei = (stH + fullTile > rhei ? rhei - stH : fullTile);
		if (ihei <= tileOffset)
			break;

		stW = 0;
		std::cout << i << std::endl;
		for (uint j = 0; j < rwid; j += tileSize)
		{
			uint iwid = (stW + fullTile > rwid ? rwid - stW : fullTile);
			if (iwid <= tileOffset)
				break;

			DataRect rect = reader->getRect(stW, stH, iwid, ihei);

			DataRectBarWrapper warp(rect);
			main = creator.create(&warp, constr);
			auto cloud = creator.toCloundBarcode(main, bc::point(stW, stH), info);
			creator.save(ke++, &cloud);

			stW += tileSize;
		}
		stH += tileSize;
	}

	if (main == nullptr)
		return nullptr;

	saveProject();

	return barconsrt;
}


void Project::readPrcoessBarcode(ClassInfo& info)
{
	if (u_displayFactor < 1.0)
		throw std::exception();

	GeoBarHolderCache reader;
	reader.openRead(getPath(BackPath::binbar).string());

	while (reader.canRead())
	{
		std::unique_ptr<CloudItem> baritem(reader.loadCloudItem(info.ind));
		std::cout << info.ind << std::endl;

		classBarcode(*baritem.get(), info);
		//classBarcode(baritem, ind, mat, map, extra);
	}
}


void Project::classBarcode(BarcodesHolder& baritem, ClassInfo& info)
{
	bool wr = info.extra.find("json;") != -1;
	//	bool ent = extra.indexOf("entr;") != -1;
	//	bool classif = extra.indexOf("barclass;") != -1;
	bool showBad = true;
	//extra.indexOf("show0;") != -1;
	bool showWhite = true; //extra.indexOf("showw;") != -1;

	uint xOff = 0;
	uint yOff = 0;
	getOffsertByTileIndex(info.ind, xOff, yOff);

	auto& vec = baritem.lines;
	for (size_t i = 0; i < vec.size(); ++i)
	{
		auto* b = vec.at(i);
		const auto& matr = b->matrix;

		if (matr.size() == 0)
			continue;

		Barscalar pointCol(255, 0, 0);
		int type = -1;
		{
			// It clones inside
			type = classer.getType(b);
			pointCol = colors[type + 1];

			//if (type == -1)
			//	type = rand() % colors.size();
			//pointCol = colors[type];
		}

		std::unordered_set<uint> vals;
		std::shared_ptr<SimpleLine> sl = std::make_shared<SimpleLine>(info.ind, i);
		int depth = b->getDeath();
		sl->depth = depth;
		//		simpleHolder.push_back(sl);

		info.map[(size_t)sl.get()] = type;

		for (const auto& pm : matr)
		{
			int ox = pm.getX();// -xOff;
			int oy = pm.getY();// -yOff;
			int x = (ox) / u_displayFactor;
			int y = (oy) / u_displayFactor;
			uint index = bc::barvalue::getStatInd(x, y);
			if (vals.find(index) != vals.end())
				continue;

			vals.insert(index);

			bc::point p = bc::barvalue::getStatPoint(index);
			sl->matr.push_back(bc::barvalue(p, pm.value));

			//info.mat.set(x, y, color);
			//			outMask.set(x, y, 255);
			int indLocal = (y * info.mat.wid() + x);
			SimpleLine* temp = info.resLinesMaps[indLocal].get();
			//			continue;
			if (temp == nullptr || /*map[(size_t)temp] == -2 ||*/ temp->getDeath() < b->getDeath())
			{
				info.resLinesMaps[indLocal] = sl;
				//				++sl->counter;
			}

			int ylek = 2;
			for (int i = MAX(x - ylek, 0); i < std::min(x + ylek, info.mat.wid()); i++)
			{
				for (int j = MAX(y - ylek, 0); j < std::min(y + ylek, info.mat.hei()); j++)
				{
					int indLocal2 = (j * info.mat.wid() + i);

					info.resLinesMaps[indLocal2] = sl;
				}
			}

			bc::point cp(x, y);
			info.mat.set(cp.x, cp.y, pointCol);
			if (cp.x - 1 >= 0)
			{
				info.mat.set(cp.x - 1, cp.y, pointCol);
			}
			if (cp.x + 1 < info.mat.wid())
			{
				info.mat.set(cp.x + 1, cp.y, pointCol);
			}
			if (cp.y - 1 >= 0)
			{
				info.mat.set(cp.x, cp.y - 1, pointCol);
			}
			if (cp.y + 1 < info.mat.wid())
			{
				info.mat.set(cp.x, cp.y + 1, pointCol);
			}
		}
	}
}

// ---------------------------


bool needSkip(const Barscalar &scal)
{
	return false; //scal > 10;
}


BarcodeHolder Project::threasholdLines(bc::Baritem *item)
{
	BarcodeHolder vec;
	for (size_t i = 0; i < item->barlines.size(); ++i)
	{
		if (item->barlines[i]->len() < 10)
		{
			vec.lines.push_back(item->barlines[i]->clone());
		}
	}
	return vec;
}

void Project::loadImage(const BackPathStr& path, int step)
{
	closeReader();

	this->u_imgPath = path;

	openReader();
	if (!reader->ready)
		return ;

//	this->u_imgMinVal = reader->min;
//	this->u_imgMaxVal = reader->max;
//	this->u_displayFactor = step;

	saveProject();
	writeImages();
}

bool Project::loadProject(const BackPathStr& prjFilepath)
{
	setProjectPath(prjFilepath);

	BackJson loadDoc = jsonFromFile(prjFilepath);
	read(loadDoc);

	//	qDebug() << searchSetts.height.start;
	//	qDebug() << searchSetts.heightMin();

	openReader();
	modelWid = reader->widght() / u_displayFactor;
	modelHei = reader->height() / u_displayFactor;
	readImages();

	BarClassifierCache bcc;
	classer.categs = bcc.loadCategories();
	classer.udpdateClasses();
	bcc.loadClasses(getPath(BackPath::classfiles), classer);
	return true;
}

static const char* jsn_displayFacto = "step";
static const char* jsn_imgMinVal	= "imgMinVal";
static const char* jsn_imgMaxVal	= "imgMaxVal";
static const char* jsn_geojsonPath  = "geojsonPath";
static const char* jsn_imgPath = "imgPath";
static const char* jsn_classfiles  = "barfiles";

void Project::read(const BackJson& json)
{
	this->u_displayFactor = json[jsn_displayFacto].get<double>();
	this->u_imgMinVal = json[jsn_imgMinVal].get<double>();
	this->u_imgMaxVal = json[jsn_imgMaxVal].get<double>();
	this->u_imgPath = json[jsn_imgPath].get<BackString>();
	this->u_geojsonPath = json[jsn_geojsonPath].get<BackString>();
	this->u_classCache = json[jsn_classfiles].get<BackString>();
}

void Project::write(BackJson& json) const
{
	json[jsn_displayFacto] = this->u_displayFactor;
	json[jsn_imgMaxVal]	= this->u_imgMaxVal;
	json[jsn_imgMinVal]	= this->u_imgMinVal;
	json[jsn_imgPath] = u_imgPath.string();
	json[jsn_geojsonPath] = this->u_geojsonPath.string();
	json[jsn_classfiles] = this->u_classCache.string();
}

static int getFid(int wid, int s)
{
	return (wid + s - 1) / s;
}
static MatrImg* tiffToImg(ImageReader *reader, const BackPathStr &path, int fctor = 10, bool save = false)
{
	bool rgb = reader->getSamples() > 1;
	int widf = getFid(reader->widght(), fctor);
	int heif = getFid(reader->height(), fctor);
	MatrImg* outr = new MatrImg(widf, heif, 3);

	float NAN_VALUE = reader->getNullValue();
	for (int h = 0, hr = 0; h < reader->height(); h += fctor, ++hr)
	{
		const rowptr& rp = reader->getRowData(h);
		for (int w = 0, wr = 0; w < reader->widght(); w += fctor, ++wr)
		{
			float value = rp.getFloat(w);
			if (value == NAN_VALUE)
			{
				outr->set(wr, hr, Barscalar(0,0,0));
			}
			else if (rgb)
			{
				auto r = rp.getValue(w);
				outr->set(wr, hr, Barscalar(r.rgba.samples[0].s, r.rgba.samples[1].s, r.rgba.samples[2].s));
			}
			else
			{
				if (value < 0)
					value = 0;
				else if (value > 255)
					value = 255;

				outr->set(wr, hr, Barscalar(value, value, value));
			}
		}
	}

	if (save)
		imwrite(path, *outr);
	return outr;
}

void Project::writeImages()
{
	if (!reader)
		return;

	images.clear();

	BackPathStr tiles = getPath(BackPath::tiles);
	if (imgType == ReadType::Tiff)
	{
		TiffReader *trear = dynamic_cast<TiffReader *>(reader);
		int s = trear->getSubImageSize();
		for (int i = 0; i < s; ++i)
		{
			int factor = 1;
			trear->setCurrentSubImage(i);
			if (reader->widght() > 2000)
			{
				images.push_back(nullptr);
			}
			else
				images.push_back(tiffToImg(reader, tiles / (intToStr(i) + ".png"), factor, true));
		}

		if (images.size() == 0)
			images.push_back(tiffToImg(reader, tiles / (intToStr(s) + ".png"), 10, true));
	}
	else
	{
		images.push_back(tiffToImg(reader, tiles / (intToStr(0) + ".png"), 1, true));
	}
}

void Project::readImages()
{
	if (!reader)
		return;

	BackDirStr tiles = getPath(BackPath::tiles);
	if (imgType == ReadType::Tiff)
	{
		TiffReader *treader = dynamic_cast<TiffReader *>(reader);
		int s = treader->getSubImageSize();
		closeImages();
		for (int i = 0; i < s; ++i)
		{
			treader->setCurrentSubImage(i);
			if (reader->widght() > 2000)
			{
				images.push_back(nullptr);
			}
			else
				images.push_back(tiffToImg(reader, tiles / (intToStr(i) + ".png"), 1, false));
		}

		if (images.size() == 0)
			images.push_back(tiffToImg(reader, tiles / (intToStr(s) + ".png"), 10, false));
	}
	else
		images.push_back(tiffToImg(reader, tiles / (intToStr(0) + ".png"), 1, false));
}


int normal(float val, int factor)
{
	return  static_cast<int>(val/factor);
}

void Project::readGeojson()
{

	if (imgType == ReadType::Simple)
	{
		return;
	}
	BackJson object = jsonFromFile(getPath(BackPath::geojson));

	BackJson features = object["features"].array();

	TiffReader *treader = dynamic_cast<TiffReader *>(reader);
	//	openReader();
	//	Size2 size = imgsrch.getTileSize();
	for (size_t i = 0; i < features.size(); i++)
	{
		auto arrcoors = features.at(i)["geometry"].object()["coordinates"].array();

		// Send in format x, y, T
		//QVector3D coord(arrcoors[0].toDouble(), arrcoors[1].toDouble(), 0);
		//coord = treader->curSubImage->convertModelToRaster(coord);
		//// Get in format x, T, y

		//if (coord.x() < 0 || coord.z() < 0 || coord.x() >= reader->widght() || coord.z() >=reader->height())
		//	continue;

//		int x = normal(coord.x(), u_displayFactor);
//		int z = normal(coord.z(), u_displayFactor);
//		coord.setY(widget->terra->getValue(x, z));

//		widget->importedMakrers->addBoundy(coord, u_displayFactor);
	}

//	widget->importedMakrers->updateBuffer();
}


void Project::readMyGeo(bool reinitY)
{
//	QFile inputFile(getPath(BackPath::geojson));
//	if (!inputFile.open(QIODevice::ReadOnly))
//	{
//		return;
//	}
//
//	QTextStream in(&inputFile);
//	while (!in.atEnd())
//	{
//		BackString line = in.readLine();
//		auto splo = line.split(' ');
//		if (splo.size()!=3)
//			continue; // skip t K
//
//		QVector3D coord(splo[0].toFloat(), splo[1].toFloat(), splo[2].toFloat());
//
//		if (reinitY)
//		{
////			coord.setY(widget->terra->getValue(coord.x(), coord.z()));
//		}
//
////		widget->markers->addBoundy(coord, 1);
//	}
//	inputFile.close();

//	widget->markers->updateBuffer();
}

void Project::exportResult(int imgNumber, const BackImage& resultMart)
{
	imwrite(getPath(BackPath::root) / "result.png", resultMart);
	saveAllJsons(geojson, imgNumber, getPath(BackPath::geojson));
}


void Project::addClassData(int classIndex, BarcodeHolder* points, BackImage* destIcon)
{
	auto rect = bc::getBarRect(points->matrix);
	BackImage rt(rect.width, rect.height, 3);
	DataRect r = reader->getRect(rect.x, rect.y, rect.width, rect.height);
	BackImage imgReprp;
	if (destIcon == nullptr)
		destIcon = &imgReprp;

	*destIcon = BackImage(r.wid, r.hei, r.data.samples, r.data.ptr.b);

	BarClassifierCache saver;
	saver.save(points, classIndex, destIcon);
	classer.addData(classIndex, points->lines, false);
}

int Project::addClassType(const BackString& name)
{
	classer.classes.push_back(Barcontainer());
	return classer.categs.addValue(name);
}
