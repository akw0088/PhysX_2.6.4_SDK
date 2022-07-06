// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			               LESSON 306: INTERSECTION TESTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson306.h"

#include "NxTriangle.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

// Force globals
extern NxVec3 gForceVec;
extern NxReal gForceStrength;
extern bool bForceMode;

// Simulation globals
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;
NxActor* heightfield = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Intersection test globals
typedef enum 
{
    SPHERE_SHAPE_TEST,
	AABB_SHAPE_TEST,
	PLANES_SHAPE_TEST,
	SPHERE_SHAPE_CHECK,
	AABB_SHAPE_CHECK,
	AABB_TRIANGLE_TEST,
	NUM_INTERSECTION_TESTS
} kIntersectionTestType;

NxU32 gIntersectionTestType = SPHERE_SHAPE_TEST;
const NxU32 gNumIntersectionTestTypes = 6;

// HUD globals
extern HUD hud;
char* gIntersectionTestTypeString[gNumIntersectionTestTypes] = 
{
	"Sphere-Shape Test","AABB-Shape Test",
	"Planes-Shape Test","Sphere-Shape Check",
	"AABB-Shape Check", "AABB-Triangle Test"
};

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
	printf("\n Special:\n --------\n v = Switch Intersection Test Type\n");
}

void DrawIntersectedActor(NxActor *actor, NxActor* selectedActor)
{
	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		DrawShape(shapes[nShapes], true);
	}
	nShapes = actor->getNbShapes();
	if (selectedActor && actor == selectedActor)
	{
		while (nShapes--)
		{
			if (shapes[nShapes]->getFlag(NX_TRIGGER_ENABLE))
			{
				DrawWireShape(shapes[nShapes], NxVec3(0,0,1), true);
			}
			else
			{
			    if (actor->userData && ((ActorUserData *)(actor->userData))->flags & UD_PASSES_INTERSECTION_TEST)
				    DrawWireShape(shapes[nShapes], NxVec3(0.5,0.5,1), true);
			    else
				    DrawWireShape(shapes[nShapes], NxVec3(1,1,1), true);
			}
		}
		if (actor->userData)  ((ActorUserData *)(actor->userData))->flags &= ~UD_PASSES_INTERSECTION_TEST;
	}
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;
        DrawIntersectedActor(actor, gSelectedActor);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, true);
        }
    }
}

void ProcessInputs()
{
    ProcessForceKeys();

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
		if (gScene)  gDebugRenderer.renderData(*gScene->getDebugRenderable());
	}
}

