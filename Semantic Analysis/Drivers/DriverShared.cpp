/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

// Comment this line out if you do not have Visual Leak Detector installed (https://vld.codeplex.com/)
// VLD will be ran on your submitted code and you will be docked points for memory leaks
//#include <vld.h>

#include "DriverShared.hpp"
#include <stdlib.h>
#include <stdio.h>

int DriverMain(int argc, char* argv[], DriverTestFn tests[], size_t testCount)
{
  // If no arguments were supplied (first is always the exe)
  if (argc == 1)
  {
    for (size_t i = 0; i < testCount; ++i)
      tests[i]();
  }
  // If a test number was supplied
  else if (argc >= 2)
  {
    size_t index = (size_t)atoi(argv[1]);
    if (index < testCount)
      tests[index]();
    else
      printf("Invalid test case number\n");
  }

  // Only pause if we have 0 or 1 arguments (2 means the driver is being run by the server)
  if (argc == 1 || argc == 2)
  {
    printf("Press enter/return to exit...\n");
    getchar();
  }
  return 0;
}
