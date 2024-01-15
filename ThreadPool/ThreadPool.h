#pragma once
#include "ThreadSafeQueue.h"
#include <functional>
#include <thread>
#include <atomic>
#include <future>
#include "ThreadPoolExceptions.h"

namespace threadpool
{

class ThreadPool
{
public:
   ThreadPool();
   ~ThreadPool();

   ThreadPool& operator=(const ThreadPool&) = delete;
   ThreadPool(const ThreadPool&) = delete;

   void Start();
   void Stop();
   size_t threadSize() const;
   bool isRunning() const;
   bool isBusy() const;
   void clearJobs();
   void waitUntilIdle();

   template <class T>
   std::future<T> postJob(std::function<T()>&& job)
   {
      std::shared_ptr<std::promise<T>> promise = std::make_shared<std::promise<T>>();

      auto lambda = [promise, job] {
         T returnValue = std::invoke(job);
         promise->set_value(returnValue);
      };
      _jobQueue.push(std::move(lambda));

      if (_running)
      {
         std::lock_guard<std::mutex> guard(_threadMutex);
         _threadSignal.notify_one();
      }

      return promise->get_future();
   }

private:
   void _threadLoop();
   void _waitForSignal();

   std::atomic<bool> _running;
   std::atomic<bool> _shouldTerminate;
   ThreadSafeQueue<std::function<void()>> _jobQueue;
   std::vector<std::thread> _threads;
   std::mutex _threadMutex;
   std::condition_variable _threadSignal;
};

ThreadPool& getThreadPoolInstance();
}
