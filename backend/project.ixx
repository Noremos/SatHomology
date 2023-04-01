module;

#include <unordered_set>
#include <random>
#include <assert.h>
//#include <iostream>

#include "../Bind/Common.h"


export module ProjectModule;

import Platform;

import ImgReader;
import IOCore;
import LayersCore;
import BarcodeModule;
import GeoprocessorModule;
import JsonCore;
import TrainIO;
import RasterLayers;
import Classifiers;
import MetadataIOCore;
import VectorLayers;
import MHashMap;
import CSBind;



namespace bc
{
	using barlinevector = std::vector<bc::barline*>;
	using barvector = std::vector<bc::barvalue>;
}


export struct InOutLayer
{
	int in;
	int subImgIndex = 0;
	int out;
	bool skipOut = false;

	bool skipOutput()
	{
		return skipOut;
	}

	bool isOutEmpty()
	{
		return out == -1 || in == out;
	}
};

export struct BarcodeProperies
{
	bc::barstruct barstruct;
	int alg = 0; // 0 - raster; 1 - cloud
	bool alg1UseHoles = false;
	bool alg1IgnoreHeight = false;
};


struct LCoreItems
{
	ILayer* core;
	BackString name;

	LCoreItems(ILayer* core) : core(core)
	{ }

	LCoreItems(ILayer* core, const BackString name) :
		core(core), name(name)
	{ }

	const BackString& getName(const BackString& def) const
	{
		if (name.length() == 0)
			return def;
		else
			return name;
	}
};

export using RetLayers = std::vector<LCoreItems>;

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
	classifier,
	classfiles,
	metadata,
	layers
	//classImages,
};

int getCon(int total, int part)
{
	return total / part + (total % part == 0 ? 0 : 1);
}



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

export class Project
{
	SettingsIO settings =
	{
		//{jsn_displayFacto, u_displayFactor},
		//{jsn_imgMaxVal, u_imgMaxVal},
		//{jsn_imgMinVal, u_imgMinVal},
		{"metacounter", metaCounter},
		//{jsn_imgPath, u_imgPath},
		//{jsn_geojsonPath, this->u_geojsonPath},
		//{jsn_classfiles, this->u_classCache},
		//{jsn_dispalyImg, this->u_subImageIndex},
		//{jsn_alg, this->u_algorithm},
		//{jsn_tileSize, this->tileSize},
		//{jsn_tileOffset, this->tileOffset}
	};

	DisplaySystem ds;

	int metaCounter = 0;
	std::unique_ptr<MetadataProvider> metaprov;

	void extraRead(const BackJson& json)
	{
		JsonObjectIOStateReader reader(json);
		extraReadWrite(&reader);
	}

	void extraWrite(BackJson& json)
	{
		JsonObjectIOStateWriter writer(json);
		extraReadWrite(&writer);
	}

	template<class P>
	static void ioPoint(JsonObjectIOState* state, BackString name, const P& p)
	{
		state->scDouble(name + "_x", p.x);
		state->scDouble(name + "_y", p.y);
	}

	void extraReadWrite(JsonObjectIOState* state)
	{
		const bool isReading = state->isReading();

		int size = layers.size();
		JsonArrayIOState* arrst = state->arrayBegin("layers", size);

		for (int i = 0; i < size; i++)
		{
			JsonObjectIOState* obj = arrst->objectBegin(i);
			MetadataProvider sub = *metaprov;// Do not reffer!

			int layId;
			ILayer* lay;
			if (isReading)
			{
				obj->scInt("layId", layId);
				lay = CoreLayerFactory::CreateCoreLayer(layId);
				layers.addMove(lay);
			}
			else
			{
				lay = layers.at(i);
				layId = layers.at(i)->getFactoryId();
				obj->scInt("layId", layId);
			}

			lay->saveLoadState(obj, sub);
			arrst->objectEnd();
		}
		state->arrayEnd();

		size = classLayers.size();
		arrst = state->arrayBegin("classBind", size);
		auto cb = classLayers.begin();
		for (int i = 0; i < size; i++)
		{
			JsonObjectIOState* obj = arrst->objectBegin(i);

			int clId;
			int layId;
			if (!isReading)
			{
				clId = cb->first;
				layId = cb->second->id;
				++cb;
			}
			obj->scInt("class_id", clId);
			obj->scInt("layer_id", layId);

			if (isReading)
			{
				classLayers[clId] = static_cast<VectorLayer*>(layers.at(layId));
			}
			arrst->objectEnd();
		}
		state->arrayEnd();

		ds.saveLoadState(state, getMeta());
	}
public:
	//	Q_PROPERTY(SeachingSettings* searchSetts READ getSerchSetts)
	//	Q_PROPERTY(SeachingSettings searchSetts MEMBER searchSetts)
	//	SeachingSettings* getSerchSetts(){return &searchSetts;}

