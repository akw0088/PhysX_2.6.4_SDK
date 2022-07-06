#ifndef TIMING_H
#define TIMING_H

#if defined(__CELLOS_LV2__) || defined(_XBOX)
	unsigned long timeGetTime();
#elif defined(WIN32) || defined(_WIN64)

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <windows.h>


#endif

	float getCurrentTime();
	float getElapsedTime();

#endif
