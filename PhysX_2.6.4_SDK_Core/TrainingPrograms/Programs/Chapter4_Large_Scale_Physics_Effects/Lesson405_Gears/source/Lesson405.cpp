// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					              LESSON 405: GEARS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson405.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

// HUD globals
extern HUD hud;

// Camera globals
extern NxVec3 gCameraPos;
extern NxReal gCameraSpeed;

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
NxU32 numGears = 10;
NxActor** gear = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Joint globals
NxRevoluteJoint** gearJoint = NULL;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
}

void NewLight()
{
	// Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { -400.0f, 100.0f, 100.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
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
			DrawActorShadow2(actor, true);
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
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 0.5);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
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

	gScene->setTiming(1.0/200.0, 2);  // timeStep, maxIter, TIMESTEP_FIXED
//	gScene->setTiming(1.0/200.0, 30);  // throw into Well of Despair with 30 time steps

	NxReal gearHeight = 1;

    gear = new NxActor*[numGears*2];
    gearJoint = new NxRevoluteJoint*[numGears*2];

	// CREATE BOTTOM ROW OF GEARS
	NxU32 i;
	for (i = 0; i < numGears; i++)
	{
		NxReal a,c,s;
		NxReal radius = 4.35;
		a = 360*(i/(NxReal)numGears);
		c = NxMath::cos(a*(NxPi/180))*radius;
		s = NxMath::sin(a*(NxPi/180))*radius;

		gear[i] = CreateBoxGear(NxVec3(c,0.3+gearHeight,s), 1.25, 1.4, 0.5, 30, 10);  // pos, minRad, maxRad, height, numTeeth, density
		gear[i]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(0,5*NxMath::mod((NxF32)i,2),0)));
		//		gear[i] = CreateBoxGear(NxVec3(0,0,0), 1.25, 1.4, 0.5, 30, 10);  // pos, minRad, maxRad, height, numTeeth, density
	}
	for (i = 0; i < numGears; i++)
	{
		NxReal a,c,s;
		NxReal radius = 4.35;
		a = 360*(i/(NxReal)numGears);
		c = NxMath::cos(a*(NxPi/180))*radius;
		s = NxMath::sin(a*(NxPi/180))*radius;

		gearJoint[i] = CreateRevoluteJoint(gear[i], NULL, NxVec3(c,0.3+gearHeight,s), NxVec3(0,1,0));
	}

	// CREATE SIDE COLUMN OF GEARS
	for (i = 0; i < numGears; i++)
	{
		NxReal a,c,s;
		NxReal radius = 4.35;
		a = 360*(i/(NxReal)numGears);
		c = NxMath::cos(a*(NxPi/180))*radius;
		s = NxMath::sin(a*(NxPi/180))*radius;

		gear[i+numGears] = CreateBoxGear(NxVec3(5.88,c+6.2+gearHeight,s), 1.25, 1.4, 0.5, 30, 10);  // pos, minRad, maxRad, height, numTeeth, density
		gear[i+numGears]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(0,5*NxMath::mod((NxF32)i,2),90)));
		//		gear[i] = CreateBoxGear(NxVec3(0,0,0), 1.25, 1.4, 0.5, 30, 10);  // pos, minRad, maxRad, height, numTeeth, density
	}
	for (i = 0; i < numGears; i++)
	{
		NxReal a,c,s;
		NxReal radius = 4.35;
		a = 360*(i/(NxReal)numGears);
		c = NxMath::cos(a*(NxPi/180))*radius;
		s = NxMath::sin(a*(NxPi/180))*radius;

		gearJoint[i+numGears] = CreateRevoluteJoint(gear[i+numGears], NULL, NxVec3(5.88,c+6.2+gearHeight,s), NxVec3(-1,0,0));
	}

	AddUserDataToActors(gScene);

	gSelectedActor = gear[0];
    gCameraPos = NxVec3(0,10,-20);
	gCameraSpeed = 20;
	gForceStrength = 50000;

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
	InitGlut(argc, argv, "Lesson 405: Gears");
	NewLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
