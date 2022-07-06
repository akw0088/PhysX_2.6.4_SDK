#ifndef PX_CONVEXMESH_H
#define PX_CONVEXMESH_H

#include "px_config.h"

struct PxEdgeListData;
struct PxCollisionHullData;
struct PxSupportVertexMapData;

namespace Opcode
{
	class HybridModel;
}

//Data
#pragma PXD_PUSH_PACK(1)

typedef struct PxConvexMeshData_
{
	PxEdgeListData*				edgeListData;
	Opcode::HybridModel*		opcodeModel;
	PxCollisionHullData*		hullData;
	PxSupportVertexMapData*		vertexMapData;
	PxdVector					aabbMin, aabbMax; //local bounding box
} PxConvexMeshData;

#pragma PXD_POP_PACK

#endif
