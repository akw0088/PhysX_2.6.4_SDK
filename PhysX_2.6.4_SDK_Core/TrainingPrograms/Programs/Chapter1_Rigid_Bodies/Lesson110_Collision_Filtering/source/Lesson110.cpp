// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			               LESSON 110: COLLISION FILTERING
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson110.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;
extern ErrorStream       gErrorStream;

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

// Camera globals
extern NxVec3 gCameraPos;
extern NxReal gCameraSpeed;

// Actor globals
extern NxActor* groundPlane;
NxActor** boxes = NULL;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

        NxShape*const* shapes = actor->getShapes();
		NxGroupsMask gm = shapes[0]->getGroupsMask();
			
		if (gm.bits1)
			glColor4f(1.0f,0.0f,0.0f,1.0f); 
		else
			glColor4f(1.0f,1.0f,1.0f,1.0f);
		
		DrawActor(actor, gSelectedActor, true);

		glColor4f(1.0f,1.0f,1.0f,1.0f);

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

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
}

/**
 128-bit mask used for collision filtering.
 
 The collision filtering equation for 2 shapes S0 and S1 is:
 
 (G0 op0 K0) op2 (G1 op1 K1) == b
 
 with
 
 G0 = NxGroupsMask for shape S0. See ::setGroupsMask
 G1 = NxGroupsMask for shape S1. See ::setGroupsMask
 K0 = filtering constant 0. See ::setFilterConstant0
 K1 = filtering constant 1. See ::setFilterConstant1
 b = filtering boolean. See ::setFilterBool
 op0, op1, op2 = filtering operations. See ::setFilterOps
 
 If the filtering equation is true, collision detection is enabled. 
*/

void SetActorGroupsMask(NxActor* actor, NxGroupsMask gm)
{
    NxU32 nbShapes = actor->getNbShapes();
    NxShape*const* shapes = actor->getShapes();
    while (nbShapes--)
    {	
		NxShape* shape = shapes[nbShapes];
		shape->setGroupsMask(gm);
	}
}

void RandomizeActorCollisionFilters(NxScene* scene)
{
    NxU32 nbActors = scene->getNbActors();
    NxActor** actors = scene->getActors();
	while (nbActors--)
	{
		NxActor* actor = actors[nbActors];

	    NxU32 random = rand();
	    random &= 1;
     
		NxGroupsMask gm;
		gm.bits0 = 0; gm.bits1 = 0; gm.bits2 = 0; gm.bits3 = 0;

		if (random)  
			gm.bits0 = 1;
		else
			gm.bits1 = 1;

		SetActorGroupsMask(actor, gm);
	}
}

void InitNx()
{
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

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

	// Set up the filtering equation
	gScene->setFilterOps(NX_FILTEROP_OR, NX_FILTEROP_OR, NX_FILTEROP_AND);
	gScene->setFilterBool(true);
	NxGroupsMask zeroMask;
	zeroMask.bits0 = zeroMask.bits1 = zeroMask.bits2 = zeroMask.bits3 = 0;
	gScene->setFilterConstant0(zeroMask);
	gScene->setFilterConstant1(zeroMask);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();
	boxes = CreateStack(NxVec3(0,0,0), NxVec3(4,3,4), NxVec3(0.5,0.5,0.5), 10);

    AddUserDataToActors(gScene);

	RandomizeActorCollisionFilters(gScene);

	NxGroupsMask oneMask;
	oneMask.bits0 = 0xffffffff;
	oneMask.bits1 = 0xffffffff;
	oneMask.bits2 = 0xffffffff;
	oneMask.bits3 = 0xffffffff;
	SetActorGroupsMask(groundPlane, oneMask);
    
	gSelectedActor = boxes[0];
	gForceStrength = 30000;
	gCameraSpeed = 10;

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
	InitGlut(argc, argv, "Lesson 110: Collision Filtering");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

