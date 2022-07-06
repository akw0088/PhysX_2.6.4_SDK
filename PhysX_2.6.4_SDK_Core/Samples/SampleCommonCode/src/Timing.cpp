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
	static uint64_t ulScale=0;
	uint64_t ulTime;

	if (ulScale==0) {
		ulScale = sys_time_get_timebase_frequency() / 1000;
	}

	asm __volatile__ ("mftb %0" : "=r" (ulTime) : : "memory");

	return ulTime/ulScale;
}

#elif defined(LINUX)

#include <sys/time.h>

unsigned long timeGetTime()
{
        timeval tim;
	gettimeofday(&tim, NULL);
	unsigned long ms = (tim.tv_sec*1000)+(tim.tv_usec/1000.0);
	return ms;
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

#if !defined(LINUX)

float getElapsedTime()
{
	static LARGE_INTEGER previousTime;
	static LARGE_INTEGER freq;
	static bool init = false;
	if(!init){
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&previousTime);
		init=true;
	}
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	unsigned long long elapsedTime = currentTime.QuadPart - previousTime.QuadPart;
	previousTime = currentTime;
	return (float)(elapsedTime)/(freq.QuadPart);
}

#else

float getElapsedTime()
{
	static timeval previousTime;
	static bool init = false;
	if(!init){
 	        gettimeofday(&previousTime, NULL);
		init=true;
	}
	timeval currentTime;
	gettimeofday(&currentTime, NULL);
	double elapsedTime = (currentTime.tv_sec+currentTime.tv_usec/1000000.0) - (previousTime.tv_sec+previousTime.tv_usec/1000000.0);
	previousTime = currentTime;
	return (float)elapsedTime;
}

#endif
