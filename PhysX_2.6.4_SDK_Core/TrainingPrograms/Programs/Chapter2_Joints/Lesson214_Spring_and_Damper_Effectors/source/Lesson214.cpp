// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					   LESSON 214: SPRING AND DAMPER EFFECTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson214.h"

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

// Actor globals
extern NxActor* groundPlane;
NxActor* sphere1 = NULL;
NxActor* sphere2 = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Spring and Damper Effector globals
NxSpringAndDamperEffector* spring = NULL;

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

	// Draw the spring
	DrawLine(sphere1->getCMassGlobalPosition(), sphere2->getCMassGlobalPosition(), NxVec3(0,1,0));

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

NxSpringAndDamperEffector* CreateSpring(NxActor* actor1, const NxVec3& pos1, NxActor* actor2, const NxVec3& pos2)
{
	NxSpringAndDamperEffectorDesc sadeDesc;

	sadeDesc.body1 = actor1;
	sadeDesc.body2 = actor2;

	sadeDesc.pos1 = pos1;
	sadeDesc.pos2 = pos2;

    NxVec3 springVec = pos2 - pos1;

	// Linear spring parameters
	sadeDesc.springDistRelaxed = springVec.magnitude();
	sadeDesc.springMaxCompressForce = 2000;
	sadeDesc.springDistCompressSaturate = 0.25*sadeDesc.springDistRelaxed;
	sadeDesc.springMaxStretchForce = 2000;
	sadeDesc.springDistStretchSaturate = 2*sadeDesc.springDistRelaxed;
//	sadeDesc.springMaxCompressForce = 200000;
//	sadeDesc.springDistCompressSaturate = 0.25*sadeDesc.springDistRelaxed;
//	sadeDesc.springMaxStretchForce = 200000;
//	sadeDesc.springDistStretchSaturate = 2*sadeDesc.springDistRelaxed;

	// Linear damper parameters
	sadeDesc.damperMaxCompressForce = 100;
	sadeDesc.damperVelCompressSaturate = 0;
	sadeDesc.damperMaxStretchForce = 100;
	sadeDesc.damperVelStretchSaturate = 200;
//	sadeDesc.damperMaxCompressForce = 10000;
//	sadeDesc.damperVelCompressSaturate = 0;
//	sadeDesc.damperMaxStretchForce = 10000;
//	sadeDesc.damperVelStretchSaturate = 200;

	return gScene->createSpringAndDamperEffector(sadeDesc);
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

	sphere1 = CreateSphere(NxVec3(3,0,0), 1, 25);
	sphere1->raiseBodyFlag(NX_BF_FROZEN_ROT);
	sphere1->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	sphere1->setLinearDamping(0.25);

	sphere2 = CreateSphere(NxVec3(-3,0,0), 1, 25);
	sphere2->raiseBodyFlag(NX_BF_FROZEN_ROT);
	sphere2->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	sphere2->setLinearDamping(0.25);

	NxVec3 pos1 = sphere1->getCMassGlobalPosition();
	NxVec3 pos2 = sphere2->getCMassGlobalPosition();
	NxVec3 springVec = pos2 - pos1;

	spring = CreateSpring(sphere1, pos1, sphere2, pos2);

#if 0
	spring->setBodies(sphere1, pos1, sphere2, pos2);

	NxReal distSpringRelaxed = springVec.magnitude();
	NxReal maxSpringCompressForce = 2000;
	NxReal distSpringCompressSaturate = 0.25*distSpringRelaxed;
	NxReal maxSpringStretchForce = 2000;
	NxReal distSpringStretchSaturate = 2.0*distSpringRelaxed;

	spring->setLinearSpring(distSpringCompressSaturate, distSpringRelaxed, distSpringStretchSaturate,
							maxSpringCompressForce, maxSpringStretchForce);

	NxReal maxDamperCompressForce = 100;
	NxReal velDamperCompressSaturate = 200;
	NxReal maxDamperStretchForce = 100;
	NxReal velDamperStretchSaturate = 200;

	spring->setLinearDamper(velDamperCompressSaturate, velDamperStretchSaturate,
							maxDamperCompressForce, maxDamperStretchForce);

#endif  // 0

	sphere1->wakeUp(1e30);
	sphere2->wakeUp(1e30);

	gSelectedActor = sphere1;
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
	InitGlut(argc, argv, "Lesson 214: Spring and Damper Effectors");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
