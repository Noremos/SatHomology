#ifndef PROJECT_H
#define PROJECT_H

#include "SimpleImgReader.h"
#include "barcodeCreator.h"
#include "BinFile.h"

#include "ReaderTypes.h"
#include "tiffreader.h"
#include "Common.h"

#include "geodataprocessor.h"

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
	DataRectBarWrapper(DataRect &base) : baseobj(base)
	{
		type = imageTypeToBar(base.data.type, base.data.samples);
	}
	int wid() const override { return baseobj.wid; }
	int hei() const override { return baseobj.hei; }
	int channels() const override { return baseobj.data.samples; }
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
	//BarType getType()
	//{
	//	//return baseobj.data.type;
	//}

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

	static BarType imageTypeToBar(const ImageType type, int samples)
	{
		if (samples > 1)
		{
			switch (type)
			{
			case ImageType::int8:
				return BarType::BYTE8_3;
			default:
				throw;
			}
		}
		switch (type)
		{
		case ImageType::int8:
			return BarType::BYTE8_1;
		default:
			throw;
		}
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
	classifier,
	classfiles
	//classImages,
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

	bool loadProject(const BackPathStr& path);
	bool saveProject();

	BackPathStr u_imgPath;
	BackDirStr u_classCache;
	float u_displayFactor;
	float u_imgMaxVal;
	float u_imgMinVal;
	BackPathStr u_geojsonPath;
	int u_subImageIndex = 0;
	BackVector<MatrImg*> images;

	int tileSize = 200;
	int tileOffset = 50;

	void closeImages();

	ImageReader *reader = nullptr;

	void setCurrentSubImage(int imgIndex, int displayWid)
	{
		if (imgType == ReadType::Tiff)
		{
			dynamic_cast<TiffReader *>(reader)->setCurrentSubImage(imgIndex);
			u_displayFactor = (float)reader->widght() / displayWid;
			u_subImageIndex = imgIndex;
		}
		else
		{
			u_subImageIndex = 0;
			u_displayFactor = 1.0f;
		}
	}

	void setReadyLaod(int curImgInd, int displayWid)
	{
		setCurrentSubImage(curImgInd, displayWid);
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
			const auto &ext = u_imgPath.extension();
			if (ext  == ".tiff" || ext == ".tif")
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
			reader->open(getPath(BackPath::img).string());
	}

	static Project *proj;

	BackString status;
public:

	void setProjectPath(const BackPathStr& path)
	{
		std::filesystem::path dir = path;
		dir = std::filesystem::absolute(dir).parent_path();
		projectPath = dir;
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

	static BackPathStr getPathSt(BackPath pType)
	{
		return getProject()->getPath(pType);
	}

	MainWidget *widget = nullptr;
private:

	BackDirStr projectPath;

public:

	bool isTileCached(int ind)
	{
		BackPathStr path = getTilePath(ind);
		return pathExists(path);
	}

	BackPathStr getTilePath(int ind)
	{
		return getPath(BackPath::tiles) / (intToStr(ind) + ".png");
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
			//return u_geojsonPath;
			return projectPath / "geojson.json";
		case BackPath::binbar:
			return projectPath / "barcode.bin";
		case BackPath::classifier:
			return projectPath / "class.json";
		case BackPath::classfiles:
			return u_classCache;
		//case BackPath::classImages:
		//	return projectPath / "classImages";
		default:
			throw;
		}
	}

	void loadImage(const BackPathStr& path, int step);
	float getImgMaxVal() const;

	float getImgMinVal() const;


	void readGeojson();
	void readMarkers();
	void readMyGeo(bool reinitY);


	struct ClassInfo
	{
		int ind;
		MatrImg& mat;
		std::unordered_map<size_t, char>& map;
		BackString extra;
		std::vector<std::shared_ptr<SimpleLine>>& resLinesMaps;
	};
	void classBarcode(BarcodesHolder& baritem, ClassInfo& info);

	void readPrcoessBarcode(ClassInfo& info);


	void exportResult(int imgNumber, const BackImage& resultMart);


	void readImages();

	void createCacheBarcode(const bc::BarConstructor& constr, int imgIndex, const FilterInfo& info);
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

	BarcodeHolder threasholdLines(bc::Baritem* item);
	void addClassData(int classIndex, BarcodeHolder* points, BackImage* destIcon = nullptr);
	int addClassType(const BackString& name);
private:
	void write(BackJson& json) const;
	void writeImages();
	void read(const BackJson& json);


public:
	barclassificator classer;
	BarClassifierCache classLoader;
private:
	std::vector<Barscalar> colors;
};

#endif
