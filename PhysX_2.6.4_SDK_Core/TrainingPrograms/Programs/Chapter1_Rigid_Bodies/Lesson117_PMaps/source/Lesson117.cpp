// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//								  LESSON 117: PMAPS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson117.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;
extern ErrorStream       gErrorStream;

// Camera globals
extern NxVec3 gCameraPos;

// HUD globals
extern HUD hud;

// Force globals
extern NxVec3 gForceVec;
extern NxReal gForceStrength;
extern bool bForceMode;

// Simulation globals
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/PMaps";
char fname2[] = "../../TrainingPrograms/Programs/Data/PMaps";
char fname3[] = "TrainingPrograms/Programs/Data/PMaps";

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
NxActor* bunny = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Mesh globals
NxTriangleMesh*	bunnyTriangleMesh;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;
        DrawActor(actor, gSelectedActor, true);

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

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	// Render the HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
}

// Bunny data
extern unsigned int		BUNNY_NBVERTICES;
extern unsigned int		BUNNY_NBFACES;
extern float			gBunnyVertices[];
extern int				gBunnyTriangles[];

//bunny related SDK variables
NxVec3*			gBunnyNormals = NULL;

NxU32 getFileSize(const char* name)
{
	#ifndef SEEK_END
	#define SEEK_END 2
	#endif
	
	FILE* File = fopen(name, "rb");
	if(!File)
		return 0;

	fseek(File, 0, SEEK_END);
	NxU32 eof_ftell = ftell(File);
	fclose(File);
	return eof_ftell;
}

void InitBunny()
{
	// Build vertex normals (used only for rendering)
//	gBunnyNormals = new NxVec3[BUNNY_NBVERTICES];
//	NxBuildSmoothNormals(BUNNY_NBFACES, BUNNY_NBVERTICES, (const NxVec3*)gBunnyVertices, (const NxU32*)gBunnyTriangles, NULL, gBunnyNormals, true);

	// Initialize PMap
	NxPMap bunnyPMap;
	bunnyPMap.dataSize	= 0;
	bunnyPMap.data		= NULL;

	// Build physical model
	NxTriangleMeshDesc bunnyDesc;
	bunnyDesc.numVertices				= BUNNY_NBVERTICES;
	bunnyDesc.numTriangles				= BUNNY_NBFACES;
	bunnyDesc.pointStrideBytes			= sizeof(NxVec3);
	bunnyDesc.triangleStrideBytes		= 3*sizeof(NxU32);
	bunnyDesc.points					= gBunnyVertices;
	bunnyDesc.triangles					= gBunnyTriangles;							
	bunnyDesc.flags						= 0;

    NxInitCooking();
    if (0)
    {
        // Cooking from file
#ifdef WIN32
        bool status = NxCookTriangleMesh(bunnyDesc, UserStream("c:\\tmp.bin", false));
        bunnyTriangleMesh = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
#endif
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookTriangleMesh(bunnyDesc, buf);
        bunnyTriangleMesh = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
    }

	// PMap stuff
	// Try loading PMAP from disk

	// Set PMap data directory
	int set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	//if (set != 0) set = chdir(&fname[1]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	FILE* fp = fopen("bunny.pmap", "rb");
	if(!fp)
	{
		// Not found => create PMap
		printf("please wait while precomputing pmap...\n");
		if(NxCreatePMap(bunnyPMap, *bunnyTriangleMesh, 64))
		{
			// The pmap has successfully been created, save it to disk for later use
			fp = fopen("bunny.pmap", "wb");
			if(fp)
			{
				fwrite(bunnyPMap.data, bunnyPMap.dataSize, 1, fp);
				fclose(fp);
				fp = NULL;
			}

			// Assign pmap to mesh
			bunnyTriangleMesh->loadPMap(bunnyPMap);

			// sdk created data => sdk deletes it
			NxReleasePMap(bunnyPMap);
		}
		printf("...done\n");
	}
	else
	{
		// Found pmap file
		bunnyPMap.dataSize	= getFileSize("bunny.pmap");
		bunnyPMap.data		= new NxU8[bunnyPMap.dataSize];
		fread(bunnyPMap.data, bunnyPMap.dataSize, 1, fp);
		fclose(fp);

		// Assign pmap to mesh
		bunnyTriangleMesh->loadPMap(bunnyPMap);

		// We created data => we delete it
		delete bunnyPMap.data;
	}
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 0.1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity;
//  PMAPS ARE NOT AVAILABLE IN HARDWARE
//    sceneDesc.simType				= NX_SIMULATION_HW;

//    gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}

	// If hardware is unavailable, fall back on software
	if (!gScene)
	{
        sceneDesc.simType				= NX_SIMULATION_SW;
        gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}
	}

	// Create the default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.5);
	defaultMaterial->setStaticFriction(0.5);
	defaultMaterial->setDynamicFriction(0.5);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	// Initialize bunny data
	InitBunny();

	bunny = CreateBunny(NxVec3(-2,0,0), bunnyTriangleMesh, 10);
	box = CreateBox(NxVec3(2,0,0), NxVec3(0.5,0.5,0.5), 10);

	AddUserDataToActors(gScene);

	gSelectedActor = bunny;
	gForceStrength = 7500;
    gCameraPos = NxVec3(0,2,-6);

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 117: PMaps");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

