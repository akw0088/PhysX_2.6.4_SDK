// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                    LESSON 501: CORE DUMP
//
//                          Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson501.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

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
char fname[] = "../../../../Data/Core_Dump";
char fname1[] = "../../../Data/Core_Dump";
char fname2[] = "../../TrainingPrograms/Programs/Data/Core_Dump";
char fname3[] = "TrainingPrograms/Programs/Data/Core_Dump";

#if __CELLOS_LV2__
#	include <sys/paths.h>
        char*fnameCD=SYS_APP_HOME"/MyCoreDump.cda";
#else
        char*fnameCD="MyCoreDump.cda";
#endif

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;

extern NxActor* gSelectedActor;

// Core dump globals
CoreContainer* ccPtr = NULL;
CoreContainer* getCoreDump(bool recreate) {
	if (recreate && ccPtr) {
		delete ccPtr;
		ccPtr = NULL;
	}
	if (ccPtr == NULL) {
		ccPtr = new CoreContainer();
	}
	return ccPtr;
}

bool bCoreDump = false;
bool bLoadCore = false;
bool bInstantiateCore = false;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
	printf("\n Special:\n --------\n c = Save Core Dump\n v = Load Core Dump\n n = Instantiate Core Dump\n");
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

	// Core dump
	if (bCoreDump)
	{
        NxCoreDump(gPhysicsSDK,fnameCD,false,false);
//      NxCoreDump(gPhysicsSDK,"MyCoreDump.cdb",true,false);  // sdk, file, binary, cooked meshes
		bCoreDump = false;
	}

	// Load core to core container
	if (bLoadCore)
	{
		CoreContainer* cc = getCoreDump(true);
		if (cc)
		{
			if (!cc->LoadCoreDump(gPhysicsSDK, fnameCD)) 
			{
				printf("Unable to load the core dump, please first save a core dump.\n");
			}
//			if (!cc->LoadCoreDump(gPhysicsSDK, "MyCoreDump.cdb")) 
//			{
//				printf("Unable to load the core dump, please first save a core dump.\n");
//			}
		}
		bLoadCore = false;
	}

	// Instantiate core from core container
	if (bInstantiateCore)
	{
		CoreContainer* cc = getCoreDump(false);
		NxScene *instScene = NULL;
		if (cc) 
		{
			instScene = cc->InstantiateCoreDump(gPhysicsSDK);
		}
		if (!instScene) 
		{
			printf("Unable to instantiate core dump, please load a core dump first.\n");
		}
		else
		{
            if (gScene)
	        {
			    ReleaseUserDataFromActors(gScene);
		        gPhysicsSDK->releaseScene(*gScene);
	        }
		    gScene = instScene;
		    AddUserDataToActors(gScene);
			NxActor** actors = gScene->getActors();
		    gSelectedActor = *actors;
			while(!IsSelectable(gSelectedActor))
			{
				gSelectedActor = *actors++;
			}
		}
		bInstantiateCore = false;
	}

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

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'c': { printf("Coredumping...\n"); bCoreDump = !bCoreDump; break; }
		case 'v': { printf("Loading Core Dump to Container...\n"); bLoadCore = !bLoadCore; break; }
		case 'n': { printf("Instantiating Core...\n"); bInstantiateCore = !bInstantiateCore; break; }
	}
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

    // Set Core Dump directory
	NxU32 set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname1[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	box = CreateBox(NxVec3(5,0,0), NxVec3(0.5,1,0.5), 20);
	sphere = CreateSphere(NxVec3(0,0,0), 1, 10);
	capsule = CreateCapsule(NxVec3(-5,0,0), 2, 0.5, 10);
//	pyramid = CreatePyramid(NxVec3(0,0,0), NxVec3(1,0.5,1.5), 10);

	AddUserDataToActors(gScene);

	gSelectedActor = capsule;
//	gSelectedActor = pyramid;

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
	InitGlut(argc, argv, "Lesson 501: Core Dump");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}