	Project()
	{
		projectPath = "";
		// "D:\\Programs\\Barcode\\_bar\\_p2\\";
		settings.extraRead = [this](const BackJson& json) {extraRead(json);};
		settings.extraWrite = [this](BackJson& json) {extraWrite(json);};

		ds.csPos = BackPoint(0, 0);
		ds.csSize = BackPoint(1000, 1000);

		// mkDirIfNotExists(u_classCache);
	}

	~Project()
	{
		if (projectPath.has_filename())
		{
			saveProject();
		}

		//closeImages();
	}

	DisplaySystem& getDisplay()
	{
		return ds;
	}

	bool prjCreate = false;
public:
	BarCategories classCategs;
	barclassificator classifier;

	int curLayerIndex;
	LayersList<ILayer> layers;

	bool block = false;

	BackDirStr u_classCache;
	int u_algorithm = 0;

	int layerCounter = 0;
	template<class LDATA>
	LDATA* addLayerData(int projId = -1)
	{
		LDATA* d = layers.add<LDATA>();
		if (projId != -1)
			d->cs.init(projId);

		//d->prov.init(u_displayFactor, tileSize, reader->width());
		d->id = layerCounter++;
		return d;
	}

	template<class LDATA>
	LDATA* addLayerData(int keepId, int projId)
	{
		LDATA* d = layers.add<LDATA>();
		d->cs.init(projId);

		//d->prov.init(u_displayFactor, tileSize, reader->width());
		d->id = keepId;
		return d;
	}

	template<class LDATA>
	LDATA* changeLayerData(int id, int projId)
	{
		// Replace, dont add!
		LDATA* layer = new LDATA();
		layer->id = id;
		layer->cs.init(projId);
		layers.set(id, layer);
		return layer;
	}

	template<class LDATA>
	LDATA* addOrUpdateOut(InOutLayer& iol, int projId)
	{
		LDATA* layer;
		if (iol.isOutEmpty())
		{
			layer = addLayerData<LDATA>(projId);
			iol.out = layer->id;
		}
		else
		{
			// Replace, dont add!
			layer = changeLayerData<LDATA>(iol.out, projId);
		}

		return layer;
	}

	// template<class LDATA>
	// LDATA* addOrUpdateOut(InOutLayer& iol)
	// {
	// 	LDATA* layer = addOrUpdateOut(iol);
	// 	layer->init(getInRaster(iol));
	// 	return layer;
	// }

	IRasterLayer* getInRaster(int inId, int subImgIndex = -1)
	{
		ILayer* layer = layers.at(inId);
		auto rlayer = dynamic_cast<IRasterLayer*>(layer);
		assert(rlayer != nullptr);
		rlayer->setSubImage(subImgIndex);
		return rlayer;
	}

	IRasterLayer* getInRaster(const InOutLayer& iol)
	{
		return getInRaster(iol.in, iol.subImgIndex);
	}


	template<class T>
	T* getInTRaster(const int in)
	{
		return dynamic_cast<T*>(getInRaster(in, 0));
	}

