// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//		          LESSON 106: PUTTING ACTORS TO SLEEP AND SKIN WIDTH
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson106.h"

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

extern NxReal gCameraSpeed;

// Simulation globals
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* sphere;
NxActor** boxes = NULL;

extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
}

void DrawActorBlueSleep(NxActor *actor, NxActor* selectedActor)
{
	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	if (actor == selectedActor) 
	{
		while (nShapes--)
		{
			DrawWireShape(shapes[nShapes], NxVec3(1,1,1), false);
		}
	}
	if (actor->isSleeping())
	{
		glDisable(GL_LIGHTING);
		glColor4f(0.0f,0.0f,1.0f,1.0f);
	}
	nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		DrawShape(shapes[nShapes], false);
	}
	if (actor->isSleeping())
	{
		glEnable(GL_LIGHTING);
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
		DrawActorBlueSleep(actor, gSelectedActor);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, false);
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
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);
	gPhysicsSDK->setParameter(NX_DEFAULT_SLEEP_LIN_VEL_SQUARED, 0.15*0.15);
	gPhysicsSDK->setParameter(NX_DEFAULT_SLEEP_ANG_VEL_SQUARED, 0.14*0.14);

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

	sphere = CreateSphere(NxVec3(0,6,0), 1, 50);
	sphere->raiseBodyFlag(NX_BF_ENERGY_SLEEP_TEST);
//	sphere->setSleepLinearVelocity(0.15*0.15);
//	sphere->setSleepAngularVelocity(0.14*0.14);
//	sphere->raiseBodyFlag(NX_BF_POSE_SLEEP_TEST);		// Use pose delta from last step to determine whether or not the body should be kept awake.
//    sphere->raiseBodyFlag(NX_BF_FILTER_SLEEP_VEL);
#if 0
	sphere->raiseBodyFlag(NX_BF_FILTER_SLEEP_VEL);		// Filter velocities used keep body awake.  Velocities are based on pose deltas, if
														// NX_BF_POSE_SLEEP_TEST flag is raised.  The filter reduces rapid oscillations and transient spikes.
	sphere->raiseBodyFlag(NX_BF_ENERGY_SLEEP_TEST);		// Enables energy-based sleeping algorithm.
														// @see NxActor.isSleeping() NxBodyDesc.sleepEnergyThreshold 
    NxBodyDesc
	flags					|= NX_BF_ENERGY_SLEEP_TEST;
	sleepEnergyThreshold	= 0.005f;
	sleepDamping			= 0.0f;
#endif  // 0
	boxes = CreateStack(NxVec3(0,0,0), NxVec3(6,3,6), NxVec3(0.25,0.25,0.25), 10);

	gSelectedActor = sphere;
	gForceStrength = 200000;
	gCameraSpeed = 20;
	bPause = true;

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
	InitGlut(argc, argv, "Lesson 106: Putting Actors to Sleep and Skin Width");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
