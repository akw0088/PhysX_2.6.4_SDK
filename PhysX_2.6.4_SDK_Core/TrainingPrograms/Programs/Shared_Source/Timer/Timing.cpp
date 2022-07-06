#include <stdio.h>
#ifdef WIN32 
#	define NOMINMAX
#	include <windows.h>
#endif

#if defined(_XBOX)
#	include <xtl.h>
#endif

#include "Nx.h"
#include "Timing.h"

#if defined(__CELLOS_LV2__)

#include <sys/sys_time.h>
#include <time_util.h>

unsigned long timeGetTime()
{
	static long ulScale=0;
	unsigned long ulTime;

	if (ulScale==0) {
		ulScale = sys_time_get_timebase_frequency() / 1000;
	}

	asm __volatile__ ("mftb %0" : "=r" (ulTime) : : "memory");

	return ulTime/ulScale;
}

#elif defined(_XBOX)
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

float getCurrentTime()
{
	unsigned int currentTime = timeGetTime();
	return (float)(currentTime)*0.001f;
}

float getElapsedTime()
{
	static unsigned int previousTime = timeGetTime();
	unsigned int currentTime = timeGetTime();
	unsigned int elapsedTime = currentTime - previousTime;
	previousTime = currentTime;
	return (float)(elapsedTime)*0.001f;
}
