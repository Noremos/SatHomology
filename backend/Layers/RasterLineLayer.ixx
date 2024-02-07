module;
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <mutex>

#ifdef __linux__
#include <pthread.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

export module RasterLineLayerModule;

import LayersCore;
import RasterLayers;
import MetadataCoreIO;
import IItemModule;
import GeoprocessorModule;
import Settings;
import CachedBarcode;
import ClusterInterface;

export class BarLineWorker
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

	void runAsync()
	{
		thread.reset(new std::jthread([this] {this->runLoop(); }));
	}

	virtual void runSync() = 0;

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



export class RasterLineLayer : public RasterLayer
{
public:
	IClassItemHolder* collectionToPredict = nullptr;
	static std::vector<Barscalar> colors;
	std::vector<SimpleLine*> clickResponser;
	int cacheId = -1;
	int parentlayerId = -1;

	// Sub to display factor
	float subToRealFactor = 1.f;

	//int layerCounter = 0;


	RasterLineLayer()
	{
		if (colors.size() == 0)
		{
			// srand(300);
			colors.push_back(Barscalar(255, 0, 0));
			colors.push_back(Barscalar(0, 0, 0));
			colors.push_back(Barscalar(0, 255, 0));
			colors.push_back(Barscalar(0, 0, 255));
			for (int k = 0; k < 40; ++k)
				colors.push_back(Barscalar(5 + rand() % 250, 5 + rand() % 250, 5 + rand() % 250));

			colors.push_back(Barscalar(255, 255, 255));
		}
	}


	RetLayers createCacheBarcode(IRasterLayer* inLayer, const BarcodeProperies& propertices, IItemFilter* filter = nullptr);
	RetLayers processCachedBarcode(IItemFilter* filter);

	static const Barscalar& getRandColor(size_t id)
	{
		return colors[id % colors.size()];
	}

	BackPathStr getCacheFilePath(const MetadataProvider& metaFolder)
	{
		MetadataProvider m = metaFolder.getSubMeta(getMetaLayerName());
		return m.getSubFolder("cached.bff");
	}

	virtual void release(const MetadataProvider& metaFolder)
	{
		// RasterLayer::release(metaFolder);
		// if (cacheId != -1)
		// {
		// 	std::remove(getCacheFilePath(metaFolder).string().c_str()); // delete file
		// }
	}

	virtual const LFID getFactoryId() const
	{
		return RASTER_LINE_LAYER_FID;
	}

	virtual void saveLoadState(JsonObjectIOState* state, const MetadataProvider& metaFolder)
	{
		RasterLayer::saveLoadState(state, metaFolder);
		state->scInt("cacheId", cacheId);
		state->scInt("parentLayerId", parentlayerId);
		state->scFloat("subToRealFactor", subToRealFactor); // TODO: rename it to SubToDsiplay
	}

	virtual int realWidth() const override
	{
		return mat.width() * subToRealFactor; // Invalid
	}
	virtual int realHeight() const override
	{
		return mat.height() * subToRealFactor; // Invaild;
	}

	//virtual void readJson(const BackJson& json, const BackDirStr& metaFolder)
	//{
	//	RasterLayer::readJson(json, metaFolder);
	//}

	//virtual void writeJson(BackJson& json, const BackDirStr& metaFolder, int& counter)
	//{
	//	RasterLayer::writeJson(json, metaFolder, counter);

	//	for (int i=0;i < clickResponser.size(); i++)
	//	{
	//		// id, id in bar
	//	}
	//}

	//void init(const BackImage& src, int tileSize = DEF_TILE_SIZE)
	//{
	//	clickResponser.clear();
	//	mat.assignCopyOf(src);
	//	clickResponser.resize(mat.length());
	//	printf("alloced for preview: %d\n", clickResponser.size());
	//	prov.init(src.width(), src.height(), src.width(), tileSize);
	//}

	void init(IRasterLayer* layer, const MetadataProvider& metadata)
	{
		assert(layer != this);

		subToRealFactor = layer->realWidth() / layer->displayWidth(); // SubToDsiplay
		int wid = layer->displayWidth();
		int hei = layer->displayHeight();

		resetResponser(wid, hei);
		parentlayerId = layer->id;
		prov = layer->prov;

		mkDirIfNotExists(metadata.getSubFolder(getMetaLayerName()));
	}

