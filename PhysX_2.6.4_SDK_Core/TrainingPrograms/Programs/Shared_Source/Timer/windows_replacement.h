#ifdef LINUX

#ifndef WINDOWS_H
#define WINDOWS_H

#include <time.h>
#include <cstdio>
typedef unsigned long DWORD;

inline DWORD timeGetTime() {
	clock_t t = clock();
	return t / 1000;
}
#endif

#define ZeroMemory(pointer, size) memset(pointer, 0, size)

#endif
