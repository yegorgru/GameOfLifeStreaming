#pragma once

#include "Singleton.h"
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <set>

namespace Streaming::Poco {

using Task = std::function<void()>;

class ThreadPoolManager;

class RunnableTask : public ::Poco::Runnable, public std::enable_shared_from_this<RunnableTask> {
public:
    explicit RunnableTask(Task task, ThreadPoolManager& manager);
    void run() override;
    
private:
    Task mTask;
    ThreadPoolManager& mManager;
};

class ThreadPoolManager : public Singleton<ThreadPoolManager>
{
    friend class Singleton<ThreadPoolManager>;
    friend class RunnableTask;
public:
    void start(unsigned int minThreads = 1, unsigned int maxThreads = 4);
    void stop();
    ~ThreadPoolManager();
public:
    void enqueue(Task&& task);
private:
    ThreadPoolManager();
private:    
    void processNextTask();
    void startQueueMonitor();
    void stopQueueMonitor();
    void removeActiveTask(const std::shared_ptr<RunnableTask>& task);
private:    
    using ThreadPoolPoco = ::Poco::ThreadPool;
    using ThreadPoolPtr = std::unique_ptr<ThreadPoolPoco>;
    using RunnableTaskPtr = std::shared_ptr<RunnableTask>;
    using TaskQueue = std::queue<RunnableTaskPtr>;
    using AtomicFlag = std::atomic<bool>;
    using ActiveTasksStorage = std::set<RunnableTaskPtr>;
private:
    ThreadPoolPtr mThreadPool;
    TaskQueue mTaskQueue;
    std::mutex mQueueMutex;
    ActiveTasksStorage mActiveTasks;
    std::mutex mActiveTasksMutex;
    AtomicFlag mQueueNotEmpty;
    std::jthread mQueueMonitorThread;
};

} // namespace Streaming::Poco