	void clearResponser()
	{
		int wid = mat.width();
		int hei = mat.height();
		assert(mat.length() > 1);
		resetResponser(wid, hei);
	}

	void resetResponser(int wid, int hei)
	{
		mat.reinit(wid, hei, 4);

		clickResponser.clear();
		clickResponser.resize(mat.length());
		printf("alloced for preview: %zd\n", clickResponser.size());
	}
	SimpleLineHolder holder;

	std::mutex addToMapMutex;
	void setMatrPoint(int x, int y, SimpleLine* newLine, const Barscalar& color)
	{
		std::lock_guard<std::mutex> guard(addToMapMutex);

		int indLocal = mat.getLineIndex(x, y);
		SimpleLine* existLine = clickResponser[indLocal];

		if (existLine == nullptr)
		{
			mat.set(x, y, color);
			clickResponser[indLocal] = newLine;
		}
		else if (existLine == newLine) // Might be due to clickResponser[indLocal2] = newLin
		{
			mat.set(x, y, color);
		}
		else if (existLine->getDeath() < newLine->getDeath())
		{
			mat.set(x, y, color);
			clickResponser[indLocal] = newLine;
		}
	}

	// using ColorGrater = std::function<Barscalar(const IClassItem* item, bool& bad)>;

	bool passLine(const IClassItem* item, const IItemFilter* filter) const
	{
		if (item->getMatrixSize() == 0)
			return false;

		if (filter && !filter->pass(item))
			return false;

		return true;
	}

	void addLine(int i, const IClassItem* curLine, const TileProvider& tileProv)
	{
		SimpleLine* sl;
		auto curIdKey = curLine->getId();
		//assert(curIdKey == i);
		// TODO: Will not work with tiles. Use tile + curIdKey to get the hash and use map in holder
		//if (holder.holder.size() <= curIdKey)
		//	holder.holder.resize(curIdKey + 1);

		sl = holder.holder[curIdKey].get();
		if (sl)
		{
			sl->tileId = tileProv.index;
			sl->barlineIndex = i;
		}
		else
		{
			sl = new SimpleLine(tileProv.index, i);
			sl->root = &holder;
			holder.holder[curIdKey].reset(sl);
		}
		
		{
			sl->parent = curLine->getParentId();
			//if (holder.holder.size() <= sl->parent)
			//	holder.holder.resize(static_cast<size_t>(sl->parent) + 1);

			auto par = holder.holder[sl->parent].get();
			if (par == nullptr)
			{
				par = new SimpleLine(-1, -1);
				par->root = &holder;
				holder.holder[sl->parent].reset(par);
			}
			par->children.push_back(curIdKey);
		}

		const auto& matr = curLine->getMatrix();
		int depth = curLine->getDeath();
		sl->depth = depth;
		sl->start = curLine->start();
		sl->end = curLine->end();
		sl->matrSrcSize = (int)matr.size();

		// Add line
		Barscalar pointCol = RasterLineLayer::colors[rand() % RasterLineLayer::colors.size()];

		// Countur
		DictWrap dictPoints;
		for (auto& p : matr)
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

		auto& outMatr = sl->matr;
		for (const auto& pm : matr)
		{
			// We have:
			// - a real img size (input)
			// - a sub img size (selected subimg in input)
			// - a display img (for LineRaster).

			// Get a pixels form a sub
			int x = pm.getX();
			int y = pm.getY();


			// Cast sub point to display (mat variable) via tileProv
			BackPixelPoint op = tileProv.tileToPreview(x, y);
			op.x = (std::min)(mat.wid() - 1, op.x);
			op.y = (std::min)(mat.hei() - 1, op.y);

			// Set display point
			setMatrPoint(op.x, op.y, sl, pointCol);

			// Get countur
			if (dictPoints.hasCorners(x, y)) // Skip
			{
				continue;
			}

			// Cast to a real img via factor and save for draw
			outMatr.push_back(bc::barvalue(op.x * subToRealFactor, op.y * subToRealFactor, pm.value));
		}
	}

	void addHolder(const IClassItemHolder& items, const TileProvider& tileProv, const IItemFilter* filter)
	{
		for (size_t i = 0; i < items.getItemsCount(); ++i)
		{
			auto curLine = items.getItem(i);
			if (!passLine(curLine, filter))
				continue;

			addLine(i, curLine, tileProv);
		}
	}
};

