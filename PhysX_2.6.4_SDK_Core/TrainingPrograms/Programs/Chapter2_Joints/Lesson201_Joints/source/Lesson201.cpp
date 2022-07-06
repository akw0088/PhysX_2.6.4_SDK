// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						         LESSON 201: JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson201.h"

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
extern bool bPause;
extern bool bHardwareScene;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;
NxActor* box1 = NULL;
NxActor* box2 = NULL;

// Joint globals
//NxD6Joint* fixedHWJoint = NULL;
NxD6Joint* revHWJoint = NULL;

//NxFixedJoint* fixedJoint = NULL;
NxRevoluteJoint* revJoint = NULL;

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
        DrawActor(actor, gSelectedActor, false);

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

NxD6Joint* CreateFixedHWJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxD6JointDesc d6Desc;
	d6Desc.actor[0] = a0;
	d6Desc.actor[1] = a1;
	d6Desc.setGlobalAnchor(globalAnchor);
	d6Desc.setGlobalAxis(globalAxis);

	d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

	d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;

	d6Desc.projectionMode = NX_JPM_NONE;

	return (NxD6Joint*)gScene->createJoint(d6Desc);
}

NxFixedJoint* CreateFixedJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis)
{
	NxFixedJointDesc fixedDesc;

	fixedDesc.actor[0] = a0;
	fixedDesc.actor[1] = a1;
	fixedDesc.setGlobalAnchor(globalAnchor);
	fixedDesc.setGlobalAxis(globalAxis);

	return (NxFixedJoint*)gScene->createJoint(fixedDesc);
}

NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis)
{
	NxRevoluteJointDesc revDesc;

	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	revDesc.setGlobalAnchor(globalAnchor);
	revDesc.setGlobalAxis(globalAxis);

	revDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	revDesc.projectionMode = NX_JPM_POINT_MINDIST;
	revDesc.projectionDistance = 1.0f;
	revDesc.projectionAngle = 0.0872f;	//about 5 degrees in radians.


	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
    if(!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);

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

	NxVec3 globalAnchor = NxVec3(0.5,5,0);
	NxVec3 globalAxis = NxVec3(0,0,1);

//  fixedJoint = CreateFixedJoint(box1, box2, globalAnchor, globalAxis);
    revJoint = CreateRevoluteJoint(box1, box2, globalAnchor, globalAxis);

	gSelectedActor = box2;
	gForceStrength = 75000;
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
	InitGlut(argc, argv, "Lesson 201: Joints");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
