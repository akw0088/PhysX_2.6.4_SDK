// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							  LESSON 205: JOINT MOTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson205.h"

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

bool bToggleMotorVelocity = false;

// Actor globals
extern NxActor* groundPlane;
NxActor* box1 = NULL;
NxActor* box2 = NULL;

// Joint globals
NxRevoluteJoint* revJoint = NULL;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
    printf("\n Special:\n --------------\n v = Raise/Lower Motor RPMs\n");
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

void ShiftMotorVelocity()
{
   NxRevoluteJointDesc revDesc;
   revJoint->saveToDesc(revDesc);

   if (revDesc.motor.velTarget == 1000)
       revDesc.motor.velTarget = 10;
   else
       revDesc.motor.velTarget = 1000;

   revJoint->loadFromDesc(revDesc);
}

void ProcessInputs()
{
    ProcessForceKeys();

    if (bToggleMotorVelocity)
	{
        bToggleMotorVelocity = false;
		ShiftMotorVelocity();
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
		case 'v': { bToggleMotorVelocity = true; break; }
	}
}

NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis)
{
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	revDesc.setGlobalAnchor(globalAnchor);
	revDesc.setGlobalAxis(globalAxis);

	revDesc.flags |= NX_RJF_MOTOR_ENABLED;

	NxMotorDesc motorDesc;
	motorDesc.velTarget = 1000;
	motorDesc.maxForce = 500;
	motorDesc.freeSpin = true;

	revDesc.motor = motorDesc;

	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
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
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1);
//	gPhysicsSDK->setParameter(NX_MAX_ANGULAR_VELOCITY, 1);

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

	box1 = CreateBox(NxVec3(0,5,0), NxVec3(0.5,2,1), 10);
	box1->raiseBodyFlag(NX_BF_KINEMATIC);
	box2 = CreateBox(NxVec3(0,1,0), NxVec3(0.5,2,1), 10);

	box2->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	box2->setAngularDamping(0);
	box2->setMaxAngularVelocity(700);

	NxVec3 globalAnchor = NxVec3(0.5,5,-1);
	NxVec3 globalAxis = NxVec3(0,0,-1);
	revJoint = CreateRevoluteJoint(box1, box2, globalAnchor, globalAxis);
	
	gSelectedActor = box2;
	gForceStrength = 250000;
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
	InitGlut(argc, argv, "Lesson 205: Joint Motors");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

