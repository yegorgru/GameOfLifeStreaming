#include "ThreadPoolManager.h"
#include "Log.h"
#include "Print.h"

#include <stop_token>

namespace Streaming::Poco {

ThreadPoolManager::ThreadPoolManager()
    : mQueueNotEmpty(false)
{
}

ThreadPoolManager::~ThreadPoolManager() {
    Log::Debug("Destroying Poco thread pool");
    stop();
}

void ThreadPoolManager::start(unsigned int minThreads, unsigned int maxThreads) {
    stop();
    
    mThreadPool = std::make_unique<ThreadPoolPoco>(minThreads, maxThreads);
    
    mQueueNotEmpty = false;
    startQueueMonitor();
}

void ThreadPoolManager::stop() {
    stopQueueMonitor();
    
    if (mThreadPool) {
        Log::Debug("Stopping Poco thread pool");
        {
            Log::Debug(Print::composeMessage(mTaskQueue.size(), " tasks in the queue before stopping"));
            std::lock_guard<std::mutex> lock(mQueueMutex);
            TaskQueue empty;
            std::swap(mTaskQueue, empty);
            mQueueNotEmpty = false;
        }
        mThreadPool->joinAll();
        Log::Debug("Poco thread pool stopped successfully");
        mThreadPool->stopAll();
        mThreadPool.reset();
    }
}

void ThreadPoolManager::processNextTask() {
    RunnableTaskPtr task;
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        if (mTaskQueue.empty()) {
            return;
        }
        task = mTaskQueue.front();
    }
    
    bool taskStarted = false;
    try {
        if (mThreadPool) {
            mThreadPool->start(*task);
            taskStarted = true;
        }
    }
    catch (const ::Poco::NoThreadAvailableException&) {
        std::this_thread::yield();
    }
    
    if (taskStarted) {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        if (!mTaskQueue.empty() && mTaskQueue.front() == task) {
            mTaskQueue.pop();
        }
    }
}

void ThreadPoolManager::startQueueMonitor() {
    if (mQueueMonitorThread.joinable()) {
        return;
    }
    
    mQueueMonitorThread = std::jthread([this](std::stop_token stopToken) {       
        while (!stopToken.stop_requested()) {
            mQueueNotEmpty.wait(false);
            
            if (stopToken.stop_requested()) {
                break;
            }
            
            processNextTask();
            
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                mQueueNotEmpty = !mTaskQueue.empty();
            }
        }
        
        Log::Debug("Queue monitor thread stopped");
    });
}

void ThreadPoolManager::stopQueueMonitor() {
    if (!mQueueMonitorThread.joinable()) {
        return;
    }
    
    mQueueMonitorThread.request_stop();
    
    mQueueNotEmpty = true;
    mQueueNotEmpty.notify_one();
    
    if (mQueueMonitorThread.joinable()) {
        mQueueMonitorThread.join();
    }
}

void ThreadPoolManager::enqueue(Task&& task) {
    if (!mThreadPool) {
        return;
    }

    auto runnableTask = std::make_shared<RunnableTask>(std::move(task), *this);
    
    {
        std::lock_guard<std::mutex> lock(mActiveTasksMutex);
        mActiveTasks.insert(runnableTask);
    }
    
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mTaskQueue.push(runnableTask);
        mQueueNotEmpty = true;
        mQueueNotEmpty.notify_one();
    }
}

void ThreadPoolManager::removeActiveTask(const std::shared_ptr<RunnableTask>& task) {
    std::lock_guard<std::mutex> lock(mActiveTasksMutex);
    mActiveTasks.erase(task);
}

RunnableTask::RunnableTask(Task task, ThreadPoolManager& manager) 
    : mTask(std::move(task))
    , mManager(manager)
{
}

void RunnableTask::run() 
{
    try {
        if (mTask) {
            mTask();
        }
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Exception in task execution: ", e.what()));
    } catch (...) {
        Log::Error("Unknown exception in task execution");
    }
    
    auto selfPtr = shared_from_this();
    mManager.removeActiveTask(selfPtr);
}

} // namespace Streaming::Poco