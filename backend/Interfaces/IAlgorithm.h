// module;
#pragma once
#define M_EXPORT
#include <memory>
#include <unordered_map>

// #include <thread>
// #ifdef __linux__
// #include <pthread.h>
// #endif

// #ifdef _WIN32
// #include <windows.h>
// #endif
#include <cassert>
#include <vector>
#include <functional>
#include "Common.h"
#include "../MLSettings.h"
#include "../Interfaces/IItem.h"
#include "../Layers/layerInterface.h"
#include "../Layers/Rasterlayers.h"

// M_EXPORT module IAlgorithm;

// import RasterLayers;
//import BackBind;
// import LayersCore;
// import IItemModule;
// import MLSettings;


// M_EXPORT class IWorker
// {
// protected:
// 	std::unique_ptr<std::jthread> thread;

// 	bool stopThreadFlag = false;
// 	bool taskUpdated = false;
// 	std::atomic<bool> busy = false;

// 	void runTask()
// 	{
// 		busy = true;
// #ifdef _WIN32
// 		SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
// #elif __linux__
// 		sched_param params;
// 		params.sched_priority = 75;
// 		pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
// #endif
// 		taskUpdated = true;
// 	}


// 	void join()
// 	{
// 		if (thread && busy)
// 			thread->join();
// 	}

// 	bool isBusy() const
// 	{
// 		return busy;
// 	}

// 	void stop()
// 	{
// 		stopThreadFlag = true;
// 	}

// 	void runAsync()
// 	{
// 		thread.reset(new std::jthread([this] {this->runLoop(); }));
// 	}

// 	void runLoop()
// 	{
// 		while (!stopThreadFlag)
// 		{
// 			if (!taskUpdated)
// 				continue;

// 			taskUpdated = false;
// 			runSync();

// #ifdef _WIN32
// 			SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_NORMAL);
// #elif __linux__
// 			sched_param params;
// 			params.sched_priority = 20;
// 			pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
// #endif
// 		}
// 	}

// 	virtual void updateTask(BackImage& rrect, const TileProvider& tileProvider) = 0;
// 	virtual void runSync() = 0;
// 	virtual void setCallback(/*Project* proj, */RasterLineLayer* layer, const IItemFilter* filter) = 0;
// };


M_EXPORT class IAlg
{
public:
	virtual MLSettings* getSettings()
	{
		return nullptr;
	}
	virtual bool hasFilter()
	{
		return false;
	}

	virtual void setFilter(IItemFilter* filter)
	{ }

	virtual RetLayers execute(InOutLayer iol) = 0;

	virtual ~IAlg()
	{ }
};

M_EXPORT class IAlgBaseSettings : public IAlg
{
public:
	IAlgBaseSettings()
	{ }

	IAlgBaseSettings(const MLSettings& settings) :
		settings(settings)
	{ }

	MLSettings* getSettings() override
	{
		return &settings;
	}

protected:
	MLSettings settings;
};


M_EXPORT using IAlgRun = RetLayers(InOutLayer iol, const MLSettings& settings);


struct AlgData
{
	BackString name;
	std::unique_ptr<IAlg> creator;
	int categoryId;
};

M_EXPORT class AlgFactory
{
	// using BaseCreator = std::function<IAlgRun>;
public:
	std::vector<AlgData> functions;
	std::unordered_map<BackString, int> categories;

	AlgFactory()
	{ }

	void operator=(const AlgFactory&) = delete;
	AlgFactory(const AlgFactory& right) = delete;
	AlgFactory(AlgFactory&& right) = delete;

	static AlgFactory* raster;
	static AlgFactory& getRasterFactory()
	{
		if(!raster)
		{
			raster = new AlgFactory();
		}
		return *raster;
	}

	static AlgFactory* vector;
	static AlgFactory& getVectorFactory()
	{
		if(!vector)
		{
			vector = new AlgFactory();
		}
		return *vector;
	}

	static void deleteRaster()
	{
		delete raster;
		raster = nullptr;
	}

	static void deleteVector()
	{
		delete vector;
		vector = nullptr;
	}

	int registerFactory(std::string_view name, std::string_view category, IAlg* pointer)
	{
		BackString bname(name);
		BackString bcategory(category);
		int catId = 0;
		if (categories.count(bcategory))
			catId = categories[bcategory];
		else
			catId = categories[bcategory] = categories.size();

		functions.push_back({bname, std::unique_ptr<IAlg>(pointer), catId});
		// creators.push_back([] { return new TWorker(); });
		return functions.size() - 1;
	}

	IAlg* get(int id)
	{
		assert(id >= 0 && id < functions.size());
		return functions[id].creator.get();
	}

	// RetLayers Execute(int id, InOutLayer& iol, const MLSettings& settings)
	// {
	// 	assert(id >= 0 && id < functions.size());
	// 	return functions[id](iol, settings);
	// }

	RetLayers execute(int id, InOutLayer& iol)
	{
		assert(id >= 0 && id < functions.size());
		return functions[id].creator->execute(iol);
	}
};

class ProtoIAlgBaseSettings : public IAlgBaseSettings
{
	std::function<IAlgRun> funcPtr;

public:

	ProtoIAlgBaseSettings(std::function<IAlgRun> func, const MLSettings& settings)
		: IAlgBaseSettings(settings), funcPtr(func)
	{ }

	RetLayers execute(InOutLayer iol) override
	{
		return funcPtr(iol, settings);
	}
};


M_EXPORT template<class TAlg>
class AlgRegister
{
public:
	AlgRegister(std::string_view name, std::string_view category = "Common")
	{
		AlgFactory::getRasterFactory().registerFactory(name, category, new TAlg());
	}
};


M_EXPORT class AlgFuncRegister
{
public:
	AlgFuncRegister(std::string_view name, IAlgRun ptr, std::function<MLSettings()> settingCreation, std::string_view category = "Common")
	{
		AlgFactory::getRasterFactory().registerFactory(name, category, new ProtoIAlgBaseSettings(ptr, settingCreation()));
	}
};
