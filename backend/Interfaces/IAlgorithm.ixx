module;

#include <memory>

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

export module IAlgorithm;

import RasterLayers;
import BackBind;
import LayersCore;
import IItemModule;
import MLSettings;


// export class IWorker
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


export class IAlg
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

export class IAlgBaseSettings : public IAlg
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


export using IAlgRun = RetLayers(InOutLayer iol, const MLSettings& settings);


export class AlgFactory
{
	// using BaseCreator = std::function<IAlgRun>;
	std::vector<std::unique_ptr<IAlg>> functions;

public:
	std::vector<BackString> names;

	AlgFactory()
	{ }

	void operator=(const AlgFactory&) = delete;
	AlgFactory(const AlgFactory& right) = delete;
	AlgFactory(AlgFactory&& right) = delete;

	static AlgFactory& getRasterFactory()
	{
		static AlgFactory factory;
		return factory;
	}

	static AlgFactory& getVectorFactory()
	{
		static AlgFactory factory;
		return factory;
	}


	int registerFactory(std::string_view name, IAlg* pointer)
	{
		names.push_back(BackString(name.data(), name.length()));
		functions.push_back(std::unique_ptr<IAlg>(pointer));
		// creators.push_back([] { return new TWorker(); });
		return functions.size() - 1;
	}

	IAlg* get(int id)
	{
		assert(id >= 0 && id < functions.size());
		return functions[id].get();
	}

	// RetLayers Execute(int id, InOutLayer& iol, const MLSettings& settings)
	// {
	// 	assert(id >= 0 && id < functions.size());
	// 	return functions[id](iol, settings);
	// }

	RetLayers execute(int id, InOutLayer& iol)
	{
		assert(id >= 0 && id < functions.size());
		return functions[id]->execute(iol);
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


export template<class TAlg>
class AlgRegister
{
public:
	AlgRegister(std::string_view name)
	{
		AlgFactory::getRasterFactory().registerFactory(name, new TAlg());
	}
};


export class AlgFuncRegister
{
public:
	AlgFuncRegister(std::string_view name, IAlgRun ptr, std::function<MLSettings()> settingCreation)
	{
		AlgFactory::getRasterFactory().registerFactory(name, new ProtoIAlgBaseSettings(ptr, settingCreation()));
	}
};
