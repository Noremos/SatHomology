//module;
//#include <memory>
//#include <thread>
//
//#ifdef __linux__
//#include <pthread.h>
//#endif
//
//#ifdef _WIN32
//#include <windows.h>
//#endif
//#include <vector>
//#include <functional>
//
//export module IAlgoriphm;
//import RasterLayers;
//import BackBind;
//import LayersCore;
//import IItemModule;
//
//
//export class IWorker
//{
//protected:
//	std::unique_ptr<std::jthread> thread;
//
//	bool stopThreadFlag = false;
//	bool taskUpdated = false;
//	std::atomic<bool> busy = false;
//
//public:
//	void runTask()
//	{
//		busy = true;
//#ifdef _WIN32
//		SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
//#elif __linux__
//		sched_param params;
//		params.sched_priority = 75;
//		pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
//#endif
//		taskUpdated = true;
//	}
//
//
//	void join()
//	{
//		if (thread && busy)
//			thread->join();
//	}
//
//	bool isBusy() const
//	{
//		return busy;
//	}
//
//	void stop()
//	{
//		stopThreadFlag = true;
//	}
//
//	void runAsync()
//	{
//		thread.reset(new std::jthread([this] {this->runLoop(); }));
//	}
//
//	void runLoop()
//	{
//		while (!stopThreadFlag)
//		{
//			if (!taskUpdated)
//				continue;
//
//			taskUpdated = false;
//			runSync();
//
//#ifdef _WIN32
//			SetThreadPriority(thread->native_handle(), THREAD_PRIORITY_NORMAL);
//#elif __linux__
//			sched_param params;
//			params.sched_priority = 20;
//			pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &params);
//#endif
//		}
//	}
//
//	virtual void updateTask(BackImage& rrect, const TileProvider& tileProvider) = 0;
//	virtual void runSync() = 0;
//	virtual void setCallback(/*Project* proj, */RasterLineLayer* layer, const IItemFilter* filter) = 0;
//};
//
//export class AlgFactory
//{
//	using BaseCreator = std::function<std::unique_ptr<IWorker>()>;
//	std::vector<BaseCreator> creators;
//public:
//
//	std::vector<BackString> names;
//	static AlgFactory factory;
//
//	template<class TWorker>
//	int registerFactory(std::string_view name)
//	{
//		names.push_back(name);
//		creators.push_back([] { return new TWorker(); });
//		return creators.size() - 1;
//	}
//
//	template<class IF>
//	std::unique_ptr<IF> Create(int id)
//	{
//		assert(id >= 0 && id < creators.size());
//		return creators[id]();
//	}
//};