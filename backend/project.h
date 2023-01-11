#ifndef PROJECT_H
#define PROJECT_H

#include "SimpleImgReader.h"
#include "barcodeCreator.h"
#include "BinFile.h"

#include "ReaderTypes.h"
#include "tiffreader.h"
#include "Common.h"


struct toStdStr
{
	template<class T>
	static std::string toStr(T val)
	{
		return std::to_string(val);
	}
};


struct Cound;

class DataRectBarWrapper : public bc::DatagridProvider
{
	DataRect &baseobj;

	// DatagridProvider interface
public:
	DataRectBarWrapper(DataRect &base) : baseobj(base) {}
	int wid() const override { return baseobj.wid; }
	int hei() const override { return baseobj.hei; }
	int channels() const override { return baseobj.data.type == ImageType::argb8 ? 3 : 1; }
	void maxAndMin(Barscalar &min, Barscalar &max) const override
	{
		min = getLiner(0);
		max = min;

		for (size_t i = 1; i < this->length(); ++i)
		{
			const Barscalar &s = getLiner(i);
			if (s < min)
			{
				min = s;
			}
			if (s > max)
			{
				max = s;
			}
		}
		baseobj.maxVal = scalarToRow(max);
		baseobj.minVal = scalarToRow(min);
	}
	size_t typeSize() const override { return getImgTypeSize(baseobj.data.type); }
	Barscalar get(int x, int y) const override { return rowToScalar(baseobj.get(x, y)); }

	static Barscalar rowToScalar(const roweltype &val)
	{
		Barscalar nva;
		if (val.sampl > 1)
		{
			auto *smaplsr = val.val.rgba.samples;
			switch (val.type)
			{
			case ImageType::int8:
				return Barscalar(smaplsr[0].s,smaplsr[1].s, smaplsr[2].s);
			case ImageType::int16:
				return Barscalar(smaplsr[0].s,smaplsr[1].s, smaplsr[2].s);
			case ImageType::int32:
				return Barscalar(smaplsr[0].i,smaplsr[1].i, smaplsr[2].i);
			case ImageType::float32:
				return Barscalar(smaplsr[0].f,smaplsr[1].f, smaplsr[2].f);
			default:
				std::exception();
			}
		}
		switch (val.type)
		{
		case ImageType::int8:
			nva = Barscalar(val.val.s, BarType::BYTE8_1);
			break;
		case ImageType::int16:
			nva = Barscalar(val.val.s, BarType::FLOAT32_1);
			break;
		case ImageType::int32:
			nva = Barscalar(val.val.i, BarType::FLOAT32_1);
			break;
			//		case ImageType::argb8:
			//			nva = Barscalar(val.val.rgba.rgba[0], val.val.rgba.rgba[1], val.val.rgba.rgba[2]);
			break;
		case ImageType::float32:
			nva = Barscalar(val.val.f, BarType::FLOAT32_1);
			break;
		default:
			std::exception();
		}
		return nva;
	}


	static roweltype scalarToRow(const Barscalar &val)
	{
		roweltype nva;
		switch (val.type)
		{
		case BarType::BYTE8_1:
			nva = roweltype((uchar)val.data.b1);
			break;
			//		case BarType::BYTE8_3:
			//			nva = roweltype(val.data.b3[0], val.data.b3[1], val.data.b3[2], 255);
			//			break;
		case BarType::FLOAT32_1:
			nva = roweltype(val.data.f);
			break;
		case BarType::NONE:
		default:
			std::exception();
		}
		return nva;
	}

	// DataSetProvider interface
public:
	void set(int x, int y, const Barscalar& val)
	{
		baseobj.data.setValue(y * wid() + x, scalarToRow(val).val);
	}
};

