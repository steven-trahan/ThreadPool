#include "ThreadPool.h"

namespace threadpool
{

ThreadPool::ThreadPool() :
   _running(false),
   _shouldTerminate(false)
{
   const uint32_t kNumThreads = ((std::thread::hardware_concurrency() - 2) > 1) ? std::thread::hardware_concurrency() - 2 : 1;

   _threads.reserve(kNumThreads);

   for (uint32_t i = 0; i < kNumThreads; ++i)
   {
      _threads.push_back(std::thread(&ThreadPool::_threadLoop, std::ref(*this)));
   }
}

ThreadPool::~ThreadPool()
{
   _running = false;
   _shouldTerminate = true;

   clearJobs();

   {
      std::lock_guard<std::mutex> guard(_threadMutex);
      _threadSignal.notify_all();
   }

   for (auto& thread : _threads)
   {
      thread.join();
   }
}

void ThreadPool::Start()
{
   if (_running)
   {
      throw PoolInWrongState();
   }

   _running = true;

   std::lock_guard<std::mutex> guard(_threadMutex);
   _threadSignal.notify_all();
}

void ThreadPool::Stop()
{
   if (!_running)
   {
      throw PoolInWrongState();
   }

   _running = false;
   {
      std::lock_guard<std::mutex> guard(_threadMutex);
      _threadSignal.notify_all();
   }
}

size_t ThreadPool::threadSize() const
{
   return _threads.size();
}

bool ThreadPool::isBusy() const
{
   return _running && !_jobQueue.empty();
}

void ThreadPool::clearJobs()
{
   _jobQueue.clear();
}

void ThreadPool::waitUntilIdle()
{
   while (isBusy())
   {
      std::this_thread::yield();
   }
}

void ThreadPool::_threadLoop()
{
   while (!_shouldTerminate)
   {
      if (_running)
      {
         auto result = _jobQueue.pop_nothrow();
         if (result.second)
         {
            result.first();
         }
         else
         {
            //No jobs available
            _waitForSignal();
         }
      }
      else
      {
         _waitForSignal();
      }
   }
}

void ThreadPool::_waitForSignal()
{
   std::unique_lock<std::mutex> guard(_threadMutex);
   _threadSignal.wait(guard, [this]
      {
         return _shouldTerminate || _running;
      }
   );
}

ThreadPool& getThreadPoolInstance()
{
   static ThreadPool* SingleInstance = nullptr;
   if (SingleInstance == nullptr)
   {
      SingleInstance = new ThreadPool();
   }

   return *SingleInstance;
}
}
