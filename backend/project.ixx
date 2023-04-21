module;

#include <unordered_set>
#include <random>
#include <assert.h>
#include <chrono>
#include <thread>
//#include <iostream>

#include "../Bind/Common.h"

#ifdef __linux__
#include <pthread.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif


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


// using CurItemHolder = BaritemHolder;
using CurItemHolder = BettyItemHolder;

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
		{"threadsCount", threadsCount},
		{"runAsync", runAsync},
		{"layerCounter", layerCounter}


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
			MetadataProvider sub = *metaprov;// Do not reffer!

			int factoryId;
			ILayer* lay;
			if (isReading)
			{
				obj->scInt("factoryId", factoryId);
				lay = CoreLayerFactory::CreateCoreLayer(factoryId);
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

			int clId;
			int coreId;
			if (!isReading)
			{
				clId = cb->first;
				coreId = cb->second->id;
				++cb;
			}
			obj->scInt("class_id", clId);
			obj->scInt("layer_id", coreId);

			if (isReading)
			{
				auto* vectLayer = layers.at(coreId);
				if (vectLayer)
					classLayers[clId] = static_cast<VectorLayer*>(vectLayer);
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
		ds.csScale = 1.0;//BackPoint(1000, 1000);

		threadsCount = std::thread::hardware_concurrency();

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
	int threadsCount = 4;
	bool runAsync = true;

	BarCategories classCategs;
	BettyClassificator classifier;

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

		metaprov.reset(new MetadataProvider(path, metaCounter));
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
		settings.read(loadDoc);
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
		settings.write(sets);
		jsonToFile(sets, getPath(BackPath::project));

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

	class BarLineWorker
	{
	protected:
		std::unique_ptr<std::jthread> thread;
		int& counter;
		IdGrater parentne;


		bool stopThreadFlag = false;
		bool taskUpdated = false;
		std::atomic<bool> busy = false;

	public:
		BarLineWorker(int& counter) : counter(counter)
		{ }

		void runTask()
		{
			busy = true;
#ifdef _WIN32
			SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
#elif __linux__
			sched_param params;
			params.sched_priority = 75;
			pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
#endif
			taskUpdated = true;
		}

		void join()
		{
			if (thread && busy)
				thread->join();
		}

		bool isBusy() const
		{
			return busy;
		}

		void stop()
		{
			stopThreadFlag = true;
		}
	};


	class CreateBarThreadWorker : public BarLineWorker
	{
		std::mutex &cacherMutex;

		const bc::BarConstructor& constr;
		IRasterLayer* inLayer;
		ItemHolderCache& cacher;

		IClassItemHolder::ItemCallback cacheClass;
		int inde;

		BackImage rect;
		TileProvider tileProv;
	public:
		CreateBarThreadWorker(std::mutex& cacherMutex,
							const bc::BarConstructor& constr,
							IRasterLayer* inLayer,
							ItemHolderCache& cacher,
							int& counter)
			: BarLineWorker(counter),
			cacherMutex(cacherMutex),
			constr(constr),
			inLayer(inLayer),
			cacher(cacher),
			tileProv(0,0)
		{
		}

		void setCallback(Project* proj, RasterLineLayer* layer, const IItemFilter* filter)
		{
			cacheClass = [this, proj, layer, filter](IClassItem* item)
			{
				if (layer->passLine(item, filter))
				{
					proj->predictForLayer(item, tileProv, layer->subToRealFactor);
					layer->addLine(parentne, inde++, item, tileProv);
				}
			};
		}

		void updateTask(BackImage& rrect, const TileProvider& tileProvider)
		{
			this->rect = std::move(rrect);
			this->tileProv = tileProvider;
		}


		void runAsync()
		{
			thread.reset(new std::jthread([this]{this->runLoop();}));
		}

		void runSync()
		{
			const auto start = std::chrono::steady_clock::now();

			// Keep this! See lyambda
			inde = 0;
			parentne.clear();
			// -------------

			printf("Start run for tile %d\n", tileProv.index);
			CurItemHolder creator;
			creator.create(&rect, constr, cacheClass);

			{
				std::lock_guard<std::mutex> guard(cacherMutex);
				cacher.save(&creator, tileProv.index);
			}


			const auto end = std::chrono::steady_clock::now();
			const auto diff = end - start;
			const double len = std::chrono::duration<double, std::milli> (diff).count();
			printf("End run for tile %d; Time: %dms\n", tileProv.index, (int)len);

			++counter;
			busy = false;
		}

		void runLoop()
		{
			while (!stopThreadFlag)
			{
				if (!taskUpdated)
					continue;

				taskUpdated = false;
				runSync();

#ifdef _WIN32
				SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#elif __linux__
				sched_param params;
				params.sched_priority = 20;
				pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
#endif
			}
		}
	};

	class TileImgIterator
	{
		TileIterator stW, stH;

		uint lastHei;
		bool peakEnd = false;

	public:
		TileImgIterator(int tileSize, int tileOffset, int rwid, int rhei) :
			stW(0, tileSize, tileOffset, rwid),
			stH(0, tileSize, tileOffset, rhei)
		{
			lastHei = 0;
			stH.shouldSkip(lastHei);
		}

		void reset()
		{
			stW.reset(0);
			stH.reset(0);

			lastHei = 0;
			stH.shouldSkip(lastHei);

			peakEnd = false;
		}

		bool iter(bc::point& offset, uint& wid, uint& hei)
		{
			while (stW.shouldSkip(wid))
			{
				stW.reset(0);
				stH.accum();
				if (stH.shouldSkip(lastHei))
				{
					return false;
				}
			}

			offset.x = stW.pos();
			offset.y = stH.pos();
			hei = lastHei;

			stW.accum();
			return true;
		}

		// get 2x2 local index
		int getLocRectIndex() const
		{
			return 2 * (stH.locIndex % 2) + (stW.locIndex % 2);
		}

		bool notFintInLocal() const
		{
			return stW.tilesInLine() > 2 || stH.tilesInLine() > 2;
		}
	};

	template<typename TWorker>
	class WorkerPool
	{
		std::vector<std::unique_ptr<TWorker>> workers;
		std::unique_ptr<TWorker> syncWorker;

	public:
		void add(TWorker* worker, bool allowSync)
		{
			if (allowSync && !syncWorker)
			{
				syncWorker.reset(worker);
				return;
			}

			worker->runAsync();
			workers.push_back(std::unique_ptr<TWorker>(worker));
		}

		void addSync(TWorker* worker)
		{
			syncWorker.reset(worker);
		}

		TWorker* getSyncWorker()
		{
			return syncWorker.get();
		}

		TWorker* getFreeWorker(bool& isAsync)
		{
			//while(true)
			{
				for (auto& worker : workers)
				{
					if (!worker->isBusy())
					{
						isAsync = true;
						return worker.get();
					}
				}
			}

			isAsync = false;
			return syncWorker.get();
		}

		void waitForAll(const bool stop)
		{
			using namespace std::chrono_literals;
			bool hasRunning = false;
			do
			{
				hasRunning = false;
				for (auto& worker : workers)
				{
					if (worker->isBusy())
					{
						hasRunning = true;
						std::this_thread::sleep_for(500ms);
						break;
					}
					else if (stop)
					{
						worker->stop();
						worker->join();
					}
				}
			} while (hasRunning);
		}
	};

	RetLayers createCacheBarcode(InOutLayer& iol, const BarcodeProperies& propertices, IItemFilter* filter = nullptr)
	{
		RetLayers ret;
		if (block) return ret;

		// Setup
		bc::BarConstructor constr;
		constr.createBinaryMasks = true;
		constr.createGraph = true;
		constr.attachMode = bc::AttachMode::morePointsEatLow;
		constr.returnType = bc::ReturnType::barcode2d;
		constr.structure.push_back(propertices.barstruct);
		//	constr.setStep(stepSB);
		// -------

		// Input Layer prepatons
		IRasterLayer* inLayer = getInRaster(iol);
		int tileSize = inLayer->prov.tileSize;
		int tileOffset = inLayer->tileOffset;
		SubImgInf curSize = inLayer->getSubImgInf(); // Cursubimg

		if (filter)
		{
			const uint fullTile = tileSize + tileOffset;
			filter->imgLen = fullTile * fullTile;
		}
		// End Input Layer

		// Setup output layers
		//
		// Line layer
		RasterLineLayer* layer = addOrUpdateOut<RasterLineLayer>(iol, inLayer->cs.getProjId());
		layer->init(inLayer, getMeta());
		layer->initCSFrom(inLayer->cs);
		layer->tileOffset = tileOffset;

		LayerProvider& prov = layer->prov;
		prov.init(curSize.wid, curSize.hei, inLayer->displayWidth(), tileSize);

		if (layer->cacheId == -1)
			layer->cacheId = metaprov->getUniqueId();

		ret.push_back(layer);
		//
		// Classes layers
		for (auto& i : classLayers)
		{
			ret.push_back(i.second);
			i.second->clear();
			i.second->color = classCategs.categs[i.first].color;
			i.second->initCSFrom(inLayer->cs);
		}
		// -------------------

		// Cacher
		ItemHolderCache cacher;
		cacher.openWrite(layer->getCacheFilePath(getMeta()));
		// ------


		// Setup tileIterators
		TileImgIterator tileIter(tileSize, tileOffset, curSize.wid, curSize.hei);

		// Threads
		std::mutex cacherMutex;

		const bool curRunAsync = runAsync;
		int curthreadsCount = curRunAsync ? threadsCount : 1;
		int counter = 0;
		if (curRunAsync)
		{
			printf("Run in async mode with %d threads\n", curthreadsCount);
		}
		else
		{
			printf("Run in sync mode\n");
		}

		const bool allowSyncInAsync = true;
		WorkerPool<CreateBarThreadWorker> wpool;
		for (unsigned short i = 0; i < curthreadsCount; i++)
		{
			CreateBarThreadWorker* worker = new CreateBarThreadWorker(cacherMutex, constr, inLayer, cacher, counter);
			worker->setCallback(this, layer, filter);
			wpool.add(worker, allowSyncInAsync);
		}

		// Run
		uint iwid, ihei;
		bc::point offset;

		const auto start = std::chrono::steady_clock::now();
		if (curRunAsync)
		{
			// How much tiles the offset covers; We skip the conflict tiles;
			// const int maxSteps = 1 + static_cast<int>((tileSize + tileOffset) / tileSize);
			tileIter.reset();
			while (tileIter.iter(offset, iwid, ihei))
			{
				TileProvider tileProv = prov.tileByOffset(offset.x, offset.y);
				auto rect = inLayer->getRect(offset.x, offset.y, iwid, ihei);

				bool isAsyncl;
				auto* worker = wpool.getFreeWorker(isAsyncl);
				worker->updateTask(rect, tileProv);

				if (isAsyncl)
					worker->runTask();
				else
					worker->runSync();
			}
			wpool.waitForAll(true);
		}
		else
		{
			while (tileIter.iter(offset, iwid, ihei))
			{
				TileProvider tileProv = prov.tileByOffset(offset.x, offset.y);
				auto rect = inLayer->getRect(offset.x, offset.y, iwid, ihei);

				auto* worker = wpool.getSyncWorker();
				worker->updateTask(rect, tileProv);
				worker->runSync();
			}
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = end - start;
		const double len = std::chrono::duration<double, std::milli> (diff).count();
		printf("All work ended in %dms\n", (int)len);

		u_algorithm = propertices.alg;
		saveProject();

		return ret;
	}


	class ProcessCacheBarThreadWorker : public BarLineWorker
	{
		RasterLineLayer* outLayer;
		const IItemFilter* filter;
		Project* proj;

		TileProvider tileProv;
		CurItemHolder holder;

	public:
		ProcessCacheBarThreadWorker(int& counter, RasterLineLayer* outLayer, const IItemFilter* filter, Project* proj) :
			BarLineWorker(counter), outLayer(outLayer), filter(filter), proj(proj),
			tileProv(0,0)
		{ }

		void runAsync()
		{
			thread.reset(new std::jthread([this] {this->runLoop(); }));
		}


		void updateTask(CurItemHolder& iholder, const TileProvider& itileProv)
		{
			tileProv = itileProv;
			holder = std::move(iholder);
		}

		void runSync()
		{
			const auto start = std::chrono::steady_clock::now();

			// Keep this! See lyambda
			parentne.clear();
			// -------------

			printf("Start run for tile %d\n", tileProv.index);

			for (size_t i = 0; i < holder.getItemsCount(); ++i)
			{
				auto item = holder.getItem(i);
				if (outLayer->passLine(item, filter))
				{
					proj->predictForLayer(item, tileProv, outLayer->subToRealFactor);
					outLayer->addLine(parentne, (int)i, item, tileProv);
				}
			}

			const auto end = std::chrono::steady_clock::now();
			const auto diff = end - start;
			const double len = std::chrono::duration<double, std::milli> (diff).count();
			printf("End run for tile %d; Time: %dms\n", tileProv.index, (int)len);

			++counter;
			busy = false;
		}

		void runLoop()
		{
			while (!stopThreadFlag)
			{
				if (!taskUpdated)
					continue;

				taskUpdated = false;
				runSync();

#ifdef _WIN32
				SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#elif __linux__
				sched_param params;
				params.sched_priority = 20;
				pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
#endif
			}
		}
	};


	RetLayers processCachedBarcode(InOutLayer& iol, IItemFilter* filter)
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
		{
			ret.push_back(i.second);
			i.second->clear();
			i.second->color = classCategs.categs[i.first].color;
			i.second->initCSFrom(inLayer->cs);
		}

		// RasterLineLayer* outLayer = addOrUpdateOut<RasterLineLayer>(iol);
		// if (outLayer->cacheId == -1)
		// 	outLayer->cacheId = metaprov->getUniqueId();
		// ret.push_back(outLayer);
		RasterLineLayer* outLayer = inLayer;
		outLayer->clearResponser();
		ret.push_back(outLayer);

		// Cacher
		ItemHolderCache cacher;
		cacher.openRead(inLayer->getCacheFilePath(getMeta()));


		// Thread
		auto& prov = inLayer->prov;

		const bool curRunAsync = runAsync;
		int curthreadsCount = curRunAsync ? threadsCount : 1;
		int counter = 0;
		if (curRunAsync)
		{
			printf("Run in async mode with %d threads\n", curthreadsCount);
		}
		else
		{
			printf("Run in sync mode\n");
		}

		const bool allowSyncInAsync = true;
		WorkerPool<ProcessCacheBarThreadWorker> wpool;
		for (unsigned short i = 0; i < curthreadsCount; i++)
		{
			ProcessCacheBarThreadWorker* worker = new ProcessCacheBarThreadWorker(counter, outLayer, filter, this);
			wpool.add(worker, allowSyncInAsync);
		}

		const auto start = std::chrono::steady_clock::now();
		if (curRunAsync)
		{
			int tileIndex = 0;
			while (cacher.canRead())
			{
				CurItemHolder holder;
				cacher.load(tileIndex, &holder);

				TileProvider tileProv = prov.tileByIndex(tileIndex);

				bool isAsync;
				auto* worker = wpool.getFreeWorker(isAsync);
				worker->updateTask(holder, tileProv);

				if (isAsync)
					worker->runTask();
				else
					worker->runSync();
			}

			wpool.waitForAll(true);
		}
		else // Sync
		{
			int tileIndex = 0;
			while (cacher.canRead())
			{
				CurItemHolder holder;
				cacher.load(tileIndex, &holder);

				TileProvider tileProv = inLayer->prov.tileByIndex(tileIndex);

				ProcessCacheBarThreadWorker* worker = wpool.getSyncWorker();
				worker->updateTask(holder, tileProv);
				worker->runSync();
			}
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = end - start;
		const double len = std::chrono::duration<double, std::milli> (diff).count();
		printf("All work ended in %dms\n", (int)len);

		return ret;
	}

	std::mutex addPrimitiveMutex;
	bool predictForLayer(IClassItem* item, const TileProvider& tileProv, float diplsayToRealFactor)
	{
		auto id = predict(item);
		if (id != -1)
		{
			VectorLayer* vl = classLayers.at(id);

			mcountor temp;
			getCountour(item->getMatrix(), temp, true);

			DrawPrimitive* p;
			{
				std::lock_guard<std::mutex> guard(addPrimitiveMutex);
				p = vl->addPrimitive(vl->color);
			}

			CSBinding& cs = vl->cs;

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
		int id = classCategs.addValue(name);
		classifier.addClass(id);
		auto* layer = addLayerData<VectorLayer>(ds.sysProj.getId());
		layer->name = "Class: " + name;
		layer->color = BackColor::random();
		layer->vecType = VectorLayer::VecType::polygons;
		layer->isSystem = true;

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

			float realFactor = inLayer->subToRealFactor / inLayer->prov.displayFactor;
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
