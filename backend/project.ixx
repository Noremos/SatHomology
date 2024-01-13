module;

#include <unordered_set>
#include <random>
#include <assert.h>
#include <chrono>
#include <algorithm>
#include <mutex>
//#include <iostream>

#include "../Bind/Common.h"


//#include "../side/Barcode/PrjBarlib/include/CellEater.h"


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
import RasterBarHolderRLayer;
import Settings;
import EnergyModule;

// using CurItemHolder = BaritemHolder;
using CurItemHolder = BettyItemHolder;


template<class T>
constexpr T mmmin(T a, T b)
{
	return a > b ? b : a;
}

template<class T>
constexpr T mmmax(const T a, const T b)
{
	return a > b ? a : b;
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


export enum class GuiState
{
	Empty = 0,
	Loaded
};

int getCon(int total, int part)
{
	return total / part + (total % part == 0 ? 0 : 1);
}

export class Project
{
	ProjectSettings& settings;
	DisplaySystem ds;

	int metaCounter = 0;

	void extraRead(const BackJson& json)
	{
		layers.clear();
		classLayers.clear();
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
		auto iter = layers.begin();
		JsonArrayIOState* arrst = state->arrayBegin("layers", size);

		for (int i = 0; i < size; i++)
		{
			JsonObjectIOState* obj = arrst->objectBegin(i);
			MetadataProvider sub = *settings.metaprov;// Do not reffer!

			int factoryId;
			ILayer* lay;
			if (isReading)
			{
				obj->scInt("factoryId", factoryId);
				lay = CoreLayerFactory::CreateCoreLayer(factoryId);
				if (lay == nullptr)
					continue; // Factory does not exist

				layers.addMove(lay);
			}
			else
			{
				lay = iter->get();
				factoryId = lay->getFactoryId();
				obj->scInt("factoryId", factoryId);
				iter++;
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

			int classId;
			int coreId;
			if (!isReading)
			{
				classId = cb->first;
				coreId = cb->second->id;
				++cb;
			}
			obj->scInt("class_id", classId);
			obj->scInt("layer_id", coreId);

			if (isReading)
			{
				auto* vectLayer = layers.at(coreId);
				if (vectLayer)
					classLayers[classId] = static_cast<VectorLayer*>(vectLayer);
				else
				{
					auto* layer = addClassLayer(classId);
					layer->name = "Class layer";
				}
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

	Project() : settings(getSettings())
	{
		projectPath = "";
		// "D:\\Programs\\Barcode\\_bar\\_p2\\";
		settings.getIO().extraRead = [this](const BackJson& json) {extraRead(json);};
		settings.getIO().extraWrite = [this](BackJson& json) {extraWrite(json);};

		ds.csPos = BackPoint(0, 0);
		ds.csScale = 1.0;//BackPoint(1000, 1000);
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
	GuiState state = GuiState::Empty;

	BarCategories classCategs;
	BettyClassificator classifier;

	int curLayerIndex;
	LayersList<ILayer> layers;

	bool block = false;

	BackDirStr u_classCache;
	int u_algorithm = 0;

	template<class LDATA>
	LDATA* addLayerData(int projId = -1)
	{
		LDATA* d = layers.add<LDATA>();
		if (projId != -1)
			d->cs.init(projId);

		//d->prov.init(u_displayFactor, tileSize, reader->width());
		d->id = settings.layerCounter++;
		return d;
	}

	// template<class LDATA>
	// LDATA* addLayerData(int keepId, int projId)
	// {
	// 	LDATA* d = layers.add<LDATA>();
	// 	d->cs.init(projId);

	// 	//d->prov.init(u_displayFactor, tileSize, reader->width());
	// 	d->id = keepId;
	// 	return d;
	// }

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
		//rlayer->setSubImage(subImgIndex);
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
	void setProjectPath(const BackPathStr& path, bool recreateMeta = false)
	{
		std::filesystem::path dir = path;
		dir = std::filesystem::absolute(dir).parent_path();
		projectPath = dir;

		settupMeta(recreateMeta);

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
		return *settings.metaprov.get();
	}

	BackPathStr getMetaPath(const BackString& item) const
	{
		return getPath(BackPath::metadata) / item;
	}

	BackPathStr getMetaPath() const
	{
		return getPath(BackPath::metadata);
	}

	void settupMeta(bool recreateMeta)
	{
		auto path = getMetaPath();
		if (pathExists(path))
		{
			if (recreateMeta)
			{
				dropDirIfExists(path);
				mkdir(path);
			}
		}
		else
			mkdir(path);

		settings.metaprov.reset(new MetadataProvider(path, metaCounter));
		classifier.open(path);
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
		settings.getIO().read(loadDoc);
		prjCreate = true;

		classCategs = BarCategories::loadCategories(getPath(BackPath::classifier));

		classifier.loadData(classCategs);
		return true;
	}

	bool saveProject()
	{
		if (!prjCreate)
			return false;

		JsonObject sets;
		settings.getIO().write(sets);
		jsonToFile(sets, getPath(BackPath::project));

		// mkDirIfNotExists(getPath(BackPath::classfiles));
		// mkDirIfNotExists(getPath(BackPath::tiles));

		classCategs.saveCategories(getPath(BackPath::classifier));
		return true;
	}

	void readGeojson();
	void readMyGeo(bool reinitY);

	int predict(const IClassItem* item)
	{
		int classType = classifier.predict(item);
		return classType;
	}

	RetLayers createCacheBarcode(InOutLayer& iol, const BarcodeProperies& propertices, IItemFilter* filter = nullptr)
	{
		if (block) return {};
		IRasterLayer* inLayer = getInRaster(iol);

		RasterLineLayer* layer = addOrUpdateOut<RasterLineLayer>(iol, inLayer->cs.getProjId());
		auto ret = layer->createCacheBarcode(inLayer, propertices, filter);

		u_algorithm = propertices.alg;
		saveProject();

		return ret;
	}


	BackPoint toGlob(const CSBinding& cs, const TileProvider& tileProv, float diplsayToRealFactor, BackPoint p)
	{
		BackPixelPoint op = tileProv.tileToPreview(p.x, p.y); // To display
		BackPoint iglob((static_cast<float>(op.x) + 0.5f), static_cast<float>(op.y) + 0.5f);
		return cs.toGlobal(iglob.x * diplsayToRealFactor, iglob.y * diplsayToRealFactor); // To real
	}

	std::mutex addPrimitiveMutex;
	bool predictForLayer(IClassItem* item, const TileProvider& tileProv, float diplsayToRealFactor)
	{
		auto classId = predict(item);
		if (classId != -1)
		{
			VectorLayer* vl = classLayers.at(classId);
			assert(vl != nullptr);
			CSBinding& cs = vl->cs;

			DrawPrimitive* p;
			{
				std::lock_guard<std::mutex> guard(addPrimitiveMutex);
				p = vl->addPrimitive(vl->color);
			}

			//BackPoint center;
			//float r;
			//getCircle(item->getMatrix(), center, r);
			//BackPoint rb(center.x + r, center.y + r);
			//BackPoint iglob = toGlob(cs, tileProv, diplsayToRealFactor, center);
			//BackPoint globRb = toGlob(cs, tileProv, diplsayToRealFactor, rb);
			//p->setCircle(iglob, globRb);


			mcountor temp;
			getCountour(item->getMatrix(), temp, true);
			for (const auto& pm : temp)
			{
				auto point = bc::barvalue::getStatPoint(pm);

				BackPixelPoint op = tileProv.tileToPreview(point.x, point.y); // To display
				BackPoint iglob((static_cast<float>(op.x) + 0.5f), static_cast<float>(op.y) + 0.5f);

				iglob = cs.toGlobal(iglob.x * diplsayToRealFactor, iglob.y * diplsayToRealFactor); // To real
				p->addPoint(iglob);
			}

			return true;
		}
		else
			return false;
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
		int classId = classCategs.addValue(name);
		classifier.addClass(classId);
		auto* layer = addClassLayer(classId);
		layer->name = "Class: " + name;

		saveProject();
		return classId;
	}

	VectorLayer* addClassLayer(int classId, bool addToList = true)
	{
		auto* layer = addLayerData<VectorLayer>(ds.sysProj.getId());
		layer->color = BackColor::random();
		//layer->vecType = VectorLayer::VecType::circles;
		layer->vecType = VectorLayer::VecType::polygons;
		if (addToList)
		{
			layer->isSystem = true;
			classLayers[classId] = layer;
		}

		return layer;
	}

	void changeClassName(int classId, const BackString& name)
	{
		classCategs.changeName(classId, name);
	}

	void removeClassType(int classId)
	{
		classCategs.remove(classId);
		classifier.removeClass(classId);

		auto claVec = classLayers.find(classId);
		claVec->second->isSystem = false;
		classLayers.erase(classId);
	}

	void removeLayer(int coreId)
	{
		ILayer* lay = layers.at(coreId);
		if (!lay->isSystem)
		{
			layers.remove(coreId);
		}
	}

	size_t addTrainData(int layerId, int classId, CachedObjectId srcItemId, BackImage* destIcon)
	{
		auto inLayer = getInTRaster<RasterLineLayer>(layerId);
		assert(inLayer);
		IRasterLayer* sourceLayer = getInRaster(inLayer->parentlayerId);
		sourceLayer->setSubImage(0);

		ItemHolderCache cached;
		cached.openRead(inLayer->getCacheFilePath(getMeta()));

		CurItemHolder item; // Caching item is raw, nit processed
		cached.loadSpecific(srcItemId.tileId, &item);

		BackImage* fromSourceImg = nullptr;
		assert(srcItemId.vecId < item.getItemsCount());
		auto line = item.getItem(srcItemId.vecId);
		if (destIcon != nullptr && sourceLayer)
		{
			auto rect = bc::getBarRect(line->getMatrix());
			auto tileProv = inLayer->prov.tileByIndex(srcItemId.tileId);
			BackPixelPoint st = tileProv.tileToFull(rect.x, rect.y);
			BackPixelPoint ed = tileProv.tileToFull(rect.right(), rect.botton());

			float realFactor = inLayer->subToRealFactor / inLayer->prov.displayFactor; // sub -> display -> real
			printf("Get rect (%d, %d) (%d, %d) with factor %f\n", st.x, st.y, ed.x + st.x, ed.y + st.y, realFactor);
			ed = (ed - st) * realFactor;
			st = st * realFactor;
			printf("Get rect (%d, %d) (%d, %d)\n", st.x, st.y, ed.x + st.x, ed.y + st.y);
			*destIcon = sourceLayer->getRect(st.x, st.y, ed.x, ed.y);
			fromSourceImg = destIcon;
		}
		//rb->barlines[srcItemId.vecId] = nullptr;

		assert(proj->classCategs.size() != 0);
		return classifier.addData(classId, line, fromSourceImg);
	}

	void removeTrainData(int classId, int localId)
	{
		classifier.removeData(classId, localId);
	}

	RetLayers exeFilter(InOutLayer& iol, bc::ProcType type, int algNum)
	{
		//if (u_displayFactor < 1.0)
		//	throw std::exception();
		IRasterLayer* input = getInRaster(iol);

		switch (algNum)
		{
		case 0:
			break;
		case 1:
			return exeQuadro(input, type);
		//case 2:
			//return exeEnergy(input, type, 100);
		case 3:
			exe3d(input, type);
			return {};
		}

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

		BackImage& imgout = layer->mat = src;

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

	class TreeWalk
	{
	public:
		TreeVectorLayer* layer;
		VectorLayer* layerRect;
		RasterLayer* rasterSpot;
		IItemFilter* filter;

		VecTree addTree(bc::barline* line)
		{
			VecTree guiTree;

			BettylineClass cls(line);
			if (filter != nullptr && !filter->pass(&cls))
				return guiTree;

			BackColor pointCol = BackColor::random();
			Barscalar barclo(pointCol.r, pointCol.g, pointCol.b);
			BackImage& imgout = rasterSpot->mat;
			for (auto& p : line->matr)
			{
				imgout.set(p.x, p.y, barclo);
			}

			const int childrenSize = line->children.size();
			for (int i = 0; i < childrenSize; i++)
			{
				VecTree t = addTree(line->children[i]);
				const bool passed = t.inited();
				if (passed)
				{
					guiTree.children.push_back(t);
					//printf("%d\n", guiTree.children.size());
				}
			}

			/*if (guiTree.children.size() < 2)
			{
				return VecTree();
			}*/


			// Barscalar pointCol = RasterLineLayer::colors[rand() % RasterLineLayer::colors.size()];
			//line->getChildsMatr(line->matr, true);
			//line->getChildsMatr(line->matr, true);

			if (line->matr.size() == 0)
				return guiTree;

			std::vector<uint> out;
			auto rect = getCountourOder(line->matr, out, true);
			if (out.size() == 0)
			{
				//return guiTree;
				//for (const auto& pm : line->matr)
				//{
				//	out.push_back(bc::barvalue::getStatInd(pm.getX(), pm.getY()));
				//}
			}


			DrawPrimitive* p = layer->addPrimitive(pointCol);
			guiTree.primId = p->id;
			guiTree.size = line->matr.size();
			for (const auto& pm : out)
			{
				auto op = bc::barvalue::getStatPoint(pm);
				//BackPoint iglob(static_cast<float>(op.x) + 0.5f, static_cast<float>(op.y) + 0.5f);
				BackPoint iglob(static_cast<float>(op.x), static_cast<float>(op.y));

				iglob = layer->cs.toGlobal((float)iglob.x, (float)iglob.y); // To real
				p->addPoint(iglob);
			}

			p = layerRect->addPrimitive(pointCol);
			p->addPoint(rect.topLeft);
			p->addPoint(rect.topRight());
			p->addPoint(rect.getBottomRight());
			p->addPoint(rect.bottomLeft());


			if (guiTree.children.size() < 2)
			{
				//guiTree.size = -1;
			}

			return guiTree;
		}

	};


	RetLayers exeGUI(InOutLayer& iol, const BarcodeProperies& propertices, IItemFilter* filter)
	{
		//if (u_displayFactor < 1.0)
		//	throw std::exception();
		IRasterLayer* input = getInRaster(iol);

		RetLayers ret;
		TreeVectorLayer* layer = addOrUpdateOut<TreeVectorLayer>(iol, input->cs.getProjId());
		layer->color = BackColor::random();
		layer->vecType = VectorLayer::VecType::polygons;
		layer->initCSFrom(input->cs);
		layer->name = "Source bounds";

		auto* layerRect = addLayerData<VectorLayer>(input->cs.getProjId());
		layerRect->color = BackColor::random();
		layerRect->vecType = VectorLayer::VecType::polygons;
		layerRect->initCSFrom(input->cs);
		layerRect->name = "Rect bounds";

		RasterLayer* rasterSpot = addLayerData<RasterLayer>(input->cs.getProjId());
		rasterSpot->initCSFrom(input->cs);
		rasterSpot->init(input);

		//ret.push_back(rasterSpot);
		ret.push_back(layer);
		//ret.push_back(layerRect);
		// layer->init(input);

		BackImage src = *(input->getCachedImage());

		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = true;
		constr.attachMode = bc::AttachMode::morePointsEatLow;
		constr.returnType = bc::ReturnType::barcode2d;
		// constr.maxRadius = 10;
		//constr.maxLen.set(15);
		constr.structure.push_back(propertices.barstruct);

		 bc::BarcodeCreator bc;
		 auto containner = bc.createBarcode(&src, constr);
		 std::unique_ptr<bc::Baritem> item(containner->exractItem(0));
		 delete containner;

		//std::unique_ptr<bc::Baritem> item(bc::Eater::createBarcode(&src));

		rasterSpot->mat = src;
		if (filter)
			filter->imgLen = src.length();

		RasterLineLayer(); // init colors

		auto* root = item->getRootNode();

		TreeWalk helper;
		helper.layer = layer;
		helper.layerRect = layerRect;
		helper.rasterSpot = rasterSpot;
		helper.filter = filter;
		assert(root->children.size() == 1);
		layer->tree = helper.addTree(root->children[0]);

		return ret;
	}


	void ResizeAspect(BackSize& size, const BackSize maxSize)
	{
		// Calculate the aspect ratio of the image
		double aspect_ratio = static_cast<float>(size.wid) / size.hei;
		double max_aspect_width = static_cast<float>(maxSize.hei) * aspect_ratio;
		double max_aspect_height = static_cast<float>(maxSize.wid) / aspect_ratio;
		size.wid = (int)MIN(maxSize.wid, max_aspect_width);
		size.hei = (int)MIN(maxSize.hei, max_aspect_height);
	}

	static void getMod(BackPixelPoint& start, BackPixelPoint& end, BackPixelPoint p, BackSize size, float aspectX, float aspectY)
	{
		const float xa = static_cast<float>(p.x) * aspectX;
		start.x = (int)round(xa);
		end.x = (int)round(mmmin<float>(xa + aspectX + 1, size.wid) - xa);

		const float ya = static_cast<float>(p.y) * aspectY;
		start.y = (int)round(ya);
		end.y = (int)mmmin<int>(round(ya + aspectY) + 1, size.hei);
	}

	RetLayers exeQuadro(IRasterLayer* input, bc::ProcType type)
	{
		RetLayers ret;
		const BackImage& src = *(input->getCachedImage());

		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = false;
		constr.returnType = bc::ReturnType::barcode2d;


		bc::BarcodeCreator bcc;

		const BackSize srcsize(src.width(), src.height());
		//BackSize b = srcsize;
		int maskMin = 0;

		BackSize b(4, 4);
		BackSize imgSize(srcsize.wid, srcsize.hei);
		ResizeAspect(imgSize, b);

		BackImage mask(imgSize.wid, imgSize.hei, 4);
		mask.reintAsInt();
		mask.fill(0);



		bc::barstruct bst(type, bc::ColorType::native, bc::ComponentType::Component);
		bst.maskId = 0;
		bst.mask = &mask;
		constr.structure.push_back(bst);


		while (true)
		{
			BackImage imgin = src;
			imgin.resize(imgSize.wid, imgSize.hei);


			RasterLayer* rasterSpot = addLayerData<RasterLayer>(input->cs.getProjId());
			rasterSpot->initCSFrom(input->cs);
			rasterSpot->init(srcsize.wid, srcsize.hei, 3);
			ret.push_back(rasterSpot);
			BackImage& out = rasterSpot->mat;
			const float aspectX = static_cast<float>(srcsize.wid) / imgSize.wid;
			const float aspectY = static_cast<float>(srcsize.hei) / imgSize.hei;


			//constr.
			//if (constr.structure.size() == 0)
			//	break;
			assert(imgin.length() == mask.length());
			std::unique_ptr<bc::Barcontainer> containner(bcc.createBarcode(&imgin, constr));

			constr.structure.clear();
			maskMin = 0;

			b.wid *= 4;
			b.hei *= 4;
			imgSize = BackSize(srcsize.wid, srcsize.hei);

			if (b.wid > src.width() || b.hei > src.height())
			{
				b.wid = src.width();
				b.hei = src.height();
			}
			else
			{
				ResizeAspect(imgSize, b);
			}

			mask.resize(imgSize.wid, imgSize.hei);
			mask.fill(0);
			const float maskAspectX = static_cast<float>(mask.width()) / imgin.width();
			const float maskAspectY = static_cast<float>(mask.height()) / imgin.height();


			for (size_t ci = 0; ci < containner->count(); ci++)
			{
				bc::Baritem* item = containner->getItem(0);

				for (size_t i = 0; i < item->barlines.size(); ++i)
				{
					const auto& matr = item->barlines[i]->matr;

					//if (matr.size() < 5)
					//	continue;

					bst.maskId = i;
					constr.structure.push_back(bst);

					// if (item->barlines[i]->len() < 10)
					const auto randCol = BackColor::random();
					const Barscalar rcol(randCol.r, randCol.g, randCol.b);
					const Barscalar rid(i, BarType::INT32_1);

					// BackSize maskSize = b;
					for (const auto& pm : matr)
					{
						BackPixelPoint pix(pm.getX(), pm.getY());
						BackPixelPoint start, end;
						getMod(start, end, pix, srcsize, aspectX, aspectY);
						for (size_t l = start.y; l < end.y; ++l)
						{
							// end.x is a length
							out.setRow(start.x, l, end.x, rcol);
						}

						getMod(start, end, pix, imgSize, maskAspectX, maskAspectY);

						for (size_t l = start.y; l < end.y; ++l)
						{
							mask.setRow(start.x, l, end.x, rid);
						}
					}
				}
			}


			//if (b.wid < 8 && b.hei < 6)
			//	break;
			if (b.wid == src.width() && b.hei == src.height())
				break;

			////b.wid /= 2;
			////b.hei /= 2;
			//b.wid *= 2;
			//b.hei *= 2;
			//if (b.wid > src.width() || b.hei > src.height())
			//{
			//	b.wid = src.width();
			//	b.hei = src.height();
			//}
		}

		//std::reverse(ret.begin(), ret.end());
		return ret;
	}


	// Linear interpolation function
	Barscalar lerp(double t)
	{
		if (t < 0)
			t = 0;

		assert(t <= 1.0 && t >= 0);
		//t = log(1 + t);
		const BackColor start(0, 255, 255);
		const BackColor end(255, 0, 0);
		uint8_t r = static_cast<uint8_t>(start.r + t * (end.r - start.r));
		uint8_t g = static_cast<uint8_t>(start.g + t * (end.g - start.g));
		uint8_t b = static_cast<uint8_t>(start.b + t * (end.b - start.b));
		return Barscalar(r, g, b);
	}

	RetLayers exeEnergy(InOutLayer& iol, bc::ProcType type, float energyStart, bool useCells = false)
	{
		IRasterLayer* input = getInRaster(iol);

		RetLayers ret;
		const BackImage& srcl = *(input->getCachedImage());
		const BackSize srcsize(srcl.width(), srcl.height());
		float aspect = 1.f;

		BackImage src(srcsize.wid, srcsize.hei, input->getRect(0, 0, 1, 1).get(0, 0).type);
		if (input->realWidth() != srcsize.wid)
		{
			aspect = static_cast<float>(input->realWidth()) / srcsize.wid;
			for (int h = 0; h < srcsize.hei; ++h)
			{
				for (int i = 0; i < srcsize.wid; ++i)
				{
					src.set(i, h, input->getRect(i * aspect, h * aspect, 1, 1).get(0, 0));
				}
			}
		}
		else
			src = srcl;

		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = false;
		constr.attachMode = bc::AttachMode::morePointsEatLow;
		//constr.attachMode = bc::AttachMode::closer;
		constr.returnType = bc::ReturnType::barcode2d;
		constr.energyStart = energyStart;

		bc::BarcodeCreator bcc;

		//BackSize b = srcsize;
		int maskMin = 0;


		bc::barstruct bst(type, bc::ColorType::native, bc::ComponentType::Component);
		constr.structure.push_back(bst);

		RasterLayer* rasterSpot = addLayerData<RasterLayer>(input->cs.getProjId());
		rasterSpot->initCSFrom(input->cs);
		rasterSpot->aspect = aspect;
		rasterSpot->init(srcsize.wid, srcsize.hei, 3);
		ret.push_back(rasterSpot);

		//bool useEmbeded = false;

		BackImage& out = rasterSpot->mat;
		if (useCells)
		{
		/*	bcc.maxe = energyStart;
			std::unique_ptr<bc::Barcontainer> containner(bcc.createBarcode(&src, constr));

			for (size_t i = 0; i < src.length(); i++)
			{
				out.setLiner(i, lerp(static_cast<float>(bcc.energy[i]) / static_cast<float>(bcc.maxe)));
			}*/

			CellBarcode bce;
			std::unique_ptr<bc::Baritem> containner(bce.run(&src, bst, energyStart));

			bc::Baritem* item = containner.get();
			for (size_t i = 0; i < item->barlines.size(); ++i)
			{
				const auto& matr = item->barlines[i]->matr;

				// BackSize maskSize = b;
				for (const auto& pm : matr)
				{
					out.set(pm.getX(), pm.getY(), lerp(pm.value.getAvgFloat()));
					//out.set(pm.getX(), pm.getY(), pm.value.getAvgUchar());
				}
			}
		}
		else
		{
			EnetrgyBarcode eb;
			float* outenergy = eb.run(&src, bst, energyStart);
			for (size_t i = 0; i < src.length(); i++)
			{
				out.setLiner(i, lerp(outenergy[i] / static_cast<float>(energyStart)));
			}
			delete[] outenergy;

		}
		return ret;
	}


	void exe3d(IRasterLayer* input, bc::ProcType type)
	{
		const BackImage& src = *(input->getCachedImage());

		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = false;
		constr.returnType = bc::ReturnType::barcode3d;

		bc::BarcodeCreator bcc;

		bc::barstruct bst(type, bc::ColorType::native, bc::ComponentType::Component);
		constr.structure.push_back(bst);

		std::unique_ptr<bc::Barcontainer> containner(bcc.createBarcode(&src, constr));
		bc::Baritem* item = containner->getItem(0);

		std::string globout = "";
		for (size_t i = 0; i < item->barlines.size(); ++i)
		{
			auto& counter = *item->barlines[i]->bar3d;
			std::string outstr = "";
			for (const auto& pm : counter)
			{
				outstr += std::to_string(pm.cx);
				outstr += " ";
				outstr += std::to_string(pm.cy);
				outstr += " ";
				outstr += std::to_string(pm.rat);
				outstr += "|";
			}
			globout += outstr + "\r\n";
		}
		WriteFile("D:\\12.txt", globout);
	}
};


bc::barlinevector geojson[3];

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
