#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

typedef unsigned int uint;
typedef long long int64;
typedef unsigned long long uint64;

#ifdef __APPLE__
inline int64_t JULIANTIMESTAMP()
{
  uint64_t ts = mach_absolute_time();
  static mach_timebase_info_data_t info = {0, 0};
  if (info.denom == 0)
    mach_timebase_info(&info);

  uint64_t elapsednano = ts * (info.numer / info.denom);
  return elapsednano / 1000;
}
#else
inline int64 JULIANTIMESTAMP()
{
  struct timespec tTime;
  clock_gettime(CLOCK_MONOTONIC, &tTime);
  int64 microSeconds = (int64) tTime.tv_sec * 1000000000 + tTime.tv_nsec;
  return microSeconds / 1000;
}
#endif

#endif
