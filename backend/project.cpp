#include "MatrImg.h"
#include "project.h"
#include "../frontend/Framework.h"


using std::min;
using std::vector;

Project *Project::proj = nullptr;
using namespace bc;

Project::Project()
{
	projectPath = "D:\\Programs\\Barcode\\_bar\\_p2\\";
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

	JsonObejct gameObject;
	write(gameObject);
	saveFile << gameObject.dump();
	saveFile.close();

	BackPathStr pas = getPath(BackPath::tiles);
	if (!pathExists(pas))
	{
		mkdir(pas);
	}

	return true;
}

void Project::closeImages()
{
	for (int i = 0; i < images.size(); ++i)
		delete images[i];

	images.clear();
}

// #include <opencv2/opencv.hpp>
// #include <opencv2/core/mat.hpp>

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


bc::Barcontainer* Project::createBarcode(const bc::BarConstructor& constr, int imgIndex, int)
{
	if (block) return nullptr;

//	reader->setCurrentSubImage(1);
	if (imgType == ReadType::Tiff)
		dynamic_cast<TiffReader*>(reader)->setCurrentSubImage(imgIndex);

	BarcodeCreator creator;

	Barcontainer *barconsrt = nullptr;
	Baritem *main = nullptr;
	const uint fullTile = tileSize + tileOffset;
	uint stH = 0, stW = 0;


//	FileBuffer boundStream;
//	if (!boundStream.openFileStream(getPath(BackPath::barlist)))
//		return barconsrt;

	uint rwid = reader->widght();
	uint rhei = reader->height();

	BarBinFile writer;
	writer.openWrite(getPath(BackPath::binbar).string());
	writer.writeHeaderProto(getCon(rwid, tileSize) * getCon(rhei, tileSize));
//	QImage remp(rwid, rhei,  QImage::Format_RGB888);

	BarBinFile::sets set;
	set.totalSize = rwid * rhei;

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
//			for (uint rX = 0; rX < iwid; ++rX)
//			{
//				for (uint rY = 0; rY < ihei; ++rY)
//				{
//					const auto v = rect.get(rX, rY);
//					const auto* p = v.val.rgba.samples;
//					remp.setPixel(stW + rX, stH + rY, qRgb(p[0].s,p[1].s,p[2].s));
//				}
//			}
//			remp.save("D:\\temp.png");

			DataRectBarWrapper warp(rect);
			std::unique_ptr<bc::Barcontainer> ret(creator.createBarcode(&warp, constr));
			main = ret->getItem(0);
			writer.write(main, ke++, set);

			stW += tileSize;
		}
		stH += tileSize;
	}

	if (main == nullptr)
		return nullptr;

	writer.close();
	//	boundStream.close();
	saveProject();

	return barconsrt;
}


bool needSkip(const Barscalar &scal)
{
	return false; //scal > 10;
}

BarcodesHolder Project::toHoldes(bc::barlinevector& lines, MatrImg& mat, bc::point offset)
{
	bc::CloudPointsBarcode::CloundPoints cloud;
	for (size_t var = 0; var < lines.size(); ++var)
	{
		if (needSkip(lines[var]->len()))
		{
			continue;
		}

		auto &m = lines[var]->matr[0];
		mat.set(m.getX() - offset.x, m.getY() - offset.y, Barscalar(255, 0, 0));
		cloud.points.push_back(bc::CloudPointsBarcode::CloundPoint(m.getX(), m.getY(), m.value.getAvgFloat()));
	}

	return toHoldes(cloud);
}


