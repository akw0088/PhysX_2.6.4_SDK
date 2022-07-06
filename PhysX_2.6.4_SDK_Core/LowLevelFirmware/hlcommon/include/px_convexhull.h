#ifndef PX_CONVEXHULL_H
#define PX_CONVEXHULL_H

#include "px_config.h"

enum PxConvexPolygonFlag
{
	PX_CPF_INTERNAL	= (1<<0),
};


struct PxEdge8Data;
struct PxEdgeDescData;

#pragma PXD_PUSH_PACK(1)

// Data definition
struct PxHullTriangleData
{
	PxU32			mRef[3];
};


struct PxHullPolygonData
{
	PxU16			mNbVerts;		//!< Number of vertices/edges in the polygon
	PxU16			mFlags;			//!< Various polygon flags (see ::ConvexPolygonFlag)
	const PxU8*		mVRef8;			//!< Vertex references
	const PxU16*	mERef16;		//!< Edge references
	PxFloat			mPlane[4];		//!< Plane equation for this polygon
	PxFloat			mMin, mMax;
};

struct PxConvexHullData
{
	// Triangle data
	PxU32				mNbHullFaces;			//!< Number of faces in the convex hull
	PxHullTriangleData*	mFaces;					//!< Convex hull faces
	// Reduced hull vertices (else use original cloud)
	PxU32				mNbHullVertices;		//!< Number of vertices in the convex hull
	PxdVector*			mHullVertices;			//!< Convex hull vertices
	PxdVector*			mHullNormals;			//!< Convex hull normals
	PxdVector			mCenter;				//!< Precomputed hull center
	
	// Polygon data
	PxU32				mNbPolygons;			//!< Number of planar polygons composing the hull
	PxHullPolygonData*	mPolygons;				//!< Array of mNbPolygons structures
	PxU8*				mVertexData8;			//!< Vertex indices indexed by hull polygons
	PxU16*				mEdgeData16;			//!< Edge indices indexed by hull polygons
	
	// Edge data
	PxU32				mNbEdges;
	PxEdge8Data*		mEdges8;
	PxdVector*			mEdgeNormals;
	PxEdgeDescData*		mEdgeToTriangles;
	PxU8*				mFacesByEdges8;
};

#pragma PXD_POP_PACK

// Accessors
class PxHullTriangleAC
{
public:
	PXD_INLINE static void			Flip(PxHullTriangleData& data)
	{
		PxU32 tmp = data.mRef[2];
		data.mRef[2] = data.mRef[1];
		data.mRef[1] = tmp;
	}

};



#endif
