#ifndef NX_EXTENSIONS_HPP
#define NX_EXTENSIONS_HPP
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#ifdef _USRDLL
   #if defined(WIN32)
	#define NXEXTENSIONSDLL_API __declspec(dllexport)
   #elif defined(LINUX)
        #if defined(NX_LINUX_USE_VISIBILITY)
            #define NXEXTENSIONSDLL_API __attribute__ ((visibility ("default")))
        #else
            #define NXEXTENSIONSDLL_API
        #endif
   #else
        #define NXEXTENSIONSDLL_API
   #endif
#elif defined NX_USE_SDK_STATICLIBS
	#define NXEXTENSIONSDLL_API
#else
   #if defined(WIN32)
	#define NXEXTENSIONSDLL_API __declspec(dllimport)
   #elif defined(LINUX)
        #define NXEXTENSIONSDLL_API
   #else
        #define NXEXTENSIONSDLL_API
   #endif
#endif

#include "nxextension.h"

/**
* Create an NxExtensions instance.
*/
NXEXTENSIONSDLL_API NxExtension *getNxExtension(NxExtensionType type);
NXEXTENSIONSDLL_API void         releaseNxExtension(NxExtension *ext);

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