std::vector<Barscalar> RasterLineLayer::colors;


class CreateBarThreadWorker : public BarLineWorker
{
	std::mutex& cacherMutex;

	bc::barstruct constr;
	IRasterLayer* inLayer;
	ItemHolderCache& cacher;

	IClassItemHolder::ItemCallback cacheClass;
	int inde;

	BackImage rect;
	TileProvider tileProv;
public:
	CreateBarThreadWorker(std::mutex& cacherMutex,
		const bc::barstruct& constr,
		IRasterLayer* inLayer,
		ItemHolderCache& cacher,
		int& counter) :
		BarLineWorker(counter),
		cacherMutex(cacherMutex),
		constr(constr),
		inLayer(inLayer),
		cacher(cacher),
		tileProv(0, 0)
	{
	}
	int algMode;

	void setCallback(/*Project* proj, */RasterLineLayer* layer, const IItemFilter* filter)
	{
		cacheClass = [this, /*proj,*/ layer, filter](IClassItem* item)
			{
				if (layer->passLine(item, filter))
				{
					//proj->predictForLayer(item, tileProv, layer->subToRealFactor);
					layer->addLine(inde++, item, tileProv);
				}
			};
	}

	void updateTask(BackImage& rrect, const TileProvider& tileProvider)
	{
		this->rect = std::move(rrect);
		this->tileProv = tileProvider;
	}


	void runSync()
	{
		const auto start = std::chrono::steady_clock::now();

		// Keep this! See lyambda
		inde = 0;
		parentne.clear();
		// -------------

		printf("Start run for tile %d\n", tileProv.index);

		CachedBaritemHolder creator;
		creator.create(&rect, constr, cacheClass);

		{
			std::lock_guard<std::mutex> guard(cacherMutex);
			cacher.save(&creator, tileProv.index);
		}


		const auto end = std::chrono::steady_clock::now();
		const auto diff = end - start;
		const double len = std::chrono::duration<double, std::milli>(diff).count();
		printf("End run for tile %d; Time: %dms\n", tileProv.index, (int)len);

		++counter;
		busy = false;
	}
};


class ProcessCacheBarThreadWorker : public BarLineWorker
{
	RasterLineLayer* outLayer;
	const IItemFilter* filter;
	//Project* proj;

	TileProvider tileProv;

public:
	CachedBaritemHolder holder;

	ProcessCacheBarThreadWorker(int& counter, RasterLineLayer* outLayer, const IItemFilter* filter/*, Project* proj*/) :
		BarLineWorker(counter), outLayer(outLayer), filter(filter),// proj(proj),
		tileProv(0, 0)
	{ }