void RenderIntersectedEntities()
{
	switch (gIntersectionTestType)
	{
	    case SPHERE_SHAPE_TEST:
		{
			NxSphere worldSphere(NxVec3(0,0,0), 5);
			NxShapesType shapeType = NX_DYNAMIC_SHAPES;  // NX_STATIC_SHAPES, NX_ALL_SHAPES
			NxU32 nbShapes = gScene->getNbDynamicShapes();  // + gScene->getNbStaticShapes()
	        NxShape** shapes = (NxShape**)NxAlloca(nbShapes*sizeof(NxShape*));
			for (NxU32 i = 0; i < nbShapes; i++)  shapes[i] = NULL;
			NxU32 activeGroups = 0xffffffff;
			NxGroupsMask* groupsMask = NULL;
	        gScene->overlapSphereShapes(worldSphere, shapeType, nbShapes, shapes, &gShapeReport, activeGroups, groupsMask);
			DrawWireSphere(&worldSphere, NxVec3(1,0,0));
			while (nbShapes--)
			{
				if (shapes[nbShapes])  
				{	
					DrawWireShape(shapes[nbShapes], NxVec3(0,0,0), true);
				}
			}
		}
		break;	    
		
		case AABB_SHAPE_TEST:
		{
			NxBounds3 worldBounds;
			worldBounds.set(NxVec3(-5,-5,-5), NxVec3(5,5,5));
			NxShapesType shapeType = NX_DYNAMIC_SHAPES;  // NX_STATIC_SHAPES, NX_ALL_SHAPES
			NxU32 nbShapes = gScene->getNbDynamicShapes();  // + gScene->getNbStaticShapes()
	        NxShape** shapes = (NxShape**)NxAlloca(nbShapes*sizeof(NxShape*));
			for (NxU32 i = 0; i < nbShapes; i++)  shapes[i] = NULL;
			NxU32 activeGroups = 0xffffffff;
			NxGroupsMask* groupsMask = NULL;
	        gScene->overlapAABBShapes(worldBounds, shapeType, nbShapes, shapes, &gShapeReport, activeGroups, groupsMask);
			NxBox box;
			NxMat34 mat;
			NxCreateBox(box, worldBounds, mat);
			DrawWireBox(box, NxVec3(1,0,0));
			while (nbShapes--)
			{
				if (shapes[nbShapes])  
				{	
					DrawWireShape(shapes[nbShapes], NxVec3(0,0,0), true);
				}
			}
		}
		break;
		
		case PLANES_SHAPE_TEST:
		{
			NxU32 nbPlanes = 2;
			NxPlane worldPlanes[2];
			worldPlanes[0].set(NxVec3(-2,0,2), NxVec3(0,0,1));
			worldPlanes[1].set(NxVec3(-2,0,2), NxVec3(1,0,0));
			NxShapesType shapeType = NX_DYNAMIC_SHAPES;  // NX_STATIC_SHAPES, NX_ALL_SHAPES
			NxU32 nbShapes = gScene->getNbDynamicShapes();  // + gScene->getNbStaticShapes()
	        NxShape** shapes = (NxShape**)NxAlloca(nbShapes*sizeof(NxShape*));
			for (NxU32 i = 0; i < nbShapes; i++)  shapes[i] = NULL;
			NxU32 activeGroups = 0xffffffff;
			NxGroupsMask* groupsMask = NULL;
	        gScene->cullShapes(nbPlanes, worldPlanes, shapeType, nbShapes, shapes, &gShapeReport, activeGroups, groupsMask);
//			NxBox box;
//			NxMat34 mat;
//			NxCreateBox(box, worldBounds, mat);
//			DrawWireBox(&box, NxVec3(1,0,0));
			while (nbShapes--)
			{
				if (shapes[nbShapes])  
				{	
					DrawWireShape(shapes[nbShapes], NxVec3(0,0,0), true);
				}
			}
		}
		break;

	    case SPHERE_SHAPE_CHECK:
		{
			NxSphere worldSphere(NxVec3(0,0,0), 5);
			NxShapesType shapeType = NX_DYNAMIC_SHAPES;  // NX_STATIC_SHAPES, NX_ALL_SHAPES
			NxU32 activeGroups = 0xffffffff;
			NxGroupsMask* groupsMask = NULL;
	        bool intersection = gScene->checkOverlapSphere(worldSphere, shapeType, activeGroups, groupsMask);
			if (intersection)
			    DrawWireSphere(&worldSphere, NxVec3(1,0,0));
			else
			    DrawWireSphere(&worldSphere, NxVec3(0,1,0));
		}
		break;	    
		
		case AABB_SHAPE_CHECK:
		{
			NxBounds3 worldBounds;
			worldBounds.set(NxVec3(-5,-5,-5), NxVec3(5,5,5));
			NxShapesType shapeType = NX_DYNAMIC_SHAPES;  // NX_STATIC_SHAPES, NX_ALL_SHAPES
			NxU32 activeGroups = 0xffffffff;
			NxGroupsMask* groupsMask = NULL;
	        gScene->checkOverlapAABB(worldBounds, shapeType, activeGroups, groupsMask);
			NxBox box;
			NxMat34 mat;
			NxCreateBox(box, worldBounds, mat);
	        bool intersection = gScene->checkOverlapAABB(worldBounds, shapeType, activeGroups, groupsMask);
			if (intersection)
			    DrawWireBox(box, NxVec3(1,0,0));
			else
			    DrawWireBox(box, NxVec3(0,1,0));
		}
		break;	

		case AABB_TRIANGLE_TEST:
		{
			NxBounds3 worldBounds;
			worldBounds.set(NxVec3(-13,-2,-2), NxVec3(-17,2,2));
//			worldBounds.set(NxVec3(7,-2,3), NxVec3(3,2,7));

			NxShapesType shapeType = NX_ALL_SHAPES;  // NX_DYNAMIC_SHAPES, NX_STATIC_SHAPES 
			NxU32 activeGroups = 0xffffffff;
			NxGroupsMask* groupsMask = NULL;

			NxShape*const* shapes = heightfield->getShapes();
			NxTriangleMeshShape* tmShape = shapes[0]->isTriangleMesh();

		    if (!tmShape)  break;

			NxU32 nbTriangles;
			const NxU32* triangles;

		    // Collide AABB against mesh shape
		    if (!tmShape->overlapAABBTriangles((const NxBounds3&)worldBounds, NX_QUERY_WORLD_SPACE, nbTriangles, triangles))
			    break;

		    // Loop through triangles
		    while (nbTriangles--)
			{
			    NxU32 index = *triangles++;

				NxTriangle currentTriangle;

			    NxTriangle edgeTri;
			    NxU32 edgeFlags;
			    tmShape->getTriangle(currentTriangle, &edgeTri, &edgeFlags, index);

			    NxMat34 shapePose = tmShape->getGlobalPose();

				NxVec3 pos[3];

				pos[0] = currentTriangle.verts[0];
				pos[1] = currentTriangle.verts[1];
				pos[2] = currentTriangle.verts[2];

				DrawLine(pos[0], pos[1], NxVec3(0,1,0));
				DrawLine(pos[1], pos[2], NxVec3(0,1,0));
				DrawLine(pos[2], pos[0], NxVec3(0,1,0));
			}

			NxBox box;
			NxMat34 mat;
			NxCreateBox(box, worldBounds, mat);
			DrawWireBox(box, NxVec3(1,0,0));
		}
		break;
	}
}

