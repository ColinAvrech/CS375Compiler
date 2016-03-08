/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_SHARED
#define COMPILER_CLASS_DRIVER_SHARED

#include <memory>
#include <vector>
#include <unordered_map>

// This is a helper template class that we use to simplify pushing unique pointers into a vector
// The push_back function returns a bool so that it can be used in conditions (such as a while loop)
// It also will only push when the unique_ptr is not null
template <typename T>
class unique_vector : public std::vector<std::unique_ptr<T>>
{
public:
  bool push_back(std::unique_ptr<T> ptr)
  {
    if (ptr)
    {
      vector::push_back(std::move(ptr));
      return true;
    }
    return false;
  }
};

typedef void (*DriverTestFn)();
typedef void (*RunWithInputFn)(const char* input);
#define DriverArraySize(array) (sizeof(array) / sizeof(array[0]))

int DriverMain(int argc, char* argv[], DriverTestFn tests[], size_t testCount);

#endif
