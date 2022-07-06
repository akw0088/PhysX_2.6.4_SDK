// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					   LESSON 107: STATIC AND KINEMATIC ACTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson107.h"

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
extern NxReal gDeltaTime;
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;
NxActor* box1 = NULL;
NxActor* box2 = NULL;
NxActor* box3 = NULL;

extern NxActor* gSelectedActor;

// Kinematic motion globals
NxReal KT = 0;
NxReal KSpeed = 0.2;
NxVec3 gKineStartPos = NxVec3(-5,0,0);
bool bMoveKinematicActor = false;
bool bToggleKinematicActor = false;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n ----------------\n v = Turn Kinematic Actor Motion On/Off\n g = Toggle Kinematic Actor to Dynamic\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;
	    DrawActor(actor, gSelectedActor, false);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, false);
        }
    }
}

void MoveActorOnPath(NxActor* actor)
{
	NxReal x = 5*(NxMath::sin(KT*NxTwoPi)*NxMath::sin(KT*NxTwoPi) - 0.5) + 2.5;	
	actor->moveGlobalPosition(gKineStartPos + NxVec3(x,0,0));
	KT += KSpeed*gDeltaTime;
}

void ProcessInputs()
{
    ProcessForceKeys();

	// Move the kinematic actor
	if (bMoveKinematicActor)  MoveActorOnPath(box3);

    // Toggle box3 from kinematic to dynamic
	if (bToggleKinematicActor)
	{
        if (box3->readBodyFlag(NX_BF_KINEMATIC))
		    box3->clearBodyFlag(NX_BF_KINEMATIC);
		else
		    box3->raiseBodyFlag(NX_BF_KINEMATIC);
	    bToggleKinematicActor = false;
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
		DrawForce2(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce2(gSelectedActor, gForceVec, NxVec3(0,1,1));
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
		case 'v': { bMoveKinematicActor = !bMoveKinematicActor; break; }	
		case 'g': { bToggleKinematicActor = true; break; }	
	}
}

void InitNx()
{
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);

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

	box1 = CreateBox(NxVec3(5,0,0), NxVec3(1,1,1), 0);
	box2 = CreateBox(NxVec3(0,0,0), NxVec3(1,1,1), 5);
	box3 = CreateBox(gKineStartPos, NxVec3(1,1,1), 5);
	box3->raiseBodyFlag(NX_BF_KINEMATIC);

	gSelectedActor = box2;
    gForceStrength = 75000;

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
	InitGlut(argc, argv, "Lesson 107: Static and Kinematic Actors");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
