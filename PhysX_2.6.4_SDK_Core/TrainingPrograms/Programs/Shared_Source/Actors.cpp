// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						 	   ACTOR CREATION METHODS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "Actors.h"

extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;

// GEOMETRIC SHAPE ACTORS: PLANE, BOX, SPHERE, AND CAPSULE
NxActor* CreateGroundPlane()
{
	// Create a plane with default descriptor
	NxPlaneShapeDesc planeDesc;
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);
	return gScene->createActor(actorDesc);
}

NxActor* CreateBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(boxDim.x,boxDim.y,boxDim.z);
	boxDesc.localPose.t = NxVec3(0,boxDim.y,0);
	actorDesc.shapes.pushBack(&boxDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateSphere(const NxVec3& pos, const NxReal radius, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a sphere
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.localPose.t = NxVec3(0,radius,0);
	actorDesc.shapes.pushBack(&sphereDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateCapsule(const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a capsule
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.height = height;
	capsuleDesc.radius = radius;
	capsuleDesc.localPose.t = NxVec3(0,radius+(NxReal)0.5*height,0);
	actorDesc.shapes.pushBack(&capsuleDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

// CONVEX MESH SHAPE ACTORS: HALF-PYRAMID, PYRAMID, DOWN WEDGE
NxActor* CreateHalfPyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Half-Pyramid
	NxVec3 verts[8] = {	NxVec3(boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,boxDim.z), NxVec3(boxDim.x,-boxDim.y,boxDim.z),
		NxVec3(boxDim.x*(NxReal)0.5,boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.5,boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.5,boxDim.y,boxDim.z*(NxReal)0.5), NxVec3(boxDim.x*(NxReal)0.5,boxDim.y,boxDim.z*(NxReal)0.5) };

	// Create descriptor for convex mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 8;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}

	if (convexShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreatePyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Pyramid
	NxVec3 verts[5] = {	NxVec3(boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,boxDim.z), NxVec3(boxDim.x,-boxDim.y,boxDim.z),
		NxVec3(0,boxDim.y,0) };

	// Create descriptor for convex mesh
	NxConvexMeshDesc* convexDesc = new NxConvexMeshDesc();
	convexDesc->numVertices			= 5;
	convexDesc->pointStrideBytes	= sizeof(NxVec3);
	convexDesc->points				= verts;
	convexDesc->flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);
	convexShapeDesc.userData = convexDesc;

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(*convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(*convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}

	if (convexShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseTriangleMesh(*convexShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateDownWedge(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Down wedge
	NxVec3 verts[6] = {	NxVec3(boxDim.x,boxDim.y,-boxDim.z), NxVec3(boxDim.x,boxDim.y,boxDim.z), NxVec3(boxDim.x,-boxDim.y,0),
		NxVec3(-boxDim.x,boxDim.y,-boxDim.z), NxVec3(-boxDim.x,boxDim.y,boxDim.z), NxVec3(-boxDim.x,-boxDim.y,0) };

	// Create descriptor for convex mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 6;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}

	if (convexShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateFlatSurface(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride)
{
	NxVec3* fsVerts = NULL;
	NxU32* fsFaces = NULL;
	NxVec3* fsNormals = NULL;

	// Initialize flat surface verts
	NxU32 nbVerts = (length+1)*(width+1);
	fsVerts = new NxVec3[nbVerts];

	// Build flat surface
	for (NxU32 i = 0; i < width + 1; i++)
	{
		for (NxU32 j = 0; j < length + 1; j++)
		{
			fsVerts[i+j*(width+1)] = NxVec3(stride*NxReal(i),0,stride*NxReal(j)) - NxVec3(0.5f*stride*NxReal(width),0,0.5f*stride*NxReal(length)); 
		}
	}

	// Initialize flat surface faces
	NxU32 nbFaces = length*width*2;
	fsFaces = new NxU32[nbFaces*3];

	NxU32 k = 0;
	for (NxU32 i = 0; i < width; i++)
	{
		for (NxU32 j = 0; j < length; j++)
		{
			// Create first triangle
			fsFaces[k] = i + j*(width+1);
			fsFaces[k+1] = i + (j+1)*(width+1);
			fsFaces[k+2] = i+1 + (j+1)*(width+1);
			k+=3;

			// Create second triangle
			fsFaces[k] = i + j*(width+1);
			fsFaces[k+1] = i+1 + (j+1)*(width+1);
			fsFaces[k+2] = i+1 + j*(width+1);
			k+=3;
		}
	}

	// allocate flat surface materials -- one for each face
	//	fsMaterials = new NxMaterialIndex[nbFaces];

	// Build vertex normals
	fsNormals = new NxVec3[nbFaces];
	//	NxBuildSmoothNormals(nbFaces, nbVerts, fsVerts, fsFaces, NULL, fsNormals, true);

	NxTriangleMeshDesc fsDesc;
	fsDesc.numVertices				= nbVerts;
	fsDesc.numTriangles				= nbFaces;
	fsDesc.pointStrideBytes			= sizeof(NxVec3);
	fsDesc.triangleStrideBytes		= 3*sizeof(NxU32);
	fsDesc.points					= fsVerts;
	fsDesc.triangles				= fsFaces;							
	fsDesc.flags					= NX_MF_HARDWARE_MESH;

	// Add the mesh material data
	//	fsDesc.materialIndexStride			= sizeof(NxMaterialIndex);
	//	fsDesc.materialIndices				= fsMaterials;

	//	fsDesc.heightFieldVerticalAxis		= NX_Y;
	//	fsDesc.heightFieldVerticalExtent	= 0;

	NxTriangleMeshShapeDesc fsShapeDesc;

	NxInitCooking();
	if (0)
	{
#ifdef WIN32
		// Cooking from file
		bool status = NxCookTriangleMesh(fsDesc, UserStream("c:\\tmp.bin", false));
		fsShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookTriangleMesh(fsDesc, buf);
		fsShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
	}

	if (fsShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&fsShapeDesc);
		actorDesc.globalPose.t = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseTriangleMesh(*fsShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateFlatHeightfield(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride)
{
	NxVec3* hfVerts = NULL;
	NxU32* hfFaces = NULL;
	NxVec3* hfNormals = NULL;

	// Initialize heightfield verts
	NxU32 nbVerts = (length+1)*(width+1);
	hfVerts = new NxVec3[nbVerts];

	// Build flat heightfield
	for (NxU32 i = 0; i < width + 1; i++)
	{
		for (NxU32 j = 0; j < length + 1; j++)
		{
			hfVerts[i+j*(width+1)] = NxVec3(stride*NxReal(i),0,stride*NxReal(j)) - NxVec3(0.5f*stride*NxReal(width),0,0.5f*stride*NxReal(length));
		}
	}

	// Initialize heightfield faces
	NxU32 nbFaces = length*width*2;
	hfFaces = new NxU32[nbFaces*3];

	NxU32 k = 0;
	for (NxU32 i = 0; i < width; i++)
	{
		for (NxU32 j = 0; j < length; j++)
		{
			// Create first triangle
			hfFaces[k] = i + j*(width+1);
			hfFaces[k+1] = i + (j+1)*(width+1);
			hfFaces[k+2] = i+1 + (j+1)*(width+1);
			k+=3;

			// Create second triangle
			hfFaces[k] = i + j*(width+1);
			hfFaces[k+1] = i+1 + (j+1)*(width+1);
			hfFaces[k+2] = i+1 + j*(width+1);
			k+=3;
		}
	}

	// allocate heightfield materials -- one for each face
	//	hfMaterials = new NxMaterialIndex[nbFaces];

	// Build vertex normals
	hfNormals = new NxVec3[nbFaces];
	//	NxBuildSmoothNormals(nbFaces, nbVerts, hfVerts, hfFaces, NULL, hfNormals, true);

	NxTriangleMeshDesc hfDesc;
	hfDesc.numVertices				= nbVerts;
	hfDesc.numTriangles				= nbFaces;
	hfDesc.pointStrideBytes			= sizeof(NxVec3);
	hfDesc.triangleStrideBytes		= 3*sizeof(NxU32);
	hfDesc.points					= hfVerts;
	hfDesc.triangles				= hfFaces;							
	hfDesc.flags					= 0;

	// Add the mesh material data
	//	hfDesc.materialIndexStride			= sizeof(NxMaterialIndex);
	//	hfDesc.materialIndices				= hfMaterials;

	hfDesc.heightFieldVerticalAxis		= NX_Y;
	hfDesc.heightFieldVerticalExtent	= 0;

	NxTriangleMeshShapeDesc hfShapeDesc;

	NxInitCooking();
	if (0)
	{
#ifdef WIN32
		// Cooking from file
		bool status = NxCookTriangleMesh(hfDesc, UserStream("c:\\tmp.bin", false));
		hfShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookTriangleMesh(hfDesc, buf);
		hfShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
	}

	if (hfShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&hfShapeDesc);
		actorDesc.globalPose.t = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseTriangleMesh(*hfShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateConvexObjectComputeHull(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Compute hull
	NxVec3 verts[8] = 
	{	
		NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), 
			NxVec3(boxDim.x,-boxDim.y,-boxDim.z), 
			NxVec3(-boxDim.x,boxDim.y,-boxDim.z), 
			NxVec3(boxDim.x,boxDim.y,-boxDim.z),
			NxVec3(-boxDim.x,-boxDim.y,boxDim.z), 
			NxVec3(boxDim.x,-boxDim.y,boxDim.z), 
			NxVec3(-boxDim.x,boxDim.y,boxDim.z), 
			NxVec3(boxDim.x,boxDim.y,boxDim.z) 
	};

	// Create descriptor for convex mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 8;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}

	if (convexShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateConvexObjectSupplyHull(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Supply hull
	NxVec3 verts[8] = 
	{	
		NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), 
			NxVec3(boxDim.x,-boxDim.y,-boxDim.z), 
			NxVec3(-boxDim.x,boxDim.y,-boxDim.z), 
			NxVec3(boxDim.x,boxDim.y,-boxDim.z),
			NxVec3(-boxDim.x,-boxDim.y,boxDim.z), 
			NxVec3(boxDim.x,-boxDim.y,boxDim.z), 
			NxVec3(-boxDim.x,boxDim.y,boxDim.z), 
			NxVec3(boxDim.x,boxDim.y,boxDim.z) 
	};

	NxU32 indices[12*3] =
	{          
		1,2,3,        
			0,2,1,    
			5,7,6,    
			4,5,6,    
			5,4,1,    
			1,4,0,    
			1,3,5,    
			3,7,5,    
			3,2,7,    
			2,6,7,    
			2,0,6,    
			4,6,0
	};

	// Create descriptor for triangle mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 8;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.numTriangles			= 12;
	convexDesc.triangles			= indices;
	convexDesc.triangleStrideBytes	= 3 * sizeof(NxU32);
	convexDesc.flags				= 0;

	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}

	if (convexShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateConcaveObject(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Supply hull
	NxVec3 verts[8] = 
	{	
		NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), 
			NxVec3(boxDim.x,-boxDim.y,-boxDim.z), 
			NxVec3(-boxDim.x,boxDim.y,-boxDim.z), 
			NxVec3(boxDim.x,boxDim.y,-boxDim.z),
			NxVec3(-boxDim.x,-boxDim.y,boxDim.z), 
			NxVec3(boxDim.x,-boxDim.y,boxDim.z), 
			NxVec3(-boxDim.x,boxDim.y,boxDim.z), 
			NxVec3(boxDim.x,boxDim.y,boxDim.z) 
	};

	NxU32 indices[12*3] =
	{          
		1,2,3,        
			0,2,1,    
			5,7,6,    
			4,5,6,    
			5,4,1,    
			1,4,0,    
			1,3,5,    
			3,7,5,    
			3,2,7,    
			2,6,7,    
			2,0,6,    
			4,6,0
	};

	// Create descriptor for convex mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 8;
	convexDesc.pointStrideBytes	= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.numTriangles		    = 12;
	convexDesc.triangles			= indices;
	convexDesc.triangleStrideBytes	= 3 * sizeof(NxU32);
	convexDesc.flags				= 0;

	// Create descriptor for convex mesh shape #1
	NxConvexShapeDesc convexShapeDesc1;
	convexShapeDesc1.localPose.t    = NxVec3(0,boxDim.y,0);

	// Create descriptor for convex mesh shape #2
	NxConvexShapeDesc convexShapeDesc2;
	convexShapeDesc2.localPose.t    = NxVec3(0,boxDim.y,2*boxDim.z);

	// Create descriptor for convex mesh shape #3
	NxConvexShapeDesc convexShapeDesc3;
	convexShapeDesc3.localPose.t    = NxVec3(2*boxDim.x,boxDim.y,2*boxDim.z);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc1.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
		convexShapeDesc2.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
		convexShapeDesc3.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc1.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
		convexShapeDesc2.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
		convexShapeDesc3.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}

	if (convexShapeDesc1.meshData && convexShapeDesc2.meshData && convexShapeDesc3.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc1);
		actorDesc.shapes.pushBack(&convexShapeDesc2);
		actorDesc.shapes.pushBack(&convexShapeDesc3);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
		//      gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);
	}

	return NULL;
}

NxActor* CreateBunny(const NxVec3& pos, NxTriangleMesh* triangleMesh, const NxReal density)
{
	NxBodyDesc bodyDesc;

	//create triangle mesh instance
	NxTriangleMeshShapeDesc bunnyShapeDesc;
	bunnyShapeDesc.meshData	= triangleMesh;
	bunnyShapeDesc.localPose.t = NxVec3(0,1,0);

	if (bunnyShapeDesc.meshData)
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&bunnyShapeDesc);
		if (density)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		else
		{
			actorDesc.body = NULL;
		}
		actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
	}

	return NULL;
}

NxActor** CreateStack(const NxVec3& pos, const NxVec3& stackDim, const NxVec3& boxDim, NxReal density)
{
	NxActor** stack = new NxActor*[(int)(8*stackDim.x*stackDim.y*stackDim.z)]; 

	NxVec3 offset = NxVec3(boxDim.x,0,boxDim.z) + pos;

	int count = 0;
	for (int i = -(int)stackDim.x; i < (int)stackDim.x; i++)
	{
		for (int j = 0; j < (int)stackDim.y; j++)
		{
			for (int k = -(int)stackDim.z; k < (int)stackDim.z; k++)
			{
				NxVec3 boxPos = NxVec3(i*boxDim.x*2,j*boxDim.y*2,k*boxDim.z*2);
				stack[count++] = CreateBox(boxPos + offset, boxDim, density);
			}
		}
	}

	return stack;
}

NxActor** CreateTower(const NxVec3& pos, const NxU32 heightBoxes, const NxVec3& boxDim, NxReal density)
{
	NxActor** tower = new NxActor*[heightBoxes]; 

	NxReal spacing = 0.01f;

	NxVec3 boxPos = pos;

	for (NxU32 i = 0; i < heightBoxes; i++)
	{
		tower[i] = CreateBox(boxPos, boxDim, density);
		boxPos.y += 2*boxDim.y + spacing;
	}

	return tower;	 
}

NxQuat AnglesToQuat(const NxVec3& angles)
{
	NxQuat quat;
	NxVec3 a;
	NxReal cr, cp, cy, sr, sp, sy, cpcy, spsy;

	a.x = (NxPi/(NxReal)360.0) * angles.x;    // Pitch
	a.y = (NxPi/(NxReal)360.0) * angles.y;    // Yaw
	a.z = (NxPi/(NxReal)360.0) * angles.z;    // Roll

	cy = NxMath::cos(a.z);
	cp = NxMath::cos(a.y);
	cr = NxMath::cos(a.x);

	sy = NxMath::sin(a.z);
	sp = NxMath::sin(a.y);
	sr = NxMath::sin(a.x);

	cpcy = cp * cy;
	spsy = sp * sy;
	quat.w = cr * cpcy + sr * spsy;		
	quat.x = sr * cpcy - cr * spsy;		
	quat.y = cr * sp * cy + sr * cp * sy;	
	quat.z = cr * cp * sy - sr * sp * cy;

	return quat;
}

NxActor* CreateBoxGear(const NxVec3& pos, const NxReal minRadius, const NxReal maxRadius, const NxReal height, const NxU32 numTeeth, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	bodyDesc.solverIterationCount = 30;  // Fast-moving body, higher solver iteration count

	NxReal perimeter = 2*NxPi*minRadius;
	NxReal toothWidth = (perimeter / (numTeeth*2))*(NxReal)0.75;
	NxReal toothHeight = height;
	NxReal toothDepth = maxRadius - minRadius;
	NxReal a,c,s;
	NxU32 i;

	NxVec3* verts = new NxVec3[numTeeth*2];

	for (i = 0; i < numTeeth; i++) 
	{
		a = 360*(i/(NxReal)numTeeth);
		c = NxMath::cos(a*(NxPi/180))*minRadius;
		s = NxMath::sin(a*(NxPi/180))*minRadius;
		verts[2*i] = NxVec3(s, -height/2, c);
		verts[2*i+1] = NxVec3(s, height/2, c);
	}

	// Create descriptor for gear center convex mesh
	NxConvexMeshDesc centerDesc;
	centerDesc.numVertices			= numTeeth*2;
	centerDesc.pointStrideBytes		= sizeof(NxVec3);
	centerDesc.points				= verts;
	centerDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc centerShapeDesc;
	centerShapeDesc.localPose.t = NxVec3(0,0,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(centerDesc, UserStream("c:\\tmp.bin", false));
		centerShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(centerDesc, buf);
		centerShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&centerShapeDesc);

	NxBoxShapeDesc *boxDesc = new NxBoxShapeDesc[numTeeth];

	for (i = 0; i < numTeeth; i++) 
	{
		a = 360*(i/(NxReal)numTeeth);
		c = NxMath::cos(a*(NxPi/180))*(minRadius+toothDepth/2);
		s = NxMath::sin(a*(NxPi/180))*(minRadius+toothDepth/2);

		boxDesc[i].dimensions.set(NxVec3(toothWidth/2,toothHeight/2,toothDepth/2));

		NxQuat q = AnglesToQuat(NxVec3(0,a,0));
		boxDesc[i].localPose.M.fromQuat(q);
		boxDesc[i].localPose.t = NxVec3(s,0,c);

		actorDesc.shapes.pushBack(&boxDesc[i]);
	}

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	NxActor* actor = gScene->createActor(actorDesc);

	return actor;
}

NxActor* CreateWheel(const NxVec3& pos, const NxReal minRadius, const NxReal maxRadius, const NxReal height, const NxU32 numTeeth, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxReal perimeter = 2*NxPi*minRadius;
	NxReal toothWidth = (perimeter / (numTeeth*2))*(NxReal)0.75;
	NxReal toothHeight = height;
	NxReal toothDepth = maxRadius - minRadius;
	NxReal a,c,s;
	NxU32 i;

	// WHEEL CENTER
	NxVec3* verts = new NxVec3[numTeeth*2];

	for (i = 0; i < numTeeth; i++) 
	{
		a = 360*(i/(NxReal)numTeeth);
		c = NxMath::cos(a*(NxPi/180))*minRadius;
		s = NxMath::sin(a*(NxPi/180))*minRadius;
		verts[2*i] = NxVec3(s, -height/2, c);
		verts[2*i+1] = NxVec3(s, height/2, c);
	}

	// Create descriptor for gear center triangle mesh
	NxConvexMeshDesc centerDesc;
	centerDesc.numVertices			= numTeeth*2;
	centerDesc.pointStrideBytes		= sizeof(NxVec3);
	centerDesc.points				= verts;
	centerDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc centerShapeDesc;
	centerShapeDesc.localPose.t = NxVec3(0,0,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(centerDesc, UserStream("c:\\tmp.bin", false));
		centerShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(centerDesc, buf);
		centerShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&centerShapeDesc);

	// WHEEL LEFT EDGE
	for (i = 0; i < numTeeth; i++) 
	{
		a = 360*(i/(NxReal)numTeeth);
		c = NxMath::cos(a*(NxPi/(NxReal)180))*maxRadius;
		s = NxMath::sin(a*(NxPi/(NxReal)180))*maxRadius;
		verts[2*i] = NxVec3(s, -(NxReal)0.1, c);
		verts[2*i+1] = NxVec3(s, (NxReal)0.1, c);
	}

	// Create descriptor for gear left edge convex mesh
	NxConvexMeshDesc leftEdgeDesc;
	leftEdgeDesc.numVertices		= numTeeth*2;
	leftEdgeDesc.pointStrideBytes	= sizeof(NxVec3);
	leftEdgeDesc.points				= verts;
	leftEdgeDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc leftEdgeShapeDesc;
	leftEdgeShapeDesc.localPose.t = NxVec3(0,-height/2-(NxReal)0.1,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(leftEdgeDesc, UserStream("c:\\tmp.bin", false));
		leftEdgeShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(leftEdgeDesc, buf);
		leftEdgeShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&leftEdgeShapeDesc);

	// WHEEL RIGHT EDGE
	for (i = 0; i < numTeeth; i++) 
	{
		a = 360*(i/(NxReal)numTeeth);
		c = NxMath::cos(a*(NxPi/180))*maxRadius;
		s = NxMath::sin(a*(NxPi/180))*maxRadius;
		verts[2*i] = NxVec3(s, -(NxReal)0.1, c);
		verts[2*i+1] = NxVec3(s, (NxReal)0.1, c);
	}

	// Create descriptor for gear left edge triangle mesh
	NxConvexMeshDesc rightEdgeDesc;
	rightEdgeDesc.numVertices		= numTeeth*2;
	rightEdgeDesc.pointStrideBytes	= sizeof(NxVec3);
	rightEdgeDesc.points			= verts;
	rightEdgeDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	NxConvexShapeDesc rightEdgeShapeDesc;
	rightEdgeShapeDesc.localPose.t = NxVec3(0,height/2+(NxReal)0.1,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(rightEdgeDesc, UserStream("c:\\tmp.bin", false));
		rightEdgeShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(rightEdgeDesc, buf);
		rightEdgeShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&rightEdgeShapeDesc);

	NxBoxShapeDesc *boxDesc = new NxBoxShapeDesc[numTeeth];

	for (i = 0; i < numTeeth; i++) 
	{
		a = 360*(i/(NxReal)numTeeth);
		c = NxMath::cos(a*(NxPi/180))*(minRadius+toothDepth/2);
		s = NxMath::sin(a*(NxPi/180))*(minRadius+toothDepth/2);

		boxDesc[i].dimensions.set(NxVec3(toothWidth/2,toothHeight/2,toothDepth/2));

		NxQuat q = AnglesToQuat(NxVec3(0,a,0));
		boxDesc[i].localPose.M.fromQuat(q);
		boxDesc[i].localPose.t = NxVec3(s,0,c);

		actorDesc.shapes.pushBack(&boxDesc[i]);
	}

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	NxActor* actor = gScene->createActor(actorDesc);

	return actor;
}

NxActor* CreateFrame(const NxVec3& pos, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxBoxShapeDesc boxDesc1;
	boxDesc1.dimensions.set(8,(NxReal)0.5,(NxReal)0.05);
	boxDesc1.localPose.t = NxVec3(0,0,(NxReal)1.7);
	actorDesc.shapes.pushBack(&boxDesc1);

	NxBoxShapeDesc boxDesc2;
	boxDesc2.dimensions.set(8,(NxReal)0.5,(NxReal)0.05);
	boxDesc2.localPose.t = NxVec3(0,0,-(NxReal)1.7);
	actorDesc.shapes.pushBack(&boxDesc2);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateStep(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxBoxShapeDesc boxDesc1;
	boxDesc1.dimensions.set(boxDim.x/2,boxDim.y/2,boxDim.z/2);
	boxDesc1.localPose.t = NxVec3(0,0,0);
	actorDesc.shapes.pushBack(&boxDesc1);

	NxBoxShapeDesc boxDesc2;
	boxDesc2.dimensions.set(boxDim.x/6,boxDim.y/16,(NxReal)0.125);
	boxDesc2.localPose.t = NxVec3(0,0,-(NxReal)0.1);
	actorDesc.shapes.pushBack(&boxDesc2);

	NxBoxShapeDesc boxDesc3;
	boxDesc3.dimensions.set((NxReal)0.1,boxDim.y/2,(NxReal)0.5);
	boxDesc3.localPose.t = NxVec3(boxDim.x/2,0,(NxReal)(0.201-0.25));
	actorDesc.shapes.pushBack(&boxDesc3);

	NxBoxShapeDesc boxDesc4;
	boxDesc4.dimensions.set((NxReal)0.1,boxDim.y/2,(NxReal)0.5);
	boxDesc4.localPose.t = NxVec3(-boxDim.x/2,0,(NxReal)(0.201-0.25));
	actorDesc.shapes.pushBack(&boxDesc4);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateChassis(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Half-Pyramid
	NxVec3 verts[16] = {NxVec3(0,4*boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.75,4*boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.75,4*boxDim.y,boxDim.z*(NxReal)0.5), NxVec3(0,4*boxDim.y,boxDim.z*(NxReal)0.5),
		NxVec3(boxDim.x,2*boxDim.y,-boxDim.z), NxVec3(-boxDim.x,2*boxDim.y,-boxDim.z), NxVec3(-boxDim.x,2*boxDim.y,boxDim.z), NxVec3(boxDim.x,2*boxDim.y,boxDim.z),
		NxVec3(boxDim.x,boxDim.y,-boxDim.z), NxVec3(-boxDim.x,boxDim.y,-boxDim.z), NxVec3(-boxDim.x,boxDim.y,boxDim.z), NxVec3(boxDim.x,boxDim.y,boxDim.z),
		NxVec3(boxDim.x*(NxReal)0.75,-boxDim.y,-boxDim.z*(NxReal)0.75), NxVec3(-boxDim.x*(NxReal)0.75,-boxDim.y,-boxDim.z*(NxReal)0.75), NxVec3(-boxDim.x*(NxReal)0.75,-boxDim.y,boxDim.z*(NxReal)0.75), NxVec3(boxDim.x*(NxReal)0.75,-boxDim.y,boxDim.z*(NxReal)0.75)
	};

	// Create descriptor for triangle mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 16;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	// The actor has one shape, a convex mesh
	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t = NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&convexShapeDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	NxActor* actor = gScene->createActor(actorDesc);	

	return actor;
}

NxActor* CreateTurret(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Half-Pyramid
	NxVec3 verts[16] = {
		NxVec3(boxDim.x*(NxReal)0.25,6*boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.75,6*boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.75,6*boxDim.y,boxDim.z*(NxReal)0.5), NxVec3(boxDim.x*(NxReal)0.25,6*boxDim.y,boxDim.z*(NxReal)0.5),
			NxVec3(boxDim.x*(NxReal)0.5,(NxReal)5.5*boxDim.y,-boxDim.z*(NxReal)0.65), NxVec3(-boxDim.x*(NxReal)0.8,(NxReal)5.5*boxDim.y,-boxDim.z*(NxReal)0.65), NxVec3(-boxDim.x*(NxReal)0.8,(NxReal)5.5*boxDim.y,boxDim.z*(NxReal)0.65), NxVec3(boxDim.x*(NxReal)0.5,(NxReal)5.5*boxDim.y,boxDim.z*(NxReal)0.65),
			NxVec3(boxDim.x*(NxReal)0.5,(NxReal)4.5*boxDim.y,-boxDim.z*(NxReal)0.65), NxVec3(-boxDim.x*(NxReal)0.8,(NxReal)4.5*boxDim.y,-boxDim.z*(NxReal)0.65), NxVec3(-boxDim.x*(NxReal)0.8,(NxReal)4.5*boxDim.y,boxDim.z*(NxReal)0.65), NxVec3(boxDim.x*(NxReal)0.5,(NxReal)4.5*boxDim.y,boxDim.z*(NxReal)0.65),
			NxVec3(boxDim.x*(NxReal)0.25,4*boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.75,4*boxDim.y,-boxDim.z*(NxReal)0.5), NxVec3(-boxDim.x*(NxReal)0.75,4*boxDim.y,boxDim.z*(NxReal)0.5), NxVec3(boxDim.x*(NxReal)0.25,4*boxDim.y,boxDim.z*(NxReal)0.5),
	};

	// Create descriptor for convex mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 16;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	// The actor has one shape, a convex mesh
	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t = NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&convexShapeDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	NxActor* actor = gScene->createActor(actorDesc);	

	return actor;
}

NxActor* CreateCannon(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Half-Pyramid
	NxVec3 verts[8] = {
		NxVec3(boxDim.x*2,(NxReal)5.4*boxDim.y,-boxDim.z*(NxReal)0.1), NxVec3(boxDim.x*(NxReal)0.5,(NxReal)5.4*boxDim.y,-boxDim.z*(NxReal)0.1), NxVec3(boxDim.x*(NxReal)0.5,(NxReal)5.4*boxDim.y,boxDim.z*(NxReal)0.1), NxVec3(boxDim.x*2,(NxReal)5.4*boxDim.y,boxDim.z*(NxReal)0.1),
			NxVec3(boxDim.x*2,(NxReal)4.6*boxDim.y,-boxDim.z*(NxReal)0.1), NxVec3(boxDim.x*(NxReal)0.5,(NxReal)4.6*boxDim.y,-boxDim.z*(NxReal)0.1), NxVec3(boxDim.x*(NxReal)0.5,(NxReal)4.6*boxDim.y,boxDim.z*(NxReal)0.1), NxVec3(boxDim.x*2,(NxReal)4.6*boxDim.y,boxDim.z*(NxReal)0.1),
	};

	// Create descriptor for triangle mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= 8;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

	// The actor has one shape, a convex mesh
	NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t = NxVec3(0,boxDim.y,0);

	NxInitCooking();
	if (0)
	{
		// Cooking from file
#ifdef WIN32
		bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
	}
	else
	{
		// Cooking from memory
		MemoryWriteBuffer buf;
		bool status = NxCookConvexMesh(convexDesc, buf);
		convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
	}
	actorDesc.shapes.pushBack(&convexShapeDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	NxActor* actor = gScene->createActor(actorDesc);	

	return actor;
}

NxActor* CreateBlade(const NxVec3& pos, const NxVec3& boxDim, const NxReal mass)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(boxDim.x,boxDim.y,boxDim.z);
	boxDesc.localPose.t = NxVec3(0,boxDim.y,0);
	actorDesc.shapes.pushBack(&boxDesc);

	if (mass)
	{
		bodyDesc.mass = mass;
		actorDesc.body = &bodyDesc;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateBall(const NxVec3& pos, const NxReal radius, const NxReal mass)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a sphere
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.localPose.t = NxVec3(0,radius,0);
	actorDesc.shapes.pushBack(&sphereDesc);

	if (mass)
	{
		bodyDesc.mass = mass;
		actorDesc.body = &bodyDesc;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

void SetActorCollisionGroup(NxActor* actor, NxCollisionGroup group)
{
	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		shapes[nShapes]->setGroup(group);
	}
}

void SetActorMaterial(NxActor *actor, NxMaterialIndex index)
{
	NxU32 nbShapes = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();

	while (nbShapes--)
	{
		shapes[nbShapes]->setMaterial(index);
	}
}

void PageInHardwareMeshes(NxScene* scene)
{
	// Page in the hardware meshes
	for (NxU32 i = 0; i < scene->getNbActors(); i++) 
	{
		NxActor* actor = scene->getActors()[i];
		NxShape*const* shapes = actor->getShapes();
		for (NxU32 s = 0; s < actor->getNbShapes(); ++s )
		{
			NxShape* shape = (NxShape*)(shapes[s]);
			if(shape->is(NX_SHAPE_MESH))
			{
				NxTriangleMeshShape* triMeshShape = (NxTriangleMeshShape *)shape;
				NxTriangleMesh& triMesh	= triMeshShape->getTriangleMesh();
				NxU32 pageCnt = triMesh.getPageCount();
				for ( NxU32 t = 0; t < pageCnt; ++t )
				{
					if (!triMeshShape->isPageInstanceMapped(t))
					{
						bool ok = triMeshShape->mapPageInstance(t);
						assert(ok);
					}
				}
			}
		}	
	}
}

void SwitchCoordinateSystem(NxScene* scene, NxMat34 mat)
{
	NxU32 nbActors = scene->getNbActors();
	NxActor** actors = scene->getActors();

	NxMat33 roll2;
	roll2.rotX(-NxPi*0.5f);

	NxMat34 invMat;
	mat.getInverse(invMat);

	bool bStaticActor;
	// Adjust actors' positions and orientations
	for (NxU32 i = 0; i < nbActors; i++)
	{
		NxActor* actor = actors[i];

		NxVec3 pos = actor->getGlobalPosition();
		pos = mat*pos;

		bStaticActor=false;
		if(!actor->isDynamic())
		{
			bStaticActor = true;
		}
		if(!bStaticActor)
			actor->setGlobalPosition(pos);

		// Adjust shape's positions and orientations within actors
		NxU32 nbShapes = actor->getNbShapes();
		NxShape*const* shape = actor->getShapes();

		NxActorDesc newActorDesc;

		if(bStaticActor)
		{
			actor->saveToDesc(newActorDesc);
		}
		for (NxU32 j = 0; j < nbShapes; j++)
		{
			// Transform the local position of each shape
			NxVec3 pos = shape[j]->getLocalPosition();
			pos = mat*pos;
			shape[j]->setLocalPosition(pos);
			if (NxPlaneShape* planeShape = shape[j]->isPlane())
			{
				NxPlaneShapeDesc planeDesc;
				planeShape->saveToDesc(planeDesc);
				NxReal y = planeDesc.normal.y;
				planeDesc.normal.y = planeDesc.normal.z;
				planeDesc.normal.z = y;

				if(bStaticActor)
				{
					newActorDesc.shapes.push_back(&planeDesc);
				}
				planeShape->setPlane(planeDesc.normal, planeDesc.d);
			}
			if (NxBoxShape* boxShape = shape[j]->isBox())
			{
				NxBoxShapeDesc boxDesc;
				boxShape->saveToDesc(boxDesc);
				NxReal y = boxDesc.dimensions.y;
				boxDesc.dimensions.y = boxDesc.dimensions.z;
				boxDesc.dimensions.z = y;

				boxShape->setDimensions(boxDesc.dimensions);

				if(bStaticActor)
				{
					newActorDesc.shapes.push_back(&boxDesc);
				}

			}	
			if (NxCapsuleShape* capsuleShape = shape[j]->isCapsule())
			{
				NxMat33 mat = shape[j]->getGlobalOrientation();
				mat = roll2*mat;
				shape[j]->setGlobalOrientation(mat);
			}	
			if (NxConvexShape* convexShape = shape[j]->isConvexMesh())
			{
				// TRANSFORM AND RE-COOK THE CONVEX MESH DATA
				NxConvexShapeDesc convexShapeDesc;
				convexShape->saveToDesc(convexShapeDesc);

				// Transform the mesh data to the new coordinate system
				NxConvexMeshDesc convexDesc;
				convexShape->getConvexMesh().saveToDesc(convexDesc);

				// TRANSFORM POINTS HERE
				NxVec3* verts = (NxVec3*)convexDesc.points;
				for (NxU32 i = 0; i < convexDesc.numVertices; i++)
				{
					NxVec3 pos = verts[i];
					pos = mat*pos;
					verts[i] = pos;
				}

				// Cook the new mesh data
				NxInitCooking();
				if (0)
				{
					// Cooking from file
#ifdef WIN32
					bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
					convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
				}
				else
				{
					// Cooking from memory
					MemoryWriteBuffer buf;
					bool status = NxCookConvexMesh(convexDesc, buf);
					convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
				}

				if(bStaticActor)
				{
					newActorDesc.shapes.push_back(&convexShapeDesc);
				}
				else
				{
					actor->createShape(convexShapeDesc);

					NxActorDesc actorDesc;
					actor->saveToDesc(actorDesc);
					actorDesc.shapes.pushBack(&convexShapeDesc);

					shape[j]->userData = (void*)-1;  // Schedule for deletion
				}

			}
			if (NxTriangleMeshShape* meshShape = shape[j]->isTriangleMesh())
			{
				// TRANSFORM AND RE-COOK THE TRIANGLE MESH DATA
				NxTriangleMeshShapeDesc meshShapeDesc;
				meshShape->saveToDesc(meshShapeDesc);

				// Transform the mesh data to the new coordinate system
				NxTriangleMeshDesc meshDesc;
				meshShape->getTriangleMesh().saveToDesc(meshDesc);

				// TRANSFORM POINTS HERE
				NxVec3* verts = (NxVec3*)meshDesc.points;
				for (NxU32 i = 0; i < meshDesc.numVertices; i++)
				{
					NxVec3 pos = verts[i];
					pos = mat*pos;
					verts[i] = pos;
				}

				// Cook the new mesh data
				NxInitCooking();
				if (0)
				{
					// Cooking from file
#ifdef WIN32
					bool status = NxCookTriangleMesh(meshDesc, UserStream("c:\\tmp.bin", false));
					meshShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
#endif
				}
				else
				{
					// Cooking from memory
					MemoryWriteBuffer buf;
					bool status = NxCookTriangleMesh(meshDesc, buf);
					meshShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
				}

				if(bStaticActor)
				{
					newActorDesc.shapes.push_back(&meshShapeDesc);
				}
				else
				{
					actor->createShape(meshShapeDesc);

				}

				NxActorDesc actorDesc;
				actor->saveToDesc(actorDesc);
				actorDesc.shapes.pushBack(&meshShapeDesc);

				shape[j]->userData = (void*)-1;  // Schedule for deletion
			}
		}

		if(bStaticActor)
		{
			newActorDesc.globalPose.t=pos;
			scene->createActor(newActorDesc);
			scene->releaseActor(*actor);
		}


		//		actor->updateMassFromShapes(0,actor->getMass());
	}

	nbActors = scene->getNbActors();
	actors = scene->getActors();

	// Remove convex shapes scheduled for deletion
	// Update actor masses
	for (NxU32 i = 0; i < nbActors; i++)
	{
		NxActor* actor = actors[i];

		// Release convex shapes scheduled for deletion
		NxU32 nbShapes = actor->getNbShapes();
		NxShape*const* shape = actor->getShapes();
		for (NxU32 j = 0; j < nbShapes; j++)
		{
			if (shape[j]->userData == (void*)-1)
			{
				if(actor->isDynamic())
					actor->releaseShape(*shape[j]);
				shape[j]->userData = 0;
			}
		}

		if (actor->isDynamic())  actor->updateMassFromShapes(0, actor->getMass());
	}
}