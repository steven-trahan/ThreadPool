#pragma once

#include <deque>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <functional>
#include "ThreadPoolExceptions.h"

namespace threadpool
{

template <class T, class Container = std::deque<T>>
class ThreadSafeQueue
{
public:
   using container_type = Container;
   using value_type = typename Container::value_type;
   using size_type = typename Container::size_type;
   using reference = typename Container::reference;
   using const_reference = typename Container::const_reference;

   ThreadSafeQueue() = default;
   ~ThreadSafeQueue() = default;
   ThreadSafeQueue(const ThreadSafeQueue&) = delete;
   ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
   ThreadSafeQueue(const ThreadSafeQueue&&) = delete;
   ThreadSafeQueue& operator=(const ThreadSafeQueue&&) = delete;
   
   bool empty() const
   {
      // Read-only access, so shared lock is sufficient
      std::shared_lock<std::shared_mutex> lock(_mutex);
      return _deque.empty();
   }

   size_type size() const
   {
      // Read-only access, so shared lock is sufficient
      std::shared_lock<std::shared_mutex> lock(_mutex);
      return _deque.size();
   }

   void push(const value_type& value)
   {
      // Write access, so exclusive lock is required
      std::lock_guard<std::shared_mutex> lock(_mutex);
      _deque.push_back(value);
   }

   void push(value_type&& value)
   {
      // Write access, so exclusive lock is required
      std::lock_guard<std::shared_mutex> lock(_mutex);
      _deque.push_back(std::move(value));
   }

   value_type pop()
   {
      // Write access, so exclusive lock is required
      std::lock_guard<std::shared_mutex> lock(_mutex);
      if (_deque.size() == 0)
      {
         throw QueueEmpty();
      }

      value_type poppedValue = _deque.front();
      _deque.pop_front();
      return poppedValue;
   }

   std::pair<value_type, bool> pop_nothrow()
   {
      // Write access, so exclusive lock is required
      std::lock_guard<std::shared_mutex> lock(_mutex);
      if (_deque.size() == 0)
      {
         value_type empty;
         return std::make_pair(std::move(empty), false);
      }

      value_type poppedValue = std::move(_deque.front());
      _deque.pop_front();
      return std::make_pair(std::move(poppedValue), true);
   }

   void clear()
   {
      // Write access, so exclusive lock is required
      std::lock_guard<std::shared_mutex> lock(_mutex);
      _deque.clear();
   }

private:
   mutable std::shared_mutex _mutex;
   Container _deque;
};
}
