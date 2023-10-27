#include "ThreadPool.h"

int main()
{
   auto& pool = threadpool::getThreadPoolInstance();

   pool.isBusy();
   pool.threadSize();
   
   pool.postJob<int>([]()->int {return 0; });
   pool.postJob<int>([]()->int {return 0; });
   pool.postJob<int>([]()->int {return 0; });


   pool.Start();
   pool.Stop();

   return 0;
}
