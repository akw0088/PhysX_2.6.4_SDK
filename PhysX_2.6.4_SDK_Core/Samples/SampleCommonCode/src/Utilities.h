
#ifndef UTILITIES_H
#define UTILITIES_H

#if defined(WIN32)
#include <direct.h>
#include <shlwapi.h>
#endif

const char* getNxSDKCreateError(const NxSDKCreateError& errorCode) 
{
	switch(errorCode) 
	{
		case NXCE_NO_ERROR: return "NXCE_NO_ERROR";
		case NXCE_PHYSX_NOT_FOUND: return "NXCE_PHYSX_NOT_FOUND";
		case NXCE_WRONG_VERSION: return "NXCE_WRONG_VERSION";
		case NXCE_DESCRIPTOR_INVALID: return "NXCE_DESCRIPTOR_INVALID";
		case NXCE_CONNECTION_ERROR: return "NXCE_CONNECTION_ERROR";
		case NXCE_RESET_ERROR: return "NXCE_RESET_ERROR";
		case NXCE_IN_USE_ERROR: return "NXCE_IN_USE_ERROR";
		default: return "Unknown error";
	}
};


static void RemoveFileFromPath(char *path)
{
  char *ls = 0;
  while(*path)
  {
    if(*path == '\\' || *path == '/') ls = path;
    path++;
  }
  if(ls) *ls = 0;
}

static void SetCWDToEXE(void)
{
#if defined(WIN32)

  char exepath[1024] = {0};
  

  GetModuleFileNameA(0, exepath, sizeof(exepath));
  
  assert(exepath[0]);
  RemoveFileFromPath(exepath);
  chdir(exepath);
#endif
}

// PT: those functions are useful for internal profiling during dev. They should not be used
// in the final code, so don't bother porting them to other platforms.

	//!	This function starts recording the number of cycles elapsed.
	//!	\param		val		[out] address of a 32 bits value where the system should store the result.
	//!	\see		EndProfile
	//!	\see		InitProfiler
	__forceinline void	StartProfile(unsigned int& val)
	{
#if defined(WIN32)
		__asm{
			cpuid
			rdtsc
			mov		ebx, val
			mov		[ebx], eax
		}
#endif
	}

	//!	This function ends recording the number of cycles elapsed.
	//!	\param		val		[out] address to store the number of cycles elapsed since the last StartProfile.
	//!	\see		StartProfile
	//!	\see		InitProfiler
	__forceinline void	EndProfile(unsigned int& val)
	{
#if defined(WIN32)
		__asm{
			cpuid
			rdtsc
			mov		ebx, val
			sub		eax, [ebx]
			mov		[ebx], eax
		}
#endif
	}

#endif
