#include <gtest/gtest.h>
#include <ThreadSafeQueue.h>
#include <future>
#include <random>

TEST(CreateThreadSafeQueue_NoError, TestThreadSafeQueue)
{
   ASSERT_NO_THROW(threadpool::ThreadSafeQueue<size_t> queue);
}

TEST(CreateThreadSafeQueue_Pop_Error, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   EXPECT_THROW(queue.pop(), threadpool::QueueEmpty);
}

TEST(CreateThreadSafeQueue_PopNoThrow_NoelementReturned, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   auto element = queue.pop_nothrow();
   EXPECT_FALSE(element.second);
}

TEST(CreateThreadSafeQueue_CheckEmpty_True, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   bool isQueueEmpty = queue.empty();
   EXPECT_TRUE(isQueueEmpty);
}

TEST(CreateThreadSafeQueue_CheckEmpty_False, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   queue.push(1);
   bool isQueueEmpty = queue.empty();
   EXPECT_FALSE(isQueueEmpty);
}

TEST(CreateThreadSafeQueue_PopNoThrow_ElementReturned, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   queue.push(1);
   auto element = queue.pop_nothrow();
   EXPECT_TRUE(element.second);
   EXPECT_EQ(element.first, static_cast<size_t>(1));
}

TEST(TSQ_StoreSequence_PopSameSequence, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<uint32_t> queue;

   std::vector<uint32_t> testSequence = { 1, 2, 3, 4, 5 };
   for (auto& number : testSequence)
   {
      queue.push(number);
   }

   for (auto& number : testSequence)
   {
      uint32_t poppedNumber = queue.pop();
      EXPECT_EQ(poppedNumber, number);
   }
}

TEST(TSQ_ThreadPushingNumbers_BothThreadsSizeReturned, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   constexpr size_t kNumItems = 20;

   auto async1 = std::async(std::launch::async, 
      [&queue, kNumItems]()
      {
         std::default_random_engine generator;
         for (size_t i = 0; i < kNumItems; ++i)
         {
            int msToSleep = generator() % kNumItems + 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(msToSleep));
            queue.push(i);
         }
      }
   );

   auto async2 = std::async(std::launch::async,
      [&queue, kNumItems]()
      {
         std::default_random_engine generator;
         for (size_t i = kNumItems; i > 0; --i)
         {
            int msToSleep = generator() % kNumItems + 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(msToSleep));
            queue.push(i);
         }
      }
   );

   async1.get();
   async2.get();

   const size_t kExpectedSize = kNumItems * 2;
   EXPECT_EQ(kExpectedSize, queue.size());
}

TEST(TSQ_ThreadsPushingAndPoppingNumbers_SizeTimes2Popped, TestThreadSafeQueue)
{
   threadpool::ThreadSafeQueue<size_t> queue;
   constexpr size_t kNumItems = 20;

   auto async1 = std::async(std::launch::async,
      [&queue, kNumItems]()
      {
         std::default_random_engine generator;
         for (size_t i = 0; i < kNumItems; ++i)
         {
            int msToSleep = generator() % kNumItems + 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(msToSleep));
            queue.push(i);
         }
      }
   );

   auto async2 = std::async(std::launch::async,
      [&queue, kNumItems]()
      {
         std::default_random_engine generator;
         for (size_t i = kNumItems; i > 0; --i)
         {
            int msToSleep = generator() % kNumItems + 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(msToSleep));
            queue.push(i);
         }
      }
   );

   size_t numPopped = 0;
   auto async3 = std::async(std::launch::async,
      [&queue, &numPopped, kNumItems]()
      {
         while (numPopped < kNumItems * 2)
         {
            auto element = queue.pop_nothrow();
            if (element.second)
            {
               ++numPopped;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
         }

         for (size_t i = 0; i < 10; ++i)
         {
            auto element = queue.pop_nothrow();
            if (element.second)
            {
               ++numPopped;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
         }
      }
   );

   async1.get();
   async2.get();
   async3.get();

   const size_t kExpectedSize = kNumItems * 2;
   EXPECT_EQ(kExpectedSize, numPopped);
}