//class BarReader
//{
//private:
//	size_t tempLineCounter = 0;
//	QFile jsonFile;
//	std::unique_ptr<bc::barlinevector> item;
//	QMap<size_t, bc::barline *> ids;
//	bool readLine()
//	{
//		QByteArray data = jsonFile.readLine();
//		if (data.size() == 0)
//			return false;
//		QJsonDocument jsonDocument(QJsonDocument::fromJson(data));
//		// Из которого выделяем объект в текущий рабочий QJsonObject
//		QJsonArray lines = jsonDocument.array();
//		item.reset(new bc::barlinevector());

//		auto parseBarscal = [](const QJsonValue &jsval) -> Barscalar {
//			if (jsval.isArray())
//			{
//				QJsonArray sr = jsval.toArray();
//				return Barscalar(sr[0].toInt(), sr[1].toInt(), sr[2].toInt());
//			}
//			else if (jsval.isDouble())
//			{
//				return Barscalar(jsval.toDouble(), BarType::FLOAT32_1);
//			}
//			else
//				return Barscalar(jsval.toInt());
//		};

//		bc::barlinevector &vec = *item;
//		for (int i = 0; i < lines.size(); ++i)
//		{
//			Barscalar valst;
//			QJsonObject objline = lines.at(i).toObject();
//			Barscalar start = parseBarscal(objline.take("s"));
//			Barscalar end = parseBarscal(objline.take("e"));
//			bc::barline *line = new bc::barline(start, end, 0);

//			size_t id = objline.take("i").toInt();

//			ids.insert(id, line);
//			QJsonArray arr = objline.take("m").toArray();
//			for (int j = 0; j < arr.size(); ++j)
//			{
//				QJsonObject objval = arr.at(j).toObject();
//				int x = objval.take("x").toInt();
//				int y = objval.take("y").toInt();
//				Barscalar val = parseBarscal(objval);

//				bc::barvalue v(x, y, val);
//				line->matr.push_back(v);
//			}
//			vec.push_back(line);
//		}

//		for (int i = 0; i < lines.size(); ++i)
//		{
//			Barscalar valst;
//			QJsonObject objline = lines.at(i).toObject();
//			QJsonArray children = objline.take("c").toArray();
//			bc::barline *cur = vec.at(i);
//			for (int j = 0; j < children.size(); ++j)
//			{
//				size_t id = children.at(j).toInt();
//				cur->children.push_back(ids[id]);
//			}
//		}

//		return true;
//	}

//public:
//	void open(BackString path)
//	{
//		jsonFile.setFileName(path);
//		if (!jsonFile.open(QIODevice::ReadOnly))
//		{
//			return;
//		}
//	}


//	bc::barline *getLine()
//	{
//		if (item == nullptr || tempLineCounter >= item->size())
//		{
//			bool read = readLine();
//			if (!read)
//				return nullptr;
//			tempLineCounter = 0;
//		}

//		return item->at(tempLineCounter++);
//	}
//};


class MainWidget;

enum MarkersShowState { found = 0, ather, barcodeNotPassed, circleNotPassed, boundyNotPassed, holmNotPassed, allExceptRed, all, none };

enum class BackPath
{
	img,
	object,
	project,
	barlist,
	roilist,
	heimap,
	tiles,
	root,
	markers,
	geojson,
	binbar,
	classifier
};


enum class ReadType
{
	Tiff,
	Simple
};

class MatrImg;

class Project
{
public:
	//	Q_PROPERTY(SeachingSettings* searchSetts READ getSerchSetts)
	//	Q_PROPERTY(SeachingSettings searchSetts MEMBER searchSetts)
	//	SeachingSettings* getSerchSetts(){return &searchSetts;}

	Project();
	~Project()
	{
		closeReader();
		closeImages();
	}
public:
	bool block = false;
	int modelWid;
	int modelHei;

	ReadType imgType;

	bool loadProject(BackString path);
	bool saveProject();

	BackString u_imgPath;
	float u_displayFactor;
	float u_imgMaxVal;
	float u_imgMinVal;
	BackString u_geojsonPath;
	BackVector<MatrImg*> images;

	int tileSize = 500;
	int tileOffset = 5;

	void closeImages();

	ImageReader *reader = nullptr;