BarcodesHolder Project::toHoldes(BarcodesHolder& lines, MatrImg& mat, bc::point offset)
{
	bc::CloudPointsBarcode::CloundPoints cloud;
	for (size_t var = 0; var < lines.lines.size(); ++var)
	{
		if (needSkip(lines.lines[var]->lines[0]->len()))
		{
			continue;
		}

		auto &m = lines.lines[var]->matrix[0];
		bc::point rp(m.getX() + offset.x, m.getY() + offset.y);
		bc::point cp = rp * u_displayFactor;
		mat.set(cp.x, cp.y, Barscalar(255, 0, 0));
		if (cp.x - 1 >= 0)
		{
			mat.set(cp.x - 1, cp.y, Barscalar(255, 0, 0));
		}
		if (cp.x + 1 < mat.wid())
		{
			mat.set(cp.x + 1, cp.y, Barscalar(255, 0, 0));
		}
		if (cp.y - 1 >= 0)
		{
			mat.set(cp.x, cp.y - 1, Barscalar(255, 0, 0));
		}
		if (cp.y + 1 < mat.wid())
		{
			mat.set(cp.x, cp.y + 1, Barscalar(255, 0, 0));
		}
		assert(cp.x >= 0);
		assert(cp.y >= 0);
		//cloud.points.push_back(bc::CloudPointsBarcode::CloundPoint(rp.x, rp.y, m.value.getAvgFloat()));
	}

	return toHoldes(cloud);
}



BarcodesHolder Project::toHoldes(const bc::CloudPointsBarcode::CloundPoints& cloud)
{
	bc::CloudPointsBarcode clodCrt;
	std::unique_ptr<bc::Barcontainer> hold(clodCrt.createBarcode(&cloud));

	BarcodesHolder holder;
	if (cloud.points.size() == 0)
		return holder;

	bc::Baritem* main = hold->getItem(0);
	for (size_t var = 0; var < main->barlines.size(); ++var)
	{
		auto *line = main->barlines[var];
		BarcodeHolder *barhold = new BarcodeHolder();
		holder.lines.push_back(barhold);

		barhold->matrix = std::move(line->matr);
		line->getChilredAsList(barhold->lines, true, true, false);
	}

	return holder;
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
	std::ifstream loadFile(prjFilepath);

	if (!loadFile.is_open())
	{
		std::cerr << "Couldn't open save file.";
		return false;
	}

	BackJson loadDoc = BackJson::parse(loadFile);
	read(loadDoc);

	//	qDebug() << searchSetts.height.start;
	//	qDebug() << searchSetts.heightMin();

	openReader();
	modelWid = reader->widght() / u_displayFactor;
	modelHei = reader->height() / u_displayFactor;
	readImages();
	return true;
}

static const char* jsn_displayFacto = "step";
static const char* jsn_imgMinVal	= "imgMinVal";
static const char* jsn_imgMaxVal	= "imgMaxVal";
static const char* jsn_geojsonPath  = "geojsonPath";
static const char* jsn_imgPath      = "imgPath";

void Project::read(const BackJson& json)
{
	this->u_displayFactor = json[jsn_displayFacto].get<double>();
	this->u_imgMinVal = json[jsn_imgMinVal].get<double>();
	this->u_imgMaxVal = json[jsn_imgMaxVal].get<double>();
	this->u_imgPath = json[jsn_imgPath].get<BackString>();
	this->u_geojsonPath = json[jsn_geojsonPath].get<BackString>();
}

void Project::write(BackJson& json) const
{
	json[jsn_displayFacto] = this->u_displayFactor;
	json[jsn_imgMaxVal]	= this->u_imgMaxVal;
	json[jsn_imgMinVal]	= this->u_imgMinVal;
	json[jsn_imgPath] = u_imgPath;
	json[jsn_geojsonPath] = this->u_geojsonPath;
}

static int getFid(int wid, int s)
{
	return (wid + s - 1) / s;
}
static MatrImg* tiffToImg(ImageReader *reader, const BackPathStr &path, int fctor = 10, bool save = false)
{
	bool rgb = reader->getSamples() > 1;
	MatrImg* outr = new MatrImg(getFid(reader->widght(), fctor), getFid(reader->height(), fctor), 3);

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
	std::ifstream loadFile(getPath(BackPath::geojson));

	if (!loadFile.is_open() || imgType == ReadType::Simple)
	{
		return;
	}

	BackJson object = BackJson::parse(loadFile).object();
	loadFile.close();

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
