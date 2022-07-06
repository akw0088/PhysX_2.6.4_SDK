#ifndef TIMING_H
#define TIMING_H

#if defined(__CELLOS_LV2__) || defined(_XBOX) || defined(LINUX)
	unsigned long timeGetTime();
#elif defined(WIN32) || defined(_WIN64)

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <windows.h>


#endif

#if defined(__CELLOS_LV2__)

#include <sys/sys_time.h>
#include <time_util.h>

typedef union _LARGE_INTEGER {
    uint64_t QuadPart;
} LARGE_INTEGER;

inline void QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount){
	SYS_TIMEBASE_GET(lpPerformanceCount->QuadPart);
}

inline void QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency){
	lpFrequency->QuadPart = sys_time_get_timebase_frequency();
}

#endif //defined(__CELLOS_LV2__)

#if defined(LINUX)

#include <stdint.h>
#include <time.h>

typedef union _LARGE_INTEGER {
	uint64_t QuadPart;
} LARGE_INTEGER;

inline void QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount){
	lpPerformanceCount->QuadPart = clock();
}

inline void QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency){
	lpFrequency->QuadPart = CLOCKS_PER_SEC;
}

#endif // defined(LINUX)

	float getCurrentTime();
	float getElapsedTime();

#endif
