#ifndef TIME_H
#define TIME_H
#include <cstddef>
#include <sys/time.h>
timeval ___StatTime_Start();
extern double  ___StatTime_End(timeval t1);
#endif
