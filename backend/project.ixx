module;

#include <unordered_set>
#include <random>
#include <iostream>

#include "../Bind/Common.h"

import CacheFilesModule;
import Platform;

import ImgReader;
import SimpleImgReaderModule;
import BarholdersModule;
import IOCore;
import LayersCore;
import ClassifiersCore;
import BarcodeModule;
import GeoprocessorModule;

export module ProjectModule;


namespace bc
{
	using barlinevector = std::vector<bc::barline*>;
	using barvector = std::vector<bc::barvalue>;
}

export struct BarcodeProperies
{
	bc::barstruct barstruct;
	int alg = 0; // 0 - raster; 1 - cloud
	bool alg1UseHoles = false;
	bool alg1IgnoreHeight = false;
};

export class DataRectBarWrapper : public bc::DatagridProvider
{
	DataRect& baseobj;

	// DatagridProvider interface
public:
	DataRectBarWrapper(DataRect& base) : baseobj(base)
	{
		type = imageTypeToBar(base.data.type, base.data.samples);
	}
	int wid() const override { return baseobj.wid; }
	int hei() const override { return baseobj.hei; }
	int channels() const override { return baseobj.data.samples; }
	void maxAndMin(Barscalar& min, Barscalar& max) const override
	{
		min = getLiner(0);
		max = min;

		for (size_t i = 1; i < this->length(); ++i)
		{
			const Barscalar& s = getLiner(i);
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
	size_t typeSize() const override { return getReaderTypeSize(baseobj.data.type); }
	Barscalar get(int x, int y) const override { return rowToScalar(baseobj.get(x, y)); }
	//BarType getType()
	//{
	//	//return baseobj.data.type;
	//}

	static Barscalar rowToScalar(const roweltype& val)
	{
		Barscalar nva;
		if (val.sampl > 1)
		{
			auto* smaplsr = val.val.rgba.samples;
			switch (val.type)
			{
			case ImageType::int8:
				return Barscalar(smaplsr[0].s, smaplsr[1].s, smaplsr[2].s);
			case ImageType::int16:
				return Barscalar(smaplsr[0].s, smaplsr[1].s, smaplsr[2].s);
			case ImageType::int32:
				return Barscalar(smaplsr[0].i, smaplsr[1].i, smaplsr[2].i);
			case ImageType::float32:
				return Barscalar(smaplsr[0].f, smaplsr[1].f, smaplsr[2].f);
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

	static roweltype scalarToRow(const Barscalar& val)
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


enum MarkersShowState { found = 0, ather, barcodeNotPassed, circleNotPassed, boundyNotPassed, holmNotPassed, allExceptRed, all, none };

export enum class BackPath
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


export enum class ReadType
{
	Tiff,
	Simple
};


int getCon(int total, int part)
{
	return total / part + (total % part == 0 ? 0 : 1);
}


union SettVariant
{
	int* i;
	float* f;
	double* d;
	BackString* s;
	BackPathStr* p;
};

struct SettingValue
{
	SettVariant data;
	BackString name;

private:
	enum SettVariantType
	{
		sv_int,
		sv_float,
		sv_double,
		sv_str,
		sv_path
	};
	SettVariantType type;

public:
	SettingValue(const BackString& name, int& val)
	{
		this->name = name;
		data.i = &val;
		type = sv_int;
	}

	SettingValue(const BackString& name, float& val)
	{
		this->name = name;
		data.f = &val;
		type = sv_double;
	}
	SettingValue(const BackString& name, double& val)
	{
		this->name = name;
		data.d = &val;
		type = sv_double;
	}

	SettingValue(const BackString& name, BackString& val)
	{
		this->name = name;
		data.s = &val;
		type = sv_str;
	}
	SettingValue(const BackString& name, BackPathStr& val)
	{
		this->name = name;
		data.p = &val;
		type = sv_path;
	}

	void writeData(BackJson& json) const
	{
		switch (type)
		{
		case sv_int:
			json[name] = *data.i;
			break;
		case sv_float:
			json[name] = *data.f;
			break;
		case sv_double:
			json[name] = *data.d;
			break;
		case sv_str:
			json[name] = *data.s;
			break;
		case sv_path:
			json[name] = *data.p;
			break;
		}
	}

	void readData(const BackJson& json)
	{
		switch (type)
		{
		case sv_int:
			*data.i = json[name].get<int>();
			break;
		case sv_float:
			*data.f = json[name].get<double>();
			break;
		case sv_double:
			*data.d = json[name].get<double>();
			break;
		case sv_str:
			*data.s = json[name].get<BackString>();
			break;
		case sv_path:
			*data.p = json[name].get<BackString>();
			break;
		}
	}
};

class SettingsIO
{
	std::vector<SettingValue> settings;
public:

	SettingsIO(std::initializer_list<SettingValue> l) : settings(l)
	{ }

	void write(BackJson& json) const
	{
		for (auto& set : settings)
		{
			set.writeData(json);
		}
	}
	void read(const BackJson& json)
	{
		for (auto& set : settings)
		{
			set.readData(json);
		}
	}
};

const char* const jsn_displayFacto = "step";
const char* const jsn_imgMinVal = "imgMinVal";
const char* const jsn_imgMaxVal = "imgMaxVal";
const char* const jsn_geojsonPath = "geojsonPath";
const char* const jsn_imgPath = "imgPath";
const char* const jsn_classfiles = "barfiles";
const char* const jsn_dispalyImg = "subImageIndex";
const char* const jsn_alg = "algIndex";
const char* const jsn_tileSize = "tileSize";
const char* const jsn_tileOffset = "tileOffset";

class Project
{
	SettingsIO settings =
	{
		{jsn_displayFacto, u_displayFactor},
		{jsn_imgMaxVal, u_imgMaxVal},
		{jsn_imgMinVal, u_imgMinVal},
		{jsn_imgPath, u_imgPath},
		{jsn_geojsonPath, this->u_geojsonPath},
		{jsn_classfiles, this->u_classCache},
		{jsn_dispalyImg, this->u_subImageIndex},
		{jsn_alg, this->u_algorithm},
		{jsn_tileSize, this->tileSize},
		{jsn_tileOffset, this->tileOffset}
	};

public:
	//	Q_PROPERTY(SeachingSettings* searchSetts READ getSerchSetts)
	//	Q_PROPERTY(SeachingSettings searchSetts MEMBER searchSetts)
	//	SeachingSettings* getSerchSetts(){return &searchSetts;}

	Project()
	{
		projectPath = "D:\\Programs\\Barcode\\_bar\\_p2\\";

		// mkDirIfNotExists(u_classCache);
	}

	~Project()
	{
		closeReader();
		closeImages();
	}
public:

	int curLayerIndex;
	//ILayer* curLayer;
	LayersList<ILayer> layers;

	bool block = false;
	int modelWid;
	int modelHei;

	ReadType imgType;

	bool saveProject();

	BackPathStr u_imgPath;
	BackDirStr u_classCache;
	float u_displayFactor;
	float u_imgMaxVal;
	float u_imgMinVal;
	BackPathStr u_geojsonPath;
	int u_subImageIndex = 0;
	int u_algorithm = 0;
	std::vector<BackImage*> images;

	int tileSize = 200;
	int tileOffset = 50;

	void closeImages();

	ImageReader* reader = nullptr;

	int layerCounter = 0;
	template<class LDATA>
	LDATA* addLayerData()
	{
		LDATA* d = layers.add<LDATA>();
		d->id = layerCounter++;
		return d;
	}

	template<class LDATA>
	LDATA* addLayerData(int keepId)
	{
		LDATA* d = layers.add<LDATA>();
		d->id = keepId;
		return d;
	}

	RasterLineLayer main;

	// void setCurrentSubImage(int imgIndex, int displayWid)
	void setCurrentSubImage(int imgIndex)
	{
		int displayWid = images[imgIndex]->width();
		if (imgType == ReadType::Tiff)
		{
			dynamic_cast<TiffReader*>(reader)->setCurrentSubImage(imgIndex);
			u_displayFactor = (float)reader->width() / displayWid;
			u_subImageIndex = imgIndex;
		}
		else
		{
			u_subImageIndex = 0;
			u_displayFactor = 1.0f;
		}

		main.init(*images[imgIndex]);
		main.name = "Main";
	}

	void setReadyLaod(int curImgInd)
	{
		//int displayWid = images[curImgInd]->width();
		setCurrentSubImage(curImgInd);
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
			const auto& ext = u_imgPath.extension();
			if (ext == ".tiff" || ext == ".tif")
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

	static Project* proj;

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

	static Project* getProject()
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
			return u_classCache / "ClassFiles";
			//case BackPath::classImages:
			//	return projectPath / "classImages";
		default:
			throw;
		}
	}
//

	void loadImage(const BackPathStr& path, int step)
	{
		closeReader();

		this->u_imgPath = path;

		openReader();
		if (!reader->ready)
			return;

			//	this->u_imgMinVal = reader->min;
			//	this->u_imgMaxVal = reader->max;
			//	this->u_displayFactor = step;

		saveProject();
		writeImages();
	}

	bool loadProject(const BackPathStr& prjFilepath)
	{
		setProjectPath(prjFilepath);

		BackJson loadDoc = jsonFromFile(prjFilepath);
		read(loadDoc);

		//	qDebug() << searchSetts.height.start;
		//	qDebug() << searchSetts.heightMin();

		openReader();
		modelWid = reader->width() / u_displayFactor;
		modelHei = reader->height() / u_displayFactor;
		readImages();

		//BarClassifierCache bcc;
		//classer.categs = bcc.loadCategories();
		//classer.udpdateClasses();
		//bcc.loadClasses(getPath(BackPath::classfiles), classer);
		return true;
	}

	float getImgMaxVal() const;

	float getImgMinVal() const;
//
//
	void readGeojson();
	void readMyGeo(bool reinitY);


//
	void exportResult(int imgNumber, const BackImage& resultMart);


	void readImages();

	RasterLineLayer* createCacheBarcode(const BarcodeProperies& propertices, int imgIndex, FilterInfo* info = nullptr, int* destLayerId = nullptr)
	{
		if (block) return nullptr;

		//	reader->setCurrentSubImage(1);
		if (imgType == ReadType::Tiff)
			dynamic_cast<TiffReader*>(reader)->setCurrentSubImage(imgIndex);

		// Settup
		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = true;
		constr.attachMode = bc::AttachMode::morePointsEatLow;
		constr.returnType = bc::ReturnType::barcode2d;
		constr.structure.push_back(propertices.barstruct);
		//	constr.setStep(stepSB);

		// -------

		 //Cacher
		BarClasser* barcodeHelper;
		RasterBarClasser _rsbarcodeHelper;

		CloudBarClasser _clBarcodeHelper;

		if (propertices.alg == 0)
		{
			barcodeHelper = &_rsbarcodeHelper;
		}
		else
		{
			_clBarcodeHelper.useHoles = propertices.alg1UseHoles;
			_clBarcodeHelper.ignoreHeight = propertices.alg1IgnoreHeight;
			barcodeHelper = &_clBarcodeHelper;
		}
		barcodeHelper->doCache(true);
		barcodeHelper->setFilter(info);
		barcodeHelper->prepare(getPath(BackPath::binbar));

		const uint fullTile = tileSize + tileOffset;
		info->imgLen = fullTile * fullTile;

		uint rwid = reader->width();
		uint rhei = reader->height();
		TileIterator stW(0, tileSize, tileOffset, rwid);
		TileIterator stH(0, tileSize, tileOffset, rhei);
		LayerProvider prov(u_displayFactor);
		prov.init(tileSize, reader->width());

		RasterLineLayer* layer = nullptr;
		if (destLayerId)
		{
			if (*destLayerId == -1)
			{
				layer = addLayerData<RasterLineLayer>();
				*destLayerId = layer->id;
			}
			else
			{
				// Replace, dont add!
				layer = new RasterLineLayer();
				layer->id = *destLayerId;
				layers.set(*destLayerId, layer);
			}

			int id = getFirstNormIndex();
			layer->init(images[id]->width(), images[id]->height());
		}

		for (uint i = 0; i < rhei; i += tileSize)
		{
			uint ihei;
			if (stH.shouldSkip(ihei))
				break;

			stW.reset(0);
			std::cout << i << std::endl;
			for (uint j = 0; j < rwid; j += tileSize)
			{
				uint iwid;
				if (stW.shouldSkip(iwid))
					break;

				bc::point offset(stW.pos(), stH.pos());
				DataRect rect = reader->getRect(offset.x, offset.y, iwid, ihei);
				uint k = getTileIndexByOffset(offset.x, offset.y);

				DataRectBarWrapper warp(rect);
				barcodeHelper->create(k, &warp, constr, prov, layer);
				stW.accum();
			}
			stH.accum();
		}

		u_algorithm = propertices.alg;
		saveProject();

		return layer;
	}

	RasterLineLayer* readPrcoessBarcode(int& destLayerId, FilterInfo& filter)
	{
		if (u_displayFactor < 1.0)
			throw std::exception();

		filter.imgLen = (tileSize + tileOffset) * (tileSize + tileOffset);

		RasterLineLayer* layer = nullptr;
		RasterLineLayer* outputLayer = nullptr;
		if (destLayerId == -1)
		{
			outputLayer = layer = addLayerData<RasterLineLayer>();
			destLayerId = layer->id;
		}
		else
		{
			// Replace, dont add!
			layer = new RasterLineLayer();
			layer->id = destLayerId;
			layers.set(destLayerId, layer);
		}
		int id = getFirstNormIndex();
		layer->init(images[id]->width(), images[id]->height());

		// Cacher
		BarClasser* classifier;
		RasterBarClasser _rsClass;// (this, &filter);
		CloudBarClasser _clClass;// (this, &filter);

		if (u_algorithm == 0)
		{
			classifier = &_rsClass;
		}
		else
		{
			classifier = &_clClass;
		}

		classifier->setFilter(&filter);
		classifier->openRead(getPath(BackPath::binbar));

		LayerProvider prov(u_displayFactor);
		prov.init(tileSize, reader->width());

		while (classifier->canRead())
		{
			classifier->classBarcodeFromCache(*layer, prov);
		}

		return layer;
	}


	void getOffsertByTileIndex(uint tileIndex, uint& offX, uint& offY)
	{
		int tilesInRow = getCon(reader->width(), tileSize);
		offY = (tileIndex / tilesInRow) * tileSize;
		offX = (tileIndex % tilesInRow) * tileSize;
	}

	uint getTileIndexByOffset(uint offX, uint offY)
	{
		int tilesInRow = getCon(reader->width(), tileSize);
		return (offY / tileSize) * tilesInRow + offX / tileSize;
	}

	int getFirstNormIndex()
	{
		if (!reader)
			return -1;
		if (imgType != ReadType::Tiff)
			return 0;

		TiffReader* treader = dynamic_cast<TiffReader*>(reader);

		int s = treader->getSubImageSize();
		BackPathStr tiles = getPath(BackPath::tiles);
		for (int i = 0; i < s; ++i)
		{
			treader->setCurrentSubImage(i);
			if (treader->width() <= 2000)
			{
				return i;
			}
		}

		return -1;
	}

	BarcodeHolder threasholdLines(bc::Baritem* item);
	void addClassData(int classIndex, BarcodeHolder* points, BackImage* destIcon = nullptr);
	int addClassType(const BackString& name);
private:
	void write(BackJson& json) const;
	void writeImages();
	void read(const BackJson& json);
};


bc::barlinevector geojson[3];

using std::min;
using std::vector;

Project* Project::proj = nullptr;
using namespace bc;

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

	mkDirIfNotExists(getPath(BackPath::classfiles));
	mkDirIfNotExists(getPath(BackPath::tiles));
	//BarClassifierCache ccb;
	//ccb.saveCategories(classer.categs);
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

// ---------------------------



// ---------------------------
//
//
//bool needSkip(const Barscalar& scal) const
//{
//	return false; //scal > 10;
//}


BarcodeHolder Project::threasholdLines(bc::Baritem* item)
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

void Project::read(const BackJson& json)
{
	settings.read(json);
	//this->u_displayFactor = json[jsn_displayFacto].get<double>();
	//this->u_imgMaxVal = json[jsn_imgMaxVal].get<double>();
	//this->u_imgMinVal = json[jsn_imgMinVal].get<double>();
	//this->u_imgPath = json[jsn_imgPath].get<BackString>();
	//this->u_geojsonPath = json[jsn_geojsonPath].get<BackString>();
	//this->u_classCache = json[jsn_classfiles].get<BackString>();
	//this->u_subImageIndex = json[jsn_dispalyImg].get<int>();
	//this->u_algorithm = json[jsn_alg].get<int>();
	//this->tileSize = json[jsn_tileSize].get<int>();
	//this->tileOffset = json[jsn_tileOffset].get<int>();
}

void Project::write(BackJson& json) const
{
	settings.write(json);
	//json[jsn_displayFacto] = this->u_displayFactor;
	//json[jsn_imgMaxVal] = this->u_imgMaxVal;
	//json[jsn_imgMinVal] = this->u_imgMinVal;
	//json[jsn_imgPath] = u_imgPath.string();
	//json[jsn_geojsonPath] = this->u_geojsonPath.string();
	//json[jsn_classfiles] = this->u_classCache.string();
	//json[jsn_dispalyImg] = this->u_subImageIndex;
	//json[jsn_alg] = this->u_algorithm;
	//json[jsn_tileSize] = this->tileSize;
	//json[jsn_tileOffset] = this->tileOffset;
}

static int getFid(int wid, int s)
{
	return (wid + s - 1) / s;
}

BackImage* tiffToImg(ImageReader* reader, const BackPathStr& path, int fctor = 10, bool save = false)
{
	bool rgb = reader->getSamples() > 1;
	int widf = getFid(reader->width(), fctor);
	int heif = getFid(reader->height(), fctor);
	MatrImg* outr = new MatrImg(widf, heif, 3);

	float NAN_VALUE = reader->getNullValue();
	for (int h = 0, hr = 0; h < reader->height(); h += fctor, ++hr)
	{
		const rowptr& rp = reader->getRowData(h);
		for (int w = 0, wr = 0; w < reader->width(); w += fctor, ++wr)
		{
			float value = rp.getFloat(w);
			if (value == NAN_VALUE)
			{
				outr->set(wr, hr, Barscalar(0, 0, 0));
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
		TiffReader* trear = dynamic_cast<TiffReader*>(reader);
		int s = trear->getSubImageSize();
		for (int i = 0; i < s; ++i)
		{
			int factor = 1;
			trear->setCurrentSubImage(i);
			if (reader->width() > 2000)
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
		TiffReader* treader = dynamic_cast<TiffReader*>(reader);
		int s = treader->getSubImageSize();
		closeImages();
		for (int i = 0; i < s; ++i)
		{
			treader->setCurrentSubImage(i);
			if (reader->width() > 2000)
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
	return  static_cast<int>(val / factor);
}

void Project::readGeojson()
{

	if (imgType == ReadType::Simple)
	{
		return;
	}
	BackJson object = jsonFromFile(getPath(BackPath::geojson));

	BackJson features = object["features"].array();

	TiffReader* treader = dynamic_cast<TiffReader*>(reader);
	//	openReader();
	//	Size2 size = imgsrch.getTileSize();
	for (size_t i = 0; i < features.size(); i++)
	{
		auto arrcoors = features.at(i)["geometry"].object()["coordinates"].array();

		// Send in format x, y, T
		//QVector3D coord(arrcoors[0].toDouble(), arrcoors[1].toDouble(), 0);
		//coord = treader->curSubImage->convertModelToRaster(coord);
		//// Get in format x, T, y

		//if (coord.x() < 0 || coord.z() < 0 || coord.x() >= reader->width() || coord.z() >=reader->height())
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

	//BarClassifierCache saver;
	//saver.save(points, classIndex, destIcon);
	//classer.addData(classIndex, points->lines, false);
}

int Project::addClassType(const BackString& name)
{
	//classer.classes.push_back(Barcontainer());
	//return classer.categs.addValue(name);
	return 0;
}


export class Project;
