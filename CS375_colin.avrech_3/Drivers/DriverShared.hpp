/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_SHARED
#define COMPILER_CLASS_DRIVER_SHARED

typedef void (*DriverTestFn)();
typedef void (*RunWithInputFn)(const char* input);
#define DriverArraySize(array) (sizeof(array) / sizeof(array[0]))

int DriverMain(int argc, char* argv[], DriverTestFn tests[], size_t testCount);

#endif
