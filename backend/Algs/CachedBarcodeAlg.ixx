//#include <mutex>
//module;
////export module CachedBarcodeAlg;
//
//import IAlgoriphm;
//import RasterBarHolderRLayer;
//
//export class BarLineWorker : public IWorker
//{
//protected:
//	IdGrater parentne;
//
//
//public:
//	BarLineWorker(int& counter) : counter(counter)
//	{ }
//};
//
//
//class CreateBarThreadWorker : public BarLineWorker
//{
//	std::mutex& cacherMutex;
//
//	const bc::BarConstructor& constr;
//	IRasterLayer* inLayer;
//	ItemHolderCache& cacher;
//
//	IClassItemHolder::ItemCallback cacheClass;
//	int inde;
//
//	BackImage rect;
//	TileProvider tileProv;
//public:
//	CreateBarThreadWorker(std::mutex& cacherMutex,
//		const bc::BarConstructor& constr,
//		IRasterLayer* inLayer,
//		ItemHolderCache& cacher,
//		int& counter) :
//		BarLineWorker(counter),
//		cacherMutex(cacherMutex),
//		constr(constr),
//		inLayer(inLayer),
//		cacher(cacher),
//		tileProv(0, 0)
//	{
//	}
//
//	void setCallback(/*Project* proj, */RasterLineLayer* layer, const IItemFilter* filter)
//	{
//		cacheClass = [this, /*proj,*/ layer, filter](IClassItem* item)
//			{
//				if (layer->passLine(item, filter))
//				{
//					//proj->predictForLayer(item, tileProv, layer->subToRealFactor);
//					layer->addLine(parentne, inde++, item, tileProv);
//				}
//			};
//	}
//
//	void updateTask(BackImage& rrect, const TileProvider& tileProvider) override
//	{
//		this->rect = std::move(rrect);
//		this->tileProv = tileProvider;
//	}
//
//
//
//
//	void runSync() override
//	{
//		const auto start = std::chrono::steady_clock::now();
//
//		// Keep this! See lyambda
//		inde = 0;
//		parentne.clear();
//		// -------------
//
//		printf("Start run for tile %d\n", tileProv.index);
//		CachedBaritemHolder creator;
//		creator.create(&rect, constr, cacheClass);
//
//		{
//			std::lock_guard<std::mutex> guard(cacherMutex);
//			cacher.save(&creator, tileProv.index);
//		}
//
//
//		const auto end = std::chrono::steady_clock::now();
//		const auto diff = end - start;
//		const double len = std::chrono::duration<double, std::milli>(diff).count();
//		printf("End run for tile %d; Time: %dms\n", tileProv.index, (int)len);
//
//		++counter;
//		busy = false;
//	}
//};
//
//
//class ProcessCacheBarThreadWorker : public BarLineWorker
//{
//	RasterLineLayer* outLayer;
//	const IItemFilter* filter;
//	//Project* proj;
//
//	TileProvider tileProv;
//
//public:
//	CachedBaritemHolder holder;
//
//	ProcessCacheBarThreadWorker(int& counter, RasterLineLayer* outLayer, const IItemFilter* filter/*, Project* proj*/) :
//		BarLineWorker(counter), outLayer(outLayer), filter(filter),// proj(proj),
//		tileProv(0, 0)
//	{ }
//
//
//
//	void updateTask(BackImage&, const TileProvider& itileProv) override
//	{
//		tileProv = itileProv;
//	}
//	bool addLine = true;
//	void runSync() override
//	{
//		const auto start = std::chrono::steady_clock::now();
//
//		// Keep this! See lyambda
//		parentne.clear();
//		// -------------
//
//		printf("Start run for tile %d\n", tileProv.index);
//		//outLayer->addHolder(*holder, tileProv, filter);
//
//		for (size_t i = 0; i < holder.getItemsCount(); ++i)
//		{
//			auto* item = holder.getRItem(i);
//			if (outLayer->passLine(item, filter))
//			{
//				if (addLine)
//				{
//					outLayer->addLine(parentne, (int)i, item, tileProv);
//				}
//				else
//				{
//					if (outLayer->collectionToPredict)
//						outLayer->collectionToPredict->addItem(*item);
//					//proj->predictForLayer(item, tileProv, outLayer->subToRealFactor);
//				}
//			}
//		}
//
//		const auto end = std::chrono::steady_clock::now();
//		const auto diff = end - start;
//		const double len = std::chrono::duration<double, std::milli>(diff).count();
//		printf("End run for tile %d; Time: %dms\n", tileProv.index, (int)len);
//
//		++counter;
//		busy = false;
//	}
//
//};
