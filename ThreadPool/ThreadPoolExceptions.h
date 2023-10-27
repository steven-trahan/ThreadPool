#pragma once
#include <exception>

namespace threadpool
{
   class QueueEmpty : public std::exception {};
   class PoolInWrongState : public std::exception {};
   class WrongMatrixSize : public std::exception {};
   class TaskMustReturnSomething : public std::exception {};
}