	void setCurrentSubImage(int imgIndex)
	{
		if (proj->imgType == ReadType::Tiff)
			dynamic_cast<TiffReader *>(reader)->setCurrentSubImage(imgIndex);
	}

	void closeReader()
	{
		if (reader)
		{
			reader->close();
			delete reader;
			reader = nullptr;
		}
	}

	void openReader()
	{
		if (!reader)
		{
			if (endsWith(u_imgPath, ".tiff") || endsWith(u_imgPath, ".tif"))
			{
				imgType = ReadType::Tiff;
				reader = new TiffReader();
			}
			else
			{
				imgType = ReadType::Simple;
				reader = new SimpleImgReader();
			}
		}

		if (!reader->ready)
			reader->open((char*)getPath(BackPath::img).c_str());
	}

	static Project *proj;

	BackString status;
public:

	void setProjectPath(const BackString& path)
	{
		std::filesystem::path dir = path;
		dir = std::filesystem::absolute(dir);
/*		projectPath = (char*)dir.c_str();
		projectPath.string
		char last = projectPath[projectPath.filename().string().length() - 1];
		if (last != '\\' || last != '/')
			projectPath += '/';
*/
	}

	static Project *getProject()
	{
		if (proj == nullptr)
			proj = new Project();

		return proj;
	}
	static void dropProject()
	{
		if (proj != nullptr)
		{
			delete proj;
			proj = nullptr;
		}
	}

	MainWidget *widget = nullptr;
private:

	BackPathStr projectPath;

public:

	bool isTileCached(int ind)
	{
		BackPathStr path = getTilePath(ind);
		return fileExists(path);
	}

	BackPathStr getTilePath(int ind)
	{
		return getPath(BackPath::tiles) / intToStr(ind) / ".png";
	}

	BackPathStr getPath(BackPath pathI) const
	{
		switch (pathI)
		{
		case BackPath::tiles:
			return projectPath / "tiles/";
		case BackPath::project:
			return projectPath / "proj.qwr";
		case BackPath::barlist:
			return projectPath / "bds.json";
		case BackPath::roilist:
			return projectPath / "bds.lst";
		case BackPath::img:
			return u_imgPath;
		case BackPath::root:
			return projectPath;
		case BackPath::markers:
			return projectPath / "markers.lst";
		case BackPath::geojson:
			return u_geojsonPath;
		case BackPath::binbar:
			return projectPath / "barcode.bin";
		case BackPath::classifier:
			return projectPath / "class.json";
//		case BackPath::classifier:
//			return projectPath + "class.json";
		default:
			throw;
		}
	}

	void loadImage(BackString path, int step);
	float getImgMaxVal() const;

	float getImgMinVal() const;


	void readGeojson();
	void readMarkers();
	void readMyGeo(bool reinitY);

	void readImages();
	bc::Barcontainer *createBarcode(const bc::BarConstructor &constr, int imgIndex, int);
	void getOffsertByTileIndex(int tileIndex, uint &offX, uint &offY);

	int getFirstNormIndex()
	{
		if (!reader)
			return -1;
		if (imgType != ReadType::Tiff)
			return 0;

		TiffReader *treader = dynamic_cast<TiffReader *>(reader);

		int s = treader->getSubImageSize();
		BackPathStr tiles = getPath(BackPath::tiles);
		for (int i = 0; i < s; ++i)
		{
			treader->setCurrentSubImage(i);
			if (treader->widght() <= 2000)
			{
				return i;
			}
		}

		// None
		return -1;
	}

	BarcodeHolder threasholdLines(bc::Baritem *item);
	BarcodesHolder toHoldes(bc::barlinevector &lines, MatrImg& mat, bc::point offset);
	BarcodesHolder toHoldes(BarcodesHolder &lines, MatrImg& mat, bc::point offset);
	BarcodesHolder toHoldes(const bc::CloudPointsBarcode::CloundPoints &cloud);

private:
	void write(BackJson& json) const;
	void writeImages();
	void read(const BackJson& json);
};

#endif