	void updateTask(const TileProvider& itileProv)
	{
		tileProv = itileProv;
	}
	bool addLine = true;
	void runSync()
	{
		const auto start = std::chrono::steady_clock::now();

		printf("Start run for tile %d\n", tileProv.index);
		//outLayer->addHolder(*holder, tileProv, filter);
		if (outLayer->collectionToPredict == nullptr)
		{
			for (size_t i = 0; i < holder.getItemsCount(); ++i)
			{
				auto* item = holder.getRItem(i);
				if (outLayer->passLine(item, filter))
				{
					outLayer->addLine((int)i, item, tileProv);
				}
			}
		}
		else
		{
			for (size_t i = 0; i < holder.getItemsCount(); ++i)
			{
				auto* item = holder.getRItem(i);
				if (outLayer->passLine(item, filter))
				{
					outLayer->collectionToPredict->addItem(*item);
				}
			}
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = end - start;
		const double len = std::chrono::duration<double, std::milli>(diff).count();
		printf("End run for tile %d; Time: %dms\n", tileProv.index, (int)len);

		++counter;
		busy = false;
	}

};



RetLayers RasterLineLayer::createCacheBarcode(IRasterLayer* inLayer, const BarcodeProperies& propertices, IItemFilter* filter)
{
	// Setup
	bc::barstruct constr;
	constr.createBinaryMasks = true;
	constr.createGraph = true;
	constr.attachMode = propertices.attachMode;
	//constr.maxRadius = 255;
	// constr.attachMode = bc::AttachMode::closer;
	constr.returnType = bc::ReturnType::barcode2d;
	//constr.addStructure(propertices.barstruct);
	//	constr.setStep(stepSB);
	// -------

	constr = propertices.get();

	// Input Layer prepatons
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
	auto& metaprov = getSettings().getMeta();
	RasterLineLayer* layer = this;
	layer->init(inLayer, getSettings().getMeta());
	layer->initCSFrom(inLayer->cs);
	layer->tileOffset = tileOffset;

	LayerProvider& prov = layer->prov;
	prov.init(curSize.wid, curSize.hei, inLayer->displayWidth(), tileSize);

	if (layer->cacheId == -1)
		layer->cacheId = metaprov.getUniqueId();

	RetLayers ret;
	ret.push_back(layer);
	//
	// Classes layers
	//for (auto& i : classLayers)
	//{
	//	ret.push_back(i.second);
	//	i.second->clear();
	//	i.second->color = classCategs.get(i.first)->color;
	//	i.second->initCSFrom(inLayer->cs);
	//}
	// -------------------

	// Cacher
	ItemHolderCache cacher;
	cacher.openWrite(layer->getCacheFilePath(metaprov));
	// ------


	// Setup tileIterators
	TileImgIterator tileIter(tileSize, tileOffset, curSize.wid, curSize.hei);

	// Threads
	std::mutex cacherMutex;

	const bool curRunAsync = getSettings().runAsync;
	int curthreadsCount = curRunAsync ? getSettings().threadsCount : 1;
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
		worker->setCallback(layer, filter);
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
	const double len = std::chrono::duration<double, std::milli>(diff).count();
	printf("All work ended in %dms\n", (int)len);

	return ret;
}

RetLayers RasterLineLayer::processCachedBarcode(IItemFilter* filter)
{
	//if (u_displayFactor < 1.0)
	//	throw std::exception();

	RasterLineLayer* inLayer = this;
	int tileSize = inLayer->prov.tileSize;
	int tileOffset = inLayer->tileOffset;

	if (filter)
	{
		const uint fullTile = tileSize + tileOffset;
		filter->imgLen = fullTile * fullTile;
	}

	// -------
	RetLayers ret;
	//for (auto& i : classLayers)
	//{
	//	ret.push_back(i.second);
	//	i.second->clear();
	//	i.second->color = classCategs.get(i.first)->color;
	//	i.second->initCSFrom(inLayer->cs);
	//}

	// RasterLineLayer* outLayer = addOrUpdateOut<RasterLineLayer>(iol);
	// if (outLayer->cacheId == -1)
	// 	outLayer->cacheId = metaprov->getUniqueId();
	// ret.push_back(outLayer);
	RasterLineLayer* outLayer = inLayer;

	if (collectionToPredict == nullptr)
		outLayer->clearResponser();

	//ret.push_back(outLayer); // Do not recreate this node

	// Cacher
	ItemHolderCache cacher;
	cacher.openRead(inLayer->getCacheFilePath(*getSettings().metaprov));


	// Thread
	auto& prov = inLayer->prov;

	const bool curRunAsync = getSettings().runAsync;
	int curthreadsCount = 1;
	int counter = 0;
	if (curRunAsync)
	{
		curthreadsCount = getSettings().threadsCount;
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
		ProcessCacheBarThreadWorker* worker = new ProcessCacheBarThreadWorker(counter, outLayer, filter);
		//worker->addLine = addLineMode;
		wpool.add(worker, allowSyncInAsync);
	}

	const auto start = std::chrono::steady_clock::now();
	if (curRunAsync)
	{
		int tileIndex = 0;
		while (cacher.canRead())
		{
			bool isAsync;
			auto* worker = wpool.getFreeWorker(isAsync);

			cacher.load(tileIndex, &worker->holder);

			TileProvider tileProv = prov.tileByIndex(tileIndex);

			worker->updateTask(tileProv);

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
			ProcessCacheBarThreadWorker* worker = wpool.getSyncWorker();
			cacher.load(tileIndex, &worker->holder);

			TileProvider tileProv = inLayer->prov.tileByIndex(tileIndex);

			worker->updateTask(tileProv);
			worker->runSync();
		}
	}

	const auto end = std::chrono::steady_clock::now();
	const auto diff = end - start;
	const double len = std::chrono::duration<double, std::milli>(diff).count();
	printf("All work ended in %dms\n", (int)len);

	return ret;
}
