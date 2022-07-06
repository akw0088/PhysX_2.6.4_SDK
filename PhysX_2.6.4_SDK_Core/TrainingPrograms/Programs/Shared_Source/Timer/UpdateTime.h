// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							         UPDATE TIME
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef UPDATE_TIME_H
#define UPDATE_TIME_H

#include "Nx.h"

#ifdef WIN32 
	#define NOMINMAX
	#include <windows.h>
#endif

#if defined(_XBOX)
	#include <xtl.h>

unsigned long timeGetTime()
{
	LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    unsigned long long ticksPerMillisecond = freq.QuadPart/1000;

	LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (unsigned long)(counter.QuadPart/ticksPerMillisecond);
}
#endif


#if defined(__CELLOS_LV2__)

#include <sys/sys_time.h>
#include <time_util.h>

unsigned long timeGetTime()
{
	static uint64_t ulScale=0;
	uint64_t ulTime;

	if (ulScale==0) {
		ulScale = sys_time_get_timebase_frequency() / 1000;
	}

	asm __volatile__ ("mftb %0" : "=r" (ulTime) : : "memory");

	return ulTime/ulScale;
}
#endif

float UpdateTime()
{
	static unsigned int previousTime = timeGetTime();
	unsigned int currentTime = timeGetTime();
	unsigned int elapsedTime = currentTime - previousTime;
	previousTime = currentTime;
	return (float)(elapsedTime)*0.001f;
}

#endif  // UPDATETIME_H