void RenderCallback()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessCameraKeys();
	SetupCamera();

    if (gScene && !bPause)
	{
		GetPhysicsResults();
        ProcessInputs();
		StartPhysics();
	}

    // Display scene
 	RenderActors(bShadows);

	RenderIntersectedEntities();

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'v': { gIntersectionTestType = (gIntersectionTestType+1)%gNumIntersectionTestTypes; 
					// Add intersection test type to HUD
					char ds[512];
					sprintf(ds, "INTERSECTION TEST: %s", gIntersectionTestTypeString[gIntersectionTestType]);
					hud.SetDisplayString(2, ds, 0.015f, 0.92f);		
					break; }
	}
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
			hfVerts[i+j*(width+1)] = NxVec3(stride*NxReal(i),0.5,stride*NxReal(j)); 
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
//	hfNormals = new NxVec3[nbFaces];
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

NxActor* CreateBigFlatConvexObject(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride)
{
	NxVec3* hfVerts = NULL;
    NxU32* hfFaces = NULL;
    NxVec3* hfNormals = NULL;

	// Initialize heightfield verts
    NxU32 nbVerts = (length+1)*(width+1) + 1;
	hfVerts = new NxVec3[nbVerts];

	// Build flat heightfield
	for (NxU32 i = 0; i < width + 1; i++)
	{
		for (NxU32 j = 0; j < length + 1; j++)
		{
			hfVerts[i+j*(width+1)] = NxVec3(stride*NxReal(i),2,stride*NxReal(j)); 
		}
	}

	hfVerts[nbVerts - 1] = NxVec3(0,0,0);

	// Initialize heightfield faces
	NxU32 nbFaces = length*width*2 + 4;
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

    //  (length + 1)*(width + 1) - 1                 (length + 1)*width 
    // 
	//                     (length + 1)*(width + 1)
	//
    //           width                                      0

	hfFaces[k] = (length + 1)*(width + 1);
	hfFaces[k+1] = 0;
	hfFaces[k+2] = width;
    k+=3;

	hfFaces[k] = (length + 1)*(width + 1);
	hfFaces[k+1] = width;
	hfFaces[k+2] = (length + 1)*(width + 1) - 1;
    k+=3;

	hfFaces[k] = (length + 1)*(width + 1);
	hfFaces[k+1] = (length + 1)*(width + 1) - 1;
	hfFaces[k+2] = (length + 1)*width;
    k+=3;

	hfFaces[k] = (length + 1)*(width + 1);
	hfFaces[k+1] = (length + 1)*width;
	hfFaces[k+2] = 0;
    k+=3;

	// allocate heightfield materials -- one for each face
//	hfMaterials = new NxMaterialIndex[nbFaces];

	// Build vertex normals
//	hfNormals = new NxVec3[nbFaces];
//	NxBuildSmoothNormals(nbFaces, nbVerts, hfVerts, hfFaces, NULL, hfNormals, true);

	NxConvexMeshDesc hfDesc;
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

//	hfDesc.heightFieldVerticalAxis		= NX_Y;
//	hfDesc.heightFieldVerticalExtent	= 0;

	NxConvexShapeDesc hfShapeDesc;

    NxInitCooking();
    if (0)
    {
#ifdef WIN32    	
        // Cooking from file
        bool status = NxCookConvexMesh(hfDesc, UserStream("c:\\tmp.bin", false));
        hfShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif        
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookConvexMesh(hfDesc, buf);
        hfShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
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

void InitializeSpecialHUD()
{
	char ds[512];

	// Add intersection test type to HUD
	sprintf(ds, "INTERSECTION TEST: %s", gIntersectionTestTypeString[gIntersectionTestType]);
	hud.AddDisplayString(ds, 0.015f, 0.92f);
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity;
    sceneDesc.simType				= NX_SIMULATION_HW;
    gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}

	// Create the default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.5);
	defaultMaterial->setStaticFriction(0.5);
	defaultMaterial->setDynamicFriction(0.5);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	box = CreateBox(NxVec3(5,0,0), NxVec3(0.5,1,0.5), 20);
	sphere = CreateSphere(NxVec3(0,0,5), 1, 10);
	capsule = CreateCapsule(NxVec3(-5,0,0), 2, 0.6, 10);
	pyramid = CreatePyramid(NxVec3(0,0,0), NxVec3(1,0.5,1.5), 10);
	heightfield = CreateFlatHeightfield(NxVec3(-20,0,-5), 5, 5, 2);

    AddUserDataToActors(gScene);

	gSelectedActor = pyramid;

	// Initialize HUD
	InitializeHUD();
	InitializeSpecialHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 306: Entity Report");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
