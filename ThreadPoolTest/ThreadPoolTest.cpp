#include "gtest/gtest.h"
#include <ThreadPool.h>

TEST(test_case_name, ThreadPoolTest)
{
   EXPECT_EQ(1, 1.0);
   EXPECT_TRUE(true);
}

TEST(CreatePool_NoError, ThreadPoolTest)
{
   EXPECT_NO_THROW(threadpool::ThreadPool pool);
}

TEST(CreatePool_GetSize_NoError, ThreadPoolTest)
{
   threadpool::ThreadPool pool;
   const size_t kExpectedSize = 10;
   EXPECT_EQ(pool.threadSize(), kExpectedSize);
}

TEST(CreatePoolWithJobs_Start_CanDestruct, ThreadPoolTest)
{
   threadpool::ThreadPool pool;

   std::vector<std::future<uint32_t>> futures;

   auto myLambda = []()->uint32_t
   {
      std::cout << "Hello World!" << std::endl;
      return 0;
   };

   for (size_t i = 0; i < 15; ++i)
   {
      futures.push_back(pool.postJob<uint32_t>(myLambda));
   }

   pool.Start();
   
   for (auto& future : futures)
   {
      uint32_t kExpectedFuture = 0;
      ASSERT_EQ(future.get(), kExpectedFuture);
   }
   pool.Stop();
}

TEST(CreatePoolWithJobs_StartTwice_Error, ThreadPoolTest)
{
   threadpool::ThreadPool pool;
   ASSERT_NO_THROW(pool.Start());
   EXPECT_THROW(pool.Start(), threadpool::PoolInWrongState);
}

TEST(CreatePool_Stop_Error, ThreadPoolTest)
{
   threadpool::ThreadPool pool;
   EXPECT_THROW(pool.Stop(), threadpool::PoolInWrongState);
}

TEST(CreatePool_CheckIsBusy_False, ThreadPoolTest)
{
   threadpool::ThreadPool pool;
   EXPECT_FALSE(pool.isBusy());
}

TEST(CreatePoolWithJobs_ClearJobs_PoolNotBusy, ThreadPoolTest)
{
   std::vector<std::future<uint32_t>> futures;
   threadpool::ThreadPool pool;

   auto myLambda = []()->uint32_t
   {
      std::cout << "Hello World!" << std::endl;
      return 0;
   };

   for (size_t i = 0; i < 15; ++i)
   {
      futures.push_back(pool.postJob<uint32_t>(myLambda));
   }

   pool.Start();
   pool.clearJobs();
   EXPECT_FALSE(pool.isBusy());
   pool.Stop();
}

TEST(CreatePoolWithJobs_WaitUntilIdle_PoolNotBusy, ThreadPoolTest)
{
   std::vector<std::future<uint32_t>> futures;
   threadpool::ThreadPool pool;

   auto myLambda = []()->uint32_t
   {
      std::cout << "Hello World!" << std::endl;
      return 0;
   };

   for (size_t i = 0; i < 15; ++i)
   {
      futures.push_back(pool.postJob<uint32_t>(myLambda));
   }

   pool.Start();
   pool.clearJobs();
   pool.Stop();
   pool.waitUntilIdle();
   EXPECT_FALSE(pool.isBusy());
}
