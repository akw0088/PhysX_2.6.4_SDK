#ifndef PXD_CONFIG_H
#define PXD_CONFIG_H

#ifdef __cplusplus
extern "C" 
{
#endif

//System headers
#include <float.h>
#include <stddef.h>

/*! 
\file 
Types and defines.
*/

/************************************************************************/
/* Compiler detection                                                   */
/************************************************************************/
#ifdef _MSC_VER
	#define	PXD_COMPILER_VC
#elif __GNUC__
	#define PXD_COMPILER_GNUC
#else
	#error "Compiler not recognized"
#endif

/*!
\def PXD_COMPILER_VC 
Set whenever Microsoft VC is detected as current compiler
*/

/************************************************************************/
/* Platform detection                                                   */
/************************************************************************/
#ifdef PXD_COMPILER_VC
	#ifdef _M_IX86
		#define PXD_PLATFORM_X86
		#define PXD_PLATFORM_WINDOWS

		#ifndef PXD_DISABLE_PPU
			#define PXD_SUPPORT_PPU
		#endif
		//Disable SSE for now
		//#define PXD_SUPPORT_SSE
		//#define PXD_SUPPORT_DYNAMICLIB // Disabled because we are rolling LowLevel into PhysXCore for 2.5
		
	#elif defined(_M_PPC)
		#define PXD_PLATFORM_PPC
		#define PXD_PLATFORM_XBOX360

		#define PXD_SUPPORT_VMX128 
	#else
		#error "Unknown platform"
	#endif
#elif defined(PXD_COMPILER_GNUC)
	#ifdef __CELLOS_LV2__
		#define PXD_PLATFORM_CELL
	#elif defined(LINUX)
		#define PXD_PLATFORM_X86
		#define PXD_PLATFORM_LINUX
	#else
		#error "Unknown platform"
	#endif
#else
	#error "Unknown compiler"
#endif

/*!
\def PXD_PLATFORM_X86
Set when Intel x86 (32-bit) compatible platform is detected.
*/
/*!
\def PXD_PLATFORM_WINDOWS
Set when we are compiling for a windows target(32 or 64bit). When it makes sense to include windows.h.
*/
/*!
\def PXD_PLATFORM_XBOX360
Set when we are compiling on the xbox360. When it makes sense to include xtl.h etc. (Compiling on a power PC is not the same thing) 
*/
/*!
\def PXD_SUPPORT_PPU
Set when we wish to support the PPU, this is logically distinct from the platform we run on.
*/
/*!
\def PXD_SUPPORT_SSE
Set when the platform supports SSE
*/
/*!
\def PXD_SUPPORT_VMX128
Set when the platform supports VMX128 (currently only the xbox360)
*/

/*!
\def PXD_SUPPORT_DYNAMICLIB
Set when we are building as a dynamic library(and should export functions)
*/

/************************************************************************/
/* Some general defines                                                 */
/************************************************************************/
#ifndef PXD_INLINE
	#define PXD_INLINE inline
#endif

/*!
\def PXD_INLINE
Macro for general inline functions
*/

#ifndef PXD_FORCE_INLINE
	#if defined(PXD_COMPILER_VC)
		#define PXD_FORCE_INLINE __forceinline
	#elif defined(PXD_PLATFORM_LINUX) // Workaround; Fedora Core 3 do not agree with force inline and PxcPool
		#define PXD_FORCE_INLINE inline
	#elif defined(PXD_COMPILER_GNUC)
		#define PXD_FORCE_INLINE inline __attribute__((always_inline))
	#else
		#define PXD_FORCE_INLINE inline
	#endif
#endif

/*!
\def PXD_FORCE_INLINE
Macro for functions that unconditionally should be inlined, given the
compiler supports that functionality.
*/

#ifdef PXD_BUILD_LIBRARY	
	#if defined(PXD_COMPILER_VC) && defined(PXD_SUPPORT_DYNAMICLIB)
		#define PXD_EXPORT_FUNCTION __declspec(dllexport)
	#else 
		#define PXD_EXPORT_FUNCTION 
	#endif
#else
	#if defined(PXD_COMPILER_VC) && defined(PXD_SUPPORT_DYNAMICLIB)
		#define PXD_EXPORT_FUNCTION __declspec(dllimport)
	#else 
		#define PXD_EXPORT_FUNCTION 
	#endif
#endif

/*!
\def PXD_EXPORT_FUNCTION
Modifier for externally exported functions.
*/

#if defined(PXD_PLATFORM_XBOX360) || defined(PXD_PLATFORM_CELL)
	#define PXD_RESTRICT __restrict
#else
	#define PXD_RESTRICT
#endif

/*!
\def PXD_RESTRICT
Equal to C99 __restrict keyword on compilers that supports it
*/


#if defined(PXD_COMPILER_VC) && defined(PXD_PLATFORM_X86)

#define PXD_PUSH_PACK(x)	pack(push, x)
#define PXD_POP_PACK		pack(pop)

#else

// pack(1) causes PPC code to assume arbitary alignment, which forces integer instruction to be used to load/store floats.
#define PXD_PUSH_PACK(x)
#define PXD_POP_PACK

#endif

#ifndef PXD_ALIGN
	#if defined(PXD_COMPILER_VC)
		#define PXD_ALIGN(alignment, decl) __declspec(align(alignment)) decl
	#elif defined(PXD_COMPILER_GNUC)
		#define PXD_ALIGN(alignment, decl) decl __attribute__ ((aligned(alignment)))
	#else
		#define PXD_ALIGN(alignment, decl)
	#endif
#endif

/************************************************************************/
/* Numerical defines                                                    */
/************************************************************************/
#define PXD_FLOAT_MAX		FLT_MAX
#define PXD_FLOAT_EPSILON	FLT_EPSILON
#define PXD_PI				3.1415926535897932384626433832795f

// PT: this used to be replicated everywhere in the code, causing bugs to sometimes reappear (e.g. TTP 3587).
// It is better to define it in a header and use the same constant everywhere. The original value (1e-05f)
// caused troubles (e.g. TTP 1705, TTP 306).
#define PXD_PARALLEL_TOLERANCE	1e-02f

// PT: typical "invalid" value in various CD algorithms
#define	PXC_INVALID		0xffffffff

/************************************************************************/
/* Compiler workarounds                                                 */
/************************************************************************/
#ifdef PXD_COMPILER_VC
#pragma warning(disable: 4355 )	// "this" used in base member initializer list
#pragma warning(disable: 4146 ) // unary minus operator applied to unsigned type.
#endif

#include "px_types.h"

// PT: skip initial "early exit" test when convexes were previously touching. Should be safe to always enable.
#define PXD_TEST_CONVEX_CONVEX_OPTIM_01	1
// PT: very experimental convex-convex optimization. Should be tested with real customer meshes before we keep it. Hence the define.
#define PXD_TEST_CONVEX_CONVEX_OPTIM_02	1
// PT: batch projection
//#define PXD_TEST_CONVEX_CONVEX_OPTIM_03	1

#ifdef __cplusplus
}
#endif

#endif
