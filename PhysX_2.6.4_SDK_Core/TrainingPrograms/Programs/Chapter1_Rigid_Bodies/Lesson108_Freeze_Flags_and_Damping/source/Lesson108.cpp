// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//	                    LESSON 108: FREEZE FLAGS AND DAMPING
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson108.h"

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

// Actor globals
extern NxActor* groundPlane;
NxActor* box1 = NULL;
NxActor* box2 = NULL;
NxActor* box3 = NULL;
NxActor* box4 = NULL;
NxActor* box5 = NULL;
extern NxActor* pyramid;

extern NxActor* gSelectedActor;

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

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
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

	box1 = CreateBox(NxVec3(10,0.5,0), NxVec3(1,1,1), 10);
	box2 = CreateBox(NxVec3(10,0.5,10), NxVec3(1,1,1), 10);
	box3 = CreateBox(NxVec3(0,0.5,10), NxVec3(1,1,1), 10);
	box4 = CreateBox(NxVec3(-10,0.5,10), NxVec3(1,1,1), 10);
	box5 = CreateBox(NxVec3(-10,0.5,0), NxVec3(1,1,1), 10);

	pyramid = CreatePyramid(NxVec3(0,0,0), NxVec3(1.5,0.75,1.5), 25);
	pyramid->setLinearDamping(0.5);

	box1->raiseBodyFlag(NX_BF_FROZEN_POS);
	box1->setLinearDamping(0.5);

	box2->raiseBodyFlag(NX_BF_FROZEN_POS_X); 
	box2->raiseBodyFlag(NX_BF_FROZEN_POS_Z);
	box2->setLinearDamping(0.5);
	
	box3->raiseBodyFlag(NX_BF_FROZEN_POS_Y); 
	box3->raiseBodyFlag(NX_BF_FROZEN_POS_Z);
	box3->setLinearDamping(0.5);
	
	box4->raiseBodyFlag(NX_BF_FROZEN_POS_X);
	box4->raiseBodyFlag(NX_BF_FROZEN_POS_Z);
	box4->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	box4->setLinearDamping(0.5);
	
	box5->raiseBodyFlag(NX_BF_FROZEN_POS_X);
	box5->raiseBodyFlag(NX_BF_FROZEN_POS_Y);
	box5->setLinearDamping(0.5);

	AddUserDataToActors(gScene);
	
	gSelectedActor = pyramid;
	gForceStrength = 500000;

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
	InitGlut(argc, argv, "Lesson 108: Freeze Flags and Damping");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