	template<class T>
	T* getInTRaster(const InOutLayer& iol)
	{
		return dynamic_cast<T*>(getInRaster(iol.in, iol.subImgIndex));
	}
	//RasterLayer main;

	void setReadyLaod(int curImgInd)
	{
		//int displayWid = images[curImgInd]->width();
		//setSubImage(curImgInd);

		classCategs = BarCategories::loadCategories(getPath(BackPath::classifier));

		classifier.open(getMetaPath());
		classifier.loadData(classCategs);
		//classifier.categs
	}

	static Project* proj;

	BackString status;
public:
	void setProjectPath(const BackPathStr& path)
	{
		std::filesystem::path dir = path;
		dir = std::filesystem::absolute(dir).parent_path();
		projectPath = dir;

		settupMeta();

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
	// bool isTileCached(int ind)
	// {
	// 	BackPathStr path = getTilePath(ind);
	// 	return pathExists(path);
	// }

	// BackPathStr getTilePath(int ind)
	// {
	// 	return getPath(BackPath::tiles) / (intToStr(ind) + ".png");
	// }

	BackPathStr getPath(BackPath pathI) const
	{
		switch (pathI)
		{
		// case BackPath::tiles:
		// 	return projectPath / "tiles/";
		case BackPath::project:
			return projectPath / "proj.qwr";
		case BackPath::barlist:
		// 	return projectPath / "bds.json";
		// case BackPath::roilist:
		// 	return projectPath / "bds.lst";
		case BackPath::root:
			return projectPath;
		case BackPath::markers:
			return projectPath / "markers.lst";
		case BackPath::geojson:
			//return u_geojsonPath;
			return projectPath / "geojson.json";
		case BackPath::classifier:
			return getPath(BackPath::metadata) / "class.json";
		// case BackPath::classfiles:
		// 	return u_classCache / "ClassFiles";
			//case BackPath::classImages:
			//	return projectPath / "classImages";
		case BackPath::metadata:
			return projectPath / "Metadata";
		case BackPath::layers:
			return getPath(BackPath::metadata) / "layers.json";
		default:
			throw;
		}
	}

	MetadataProvider& getMeta()
	{
		return *metaprov.get();
	}

	BackPathStr getMetaPath(const BackString& item) const
	{
		return getPath(BackPath::metadata) / item;
	}

	BackPathStr getMetaPath() const
	{
		return getPath(BackPath::metadata);
	}

	void settupMeta()
	{
		mkDirIfNotExists(getPath(BackPath::metadata));
		metaprov.reset(new MetadataProvider(getPath(BackPath::metadata), metaCounter));
		classifier.open(getMetaPath());
	}

	RasterFromDiskLayer* loadImage(const BackPathStr& path, int step)
	{
		prjCreate = true;
			//	this->u_imgMinVal = reader->min;
			//	this->u_imgMaxVal = reader->max;
			//	this->u_displayFactor = step;
		RasterFromDiskLayer* layer = addLayerData<RasterFromDiskLayer>();
		layer->open(path, getMeta());

		saveProject();

		return layer;
	}

	bool loadProject(const BackPathStr& prjFilepath)
	{
		setProjectPath(prjFilepath);

		BackJson loadDoc = jsonFromFile(prjFilepath);
		read(loadDoc);
		prjCreate = true;

		classCategs = BarCategories::loadCategories(getPath(BackPath::classifier));

		classifier.loadData(classCategs);
		return true;
	}

	bool saveProject()
	{
		if (!prjCreate)
			return false;

		JsonObject gameObject;
		write(gameObject);
		jsonToFile(gameObject, getPath(BackPath::project));

		// mkDirIfNotExists(getPath(BackPath::classfiles));
		// mkDirIfNotExists(getPath(BackPath::tiles));

		classCategs.saveCategories(getPath(BackPath::classifier));
		return true;
	}

	void readGeojson();
	void readMyGeo(bool reinitY);

	void exportResult(int imgNumber, const BackImage& resultMart);

	int predict(const IClassItem* item)
	{
		int classType = classifier.predict(item);
		return classType;
	}

	RetLayers createCacheBarcode(InOutLayer& iol, const BarcodeProperies& propertices, FilterInfo* info = nullptr)
	{
		RetLayers ret;
		if (block) return ret;

		// Settup
		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = true;
		constr.attachMode = bc::AttachMode::morePointsEatLow;
		constr.returnType = bc::ReturnType::barcode2d;
		constr.structure.push_back(propertices.barstruct);
		//	constr.setStep(stepSB);

		// -------
		IRasterLayer* inLayer = getInRaster(iol);

		uint rwid = inLayer->realWidth();
		uint rhei = inLayer->realHeight();

		int tileSize = inLayer->prov.tileSize;
		int tileOffset = inLayer->tileOffset;

		if (info)
		{
			const uint fullTile = tileSize + tileOffset;
			info->imgLen = fullTile * fullTile;
		}

		TileIterator stW(0, tileSize, tileOffset, rwid);
		TileIterator stH(0, tileSize, tileOffset, rhei);

		RasterLineLayer* layer = addOrUpdateOut<RasterLineLayer>(iol, inLayer->cs.getProjId());
		layer->init(inLayer, getMeta());
		layer->initCSFrom(inLayer->cs);

		if (layer->cacheId == -1)
			layer->cacheId = metaprov->getUniqueId();

		ret.push_back(layer);
		for (auto& i : classLayers)
		{
			ret.push_back(i.second);
		}

		// Cacher
		ItemHolderCache cacher;
		cacher.openWrite(layer->getCacheFilePath(getMeta()));

		IdGrater parentne;
		uint tileIndex = 0;
		int inde = 0;
		IClassItemHolder::ItemCallback cacheClass;

		cacheClass = [this, &parentne, &inde, layer, &tileIndex, info](IClassItem* item)
		{
			if (layer->passLine(item, info))
			{
				auto id = predict(item);
				if (id != -1)
				{
					VectorLayer* vl = classLayers.at(id);

					bc::barvector temp;
					getCountourSimple(item->getMatrix(), temp);

					auto& p = vl->addPrimitive(vl->color);
					for (const auto& pm : temp)
					{
						p.addPoint(BackPoint(pm.getX(), pm.getY()));
					}
				}
				else
					layer->addLine(parentne, inde++, item, tileIndex);
			}
		};

		for (uint i = 0; i < rhei; i += tileSize)
		{
			uint ihei;
			if (stH.shouldSkip(ihei))
				break;

			stW.reset(0);
			//std::cout << i << std::endl;
			for (uint j = 0; j < rwid; j += tileSize)
			{
				uint iwid;
				if (stW.shouldSkip(iwid))
					break;

				bc::point offset(stW.pos(), stH.pos());
				BackImage rect = inLayer->getRect(offset.x, offset.y, iwid, ihei);
				tileIndex = inLayer->prov.tileByOffset(offset.x, offset.y).index;

				BaritemHolder creator;
				inde = 0; // Keep this! See lyambda
				creator.create(&rect, constr, cacheClass);
				cacher.save(&creator, tileIndex);

				stW.accum();
			}
			stH.accum();
		}

		u_algorithm = propertices.alg;
		saveProject();

		return ret;
	}

	RetLayers processCachedBarcode(InOutLayer& iol, FilterInfo* filter)
	{
		//if (u_displayFactor < 1.0)
		//	throw std::exception();

		RasterLineLayer* inLayer = getInTRaster<RasterLineLayer>(iol);
		if (!inLayer)
			return RetLayers();

		int tileSize = inLayer->prov.tileSize;
		int tileOffset = inLayer->tileOffset;

		if (filter)
		{
			const uint fullTile = tileSize + tileOffset;
			filter->imgLen = fullTile * fullTile;
		}

		// -------
		RetLayers ret;
		for (auto& i : classLayers)
			ret.push_back(i.second);

		// RasterLineLayer* outLayer = addOrUpdateOut<RasterLineLayer>(iol);
		// if (outLayer->cacheId == -1)
		// 	outLayer->cacheId = metaprov->getUniqueId();
		// ret.push_back(outLayer);
		RasterLineLayer* outLayer = inLayer;
		outLayer->init(inLayer, getMeta());

		// Cacher
		ItemHolderCache cacher;
		cacher.openRead(inLayer->getCacheFilePath(getMeta()));

		int tileIndex = 0;
		while (cacher.canRead())
		{
			BaritemHolder holder;
			cacher.load(tileIndex, &holder);

			IdGrater parentne;

			const auto& vec = holder.getItems();
			for (size_t i = 0; i < vec.size(); ++i)
			{
				auto item = vec.at(i);
				if (outLayer->passLine(item, filter))
				{
					auto id = predict(item);
					if (id != -1)
					{
						VectorLayer* vl = classLayers.at(id);

						bc::barvector temp;
						getCountourSimple(item->getMatrix(), temp);

						auto& p = vl->addPrimitive(vl->color);
						for (const auto& pm : temp)
						{
							BackPoint iglob = vl->cs.toGlobal(pm.getX(), pm.getY());
							p.addPoint(iglob);
						}
					}
					else
						outLayer->addLine(parentne, (int)i, item, tileIndex);
				}
			}
		}

		return ret;
	}

	//void getOffsertByTileIndex(uint tileIndex, uint& offX, uint& offY)
	//{
	//	int tilesInRow = getCon(reader->width(), tileSize);
	//	offY = (tileIndex / tilesInRow) * tileSize;
	//	offX = (tileIndex % tilesInRow) * tileSize;
	//}

	//uint getTileIndexByOffset(uint offX, uint offY)
	//{
	//	int tilesInRow = getCon(reader->width(), tileSize);
	//	return (offY / tileSize) * tilesInRow + offX / tileSize;
	//}


	// BarcodeHolder threasholdLines(bc::Baritem* item)
	// {
	// 	BarcodeHolder vec;
	// 	for (size_t i = 0; i < item->barlines.size(); ++i)
	// 	{
	// 		if (item->barlines[i]->len() < 10)
	// 		{
	// 			vec.lines.push_back(item->barlines[i]->clone());
	// 		}
	// 	}
	// 	return vec;
	// }

	MMMAP<int, VectorLayer*> classLayers;
	int addClassType(const BackString& name)
	{
		int id = classCategs.addValue(name);
		classifier.addClass(id);
		auto* layer = addLayerData<VectorLayer>(DEFAULT_PROJECTION);
		layer->name = "Class: " + name;
		layer->color = BackColor::random();
		layer->vecType = VectorLayer::VecType::polygons;
		classLayers[id] = layer;
		saveProject();
		return id;
	}

	void changeClassName(int classId, const BackString& name)
	{
		classCategs.changeName(classId, name);
	}

	void removeClassType(int classId)
	{
		classCategs.remove(classId);
		classifier.removeClass(classId);
		classLayers.erase(classId);
	}

	size_t addTrainData(int layerId, int classId, CachedObjectId srcItemId, BackImage* destIcon)
	{
		auto inLayer = getInTRaster<RasterLineLayer>(layerId);
		assert(inLayer);

		ItemHolderCache cached;
		cached.openRead(inLayer->getCacheFilePath(getMeta()));

		BaritemHolder item;
		cached.loadSpecific(srcItemId.tileId, &item);

		auto line = item.getItems()[srcItemId.vecId];
		if (destIcon != nullptr)
		{
			auto rect = bc::getBarRect(line->getMatrix());
			*destIcon = inLayer->getRect(rect.x, rect.y, rect.width, rect.height);
		}
		//rb->barlines[srcItemId.vecId] = nullptr;

		assert(proj->classCategs.size() !=0);
		return classifier.addData(classId, line, destIcon);
	}

	void removeTrainData(int classId, int localId)
	{
		classifier.removeData(classId, localId);
	}

	RetLayers exeFilter(InOutLayer& iol, int algNum)
	{
		//if (u_displayFactor < 1.0)
		//	throw std::exception();
		IRasterLayer* input = getInRaster(iol);

		RetLayers ret;
		RasterLayer* layer = addOrUpdateOut<RasterLayer>(iol, input->cs.getProjId());
		layer->initCSFrom(input->cs);

		ret.push_back(layer);
		layer->init(input);



		const BackImage src = *(input->getCachedImage());

		uint hist[256];//256
		uint offs[256];//256
		std::fill_n(hist, 256, 0);
		std::fill_n(offs, 256, 0);
		for (size_t i = 0; i < src.length(); i++)
		{
			auto p = (int)src.getLiner(i);
			++hist[p];//����� vector, �� ��
		}

		for (size_t i = 1; i < 256; ++i)
		{
			hist[i] += hist[i - 1];
			offs[i] = hist[i - 1];
		}

		std::unique_ptr<uint> ods;
		uint* data = new uint[src.length()];//256
		ods.reset(data);

		for (size_t i = 0; i < src.length(); i++)
		{
			uchar p = src.getLiner(i).getAvgUchar();
			data[offs[p]++] = i;
		}

		//std::reverse(data, data + src.length());

		std::vector<char> setted;
		setted.resize(src.length());
		std::fill(setted.begin(), setted.end(), 0);

		BackImage& imgout = layer->mat;
		imgout = src;

		//.width(), src.hei(), src.channels());
		//imgout.fill(0);

		const char poss[9][2] = { { -1,0 },{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },{ 1,1 },{ 0,1 },{ -1,1 },{ -1,0 } };
		for (size_t i = 0; i < src.length(); i++)
		{
			auto dat = data[src.length() - i - 1];
			auto p = bc::barvalue::getStatPoint(dat, src.width());
			if (setted[i] == 10)
			{
				continue;
			}

			Barscalar val = imgout.getLiner(dat);

			for (int u = 0; u < 8; ++u)
			{
				bc::point IcurPoint(p + poss[u]);

				if (IcurPoint.x < 0 || IcurPoint.x >= src.width() || IcurPoint.y < 0 || IcurPoint.y >= src.height())
					continue;

				auto re = IcurPoint.getLiner(src.wid());

				//if (setted[re] == 10)
				//{
				//	continue;
				//}

				Barscalar valNext = imgout.get(IcurPoint.x, IcurPoint.y);
				if (valNext.absDiff(val) < 15)
					imgout.set(IcurPoint.x, IcurPoint.y, val);
				else
				{
					setted[re] = 10;
					//imgout.set(IcurPoint.x, IcurPoint.y, 0);
				}
			}
		}

		return ret;
	}

private:
	void write(BackJson& json) const;
	void read(const BackJson& json);
};


bc::barlinevector geojson[3];

using std::min;
using std::vector;

Project* Project::proj = nullptr;
using namespace bc;

// ---------------------------



// ---------------------------
//
//
//bool needSkip(const Barscalar& scal) const
//{
//	return false; //scal > 10;
//}




void Project::read(const BackJson& json)
{
	settings.read(json);
}

void Project::write(BackJson& json) const
{
	settings.write(json);
}

static int getFid(int wid, int s)
{
	return (wid + s - 1) / s;
}

int normal(float val, int factor)
{
	return  static_cast<int>(val / factor);
}

void Project::readGeojson()
{
	//if (imgType == ReadType::Simple)
	//{
	//	return;
	//}
	BackJson object = jsonFromFile(getPath(BackPath::geojson));

	BackJson features = object["features"];// .array();

	//TiffReader* treader = dynamic_cast<TiffReader*>(reader);
	//	openReader();
	//	Size2 size = imgsrch.getTileSize();
	for (size_t i = 0; i < features.size(); i++)
	{
		//auto arrcoors = features.at(i)["geometry"].object()["coordinates"].array();

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
