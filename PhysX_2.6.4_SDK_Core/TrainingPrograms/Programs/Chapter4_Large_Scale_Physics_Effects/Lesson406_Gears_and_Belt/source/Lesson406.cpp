// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					         LESSON 406: GEARS AND BELT
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson406.h"

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
NxActor* wheel[2];
NxActor* frame;
NxActor* step[47];

// Focus actor
extern NxActor* gSelectedActor;

// Joint globals
NxRevoluteJoint* wheelJoint[2];
NxRevoluteJoint* stepJoint[47];

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
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
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
//	gScene->setTiming(1.0/200.0, 30);  // timeStep, maxIter, TIMESTEP_FIXED

	NxReal c = -13;
	NxReal s = 0;
	NxReal h = 5;
	NxReal stepWidth = 0.84;
	NxReal b = 0.1;
	NxReal numTeeth = 15;
	NxReal radius = 2;
	NxReal height = 1;

	NxI32 i;

	wheel[0] = CreateWheel(NxVec3(c+0.1,h,s),1.75,2.0,1.8,14,10);  // pos, minRadius, maxRadius, height, numTeeth, density
    wheel[0]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,0,0)));

	wheel[1] = CreateWheel(NxVec3(c+15,h,s),1.75,2.0,1.8,14,10);  // pos, minRadius, maxRadius, height, numTeeth, density
    wheel[1]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,0,0)));

	frame = CreateFrame(NxVec3(-5.5,h,s),10);

	wheelJoint[0] = CreateWheelJoint(wheel[0],frame,NxVec3(c+0.1,h,s),NxVec3(0,0,-1));
	wheelJoint[1] = CreateWheelJoint(wheel[1],frame,NxVec3(c+15,h,s),NxVec3(0,0,-1));

	for (i = 0; i < 17; i++) 
	{
		step[i] = CreateStep(NxVec3(c+stepWidth*i+(i-1)*b,h+2.3,s), NxVec3(5, stepWidth, 0.1), 10);
		step[i]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-90,-90,0)));
	}

	// 17 to 22
		step[17] = CreateStep(NxVec3(c+16.1,7.1,s), NxVec3(5, stepWidth, 0.1), 10);
		step[17]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-70,90,0)));

		step[18] = CreateStep(NxVec3(c + 16.8, 6.5, s), NxVec3(5, stepWidth, 0.1), 10);
		step[18]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-40,90,0)));

		step[19] = CreateStep(NxVec3(c + 17.2, 5.7, s), NxVec3(5, stepWidth, 0.1), 10);
		step[19]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(0,90,0)));

		step[20] = CreateStep(NxVec3(c + 17.1, 4.7, s), NxVec3(5, stepWidth, 0.1), 10);
		step[20]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(15,90,0)));

		step[21] = CreateStep(NxVec3(c + 16.8, 3.8, s), NxVec3(5, stepWidth, 0.1), 10);
		step[21]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(40,90,0)));

		step[22] = CreateStep(NxVec3(c + 16.3, 3.1, s), NxVec3(5, stepWidth, 0.1), 10);
		step[22]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(60,90,0)));

	for (i = 0; i < 17; i++) 
	{
		step[i+23] = CreateStep(NxVec3(c + 15.4 - stepWidth*i-(i-1)*b, 2.7, s), NxVec3(5, stepWidth, 0.1), 10);
		step[i+23]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,-90,0)));
	}

	// 40 to 46
		step[40] = CreateStep(NxVec3(c -0.5, 2.6, s), NxVec3(5, stepWidth, 0.1), 10);
		step[40]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(110,90,0)));

		step[41] = CreateStep(NxVec3(c -1.2, 3.1, s), NxVec3(5, stepWidth, 0.1), 10);
		step[41]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(140,90,0)));

		step[42] = CreateStep(NxVec3(c -1.8, 3.8, s), NxVec3(5, stepWidth, 0.1), 10);
		step[42]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(150,90,0)));

		step[43] = CreateStep(NxVec3(c-2.1, 4.7, s), NxVec3(5, stepWidth, 0.1), 10);
		step[43]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(170,90,0)));

		step[44] = CreateStep(NxVec3(c-2.1, 5.6, s), NxVec3(5, stepWidth, 0.1), 10);
		step[44]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-170,90,0)));

		step[45] = CreateStep(NxVec3(c-1.5, 6.5, s), NxVec3(5, stepWidth, 0.1), 10);
		step[45]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-140,90,0)));

		step[46] = CreateStep(NxVec3(c-0.8, 7.1, s), NxVec3(5, stepWidth, 0.1), 10);
		step[46]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-110,90,0)));

	for (i = 0; i < 46; i++) 
	{
		NxVec3 globalAnchor = 0.5*(step[i]->getCMassGlobalPosition() + step[i+1]->getCMassGlobalPosition());
		stepJoint[i] = CreateStepJoint(step[i],step[i+1], globalAnchor, NxVec3(0,0,-1));		
	}	

	NxVec3 ga = 0.5*(step[46]->getCMassGlobalPosition() + step[0]->getCMassGlobalPosition());
	stepJoint[46] = CreateStepJoint(step[46],step[0], ga, NxVec3(0,0,-1));

	AddUserDataToActors(gScene);

	gSelectedActor = wheel[0];
    gCameraPos = NxVec3(0,10,-20);
	gCameraSpeed = 20;
	gForceStrength = 15000000;

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
	InitGlut(argc, argv, "Lesson 406: Gears and Belt");
	NewLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
