#ifndef PX_TYPES_H
#define PX_TYPES_H

#ifdef __cplusplus
extern "C" 
{
#endif

/************************************************************************/
/* Basic type definitions                                               */
/************************************************************************/

// Types without specific size. Platform independent
typedef char PxBool;	// PT: "char" used instead of bool, for the C API. Creates a few performance warnings.
typedef float PxFloat;
typedef char PxChar;
typedef unsigned char PxUChar;
typedef char PxByte;
typedef unsigned char PxUByte;
typedef short int PxShort;
typedef unsigned short int PxUShort;
typedef int PxInt;
typedef unsigned int PxUInt;
typedef int PxIBool;	// PT: fast "integer bool" similar to BOOL

// Platform specific fixed-size types
#if defined(PXD_PLATFORM_X86) && defined(PXD_COMPILER_VC)
	typedef char PxI8;
	typedef unsigned char PxU8;
	typedef short PxI16;
	typedef unsigned short PxU16;
	typedef int PxI32;
	typedef unsigned int PxU32;
	typedef unsigned __int64 PxU64;

	typedef float PxdF32;
	typedef double PxdF64;
#elif defined(PXD_PLATFORM_PPC) && defined(PXD_COMPILER_VC)
	typedef char PxI8;
	typedef unsigned char PxU8;
	typedef short PxI16;
	typedef unsigned short PxU16;
	typedef int PxI32;
	typedef unsigned int PxU32;
	typedef unsigned __int64 PxU64;

	typedef float PxdF32;
	typedef double PxdF64;
#elif defined(PXD_PLATFORM_CELL) && defined(PXD_COMPILER_GNUC)
	typedef char PxI8;
	typedef unsigned char PxU8;
	typedef short PxI16;
	typedef unsigned short PxU16;
	typedef int PxI32;
	typedef unsigned int PxU32;
	typedef unsigned long long PxU64;

	typedef float PxdF32;
	typedef double PxdF64;
#elif defined(PXD_PLATFORM_X86) && defined(PXD_COMPILER_GNUC)
	typedef char PxI8;
	typedef unsigned char PxU8;
	typedef short PxI16;
	typedef unsigned short PxU16;
	typedef int PxI32;
	typedef unsigned int PxU32;
	typedef unsigned long long PxU64;

	typedef float PxdF32;
	typedef double PxdF64;
#else
	#error "Define platform specific types for your platform"
#endif

//! Generic opaque handle
typedef PxU32 PxdHandle;

#pragma PXD_PUSH_PACK(1)

// Generic math types

/*!
Basic vector in R3.
*/
typedef struct PxdVector_
{
	PxFloat x, y, z;
} PxdVector;

/*!
Combination of two vectors in R3
*/
typedef struct PxdSpatialVector_
{
	PxdVector linear, angular;
} PxdSpatialVector;

/*!
Quaternion.
Defined as q = w + ix + jy + kz = [w (x,y,z)] 
*/
typedef struct PxdQuaternion_
{
	PxFloat x, y, z, w;
} PxdQuaternion;

/*!
Transform
Describes the transition from one coordinate system to another.
*/
typedef struct PxdTransform_
{
	PxdVector translation;
	PxdQuaternion rotation;
} PxdTransform;

#pragma PXD_POP_PACK


#ifdef __cplusplus
}
#endif

#endif
