// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    LESSON 109: COLLISION GROUPS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson109.h"

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
NxActor* sphere1 = NULL;
NxActor* sphere2 = NULL;
NxActor* sphere3 = NULL;
NxActor* capsule1 = NULL;
NxActor* capsule2 = NULL;
NxActor* capsule3 = NULL;

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

void CheckContacts()
{
	NxU32 nbActors = gScene->getNbActors();
	NxActor** actors = gScene->getActors();
	while (nbActors--)
	{
		NxActor* actor = *actors++;

		// Get actor user data
		ActorUserData* ud = (ActorUserData*)actor->userData;
		if (!ud)  continue;

		if (ud->contactEvents != 0)
		{
			NxU32 y;
			y = 0;
		}
		
		// Setup object's color according to events
		if (ud->contactEvents & NX_NOTIFY_ON_START_TOUCH)
		{
			NxU32 y;
			y = 0;
		}
		else if(ud->contactEvents & NX_NOTIFY_ON_END_TOUCH)
		{
			NxU32 y;
			y = 0;
		}
		else
		{
			NxU32 y;
			y = 0;
		}
	}
}

void ProcessInputs()
{
    ProcessForceKeys();

    CheckContacts();

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

void SetActorCollisionGroup(NxActor *actor, NxCollisionGroup group)
{
	NxU32 nbShapes = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();

	while (nbShapes--)
	{
		shapes[nbShapes]->setGroup(group);
	}
}

void InitActorCollisionGroups()
{
    SetActorCollisionGroup(box1, 1);
    SetActorCollisionGroup(box2, 1);
    SetActorCollisionGroup(box3, 1);

    SetActorCollisionGroup(capsule1, 2);
    SetActorCollisionGroup(capsule2, 2);
    SetActorCollisionGroup(capsule3, 2);

    SetActorCollisionGroup(sphere1, 3);
    SetActorCollisionGroup(sphere2, 3);
    SetActorCollisionGroup(sphere3, 3);

	gScene->setGroupCollisionFlag(1,2,true);
	gScene->setGroupCollisionFlag(2,3,true);
	gScene->setGroupCollisionFlag(1,3,false);
}

void InitContactReports()
{
	gScene->setActorPairFlags(*box1, *sphere1, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box1, *sphere2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box1, *sphere3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box2, *sphere1, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box2, *sphere2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box2, *sphere3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box3, *sphere1, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box3, *sphere2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box3, *sphere3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
}

void InitContactReportsPerPair()
{
	gScene->setActorPairFlags(*box1, *sphere1, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box1, *sphere2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box1, *sphere3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box2, *sphere1, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box2, *sphere2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box2, *sphere3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box3, *sphere1, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box3, *sphere2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	gScene->setActorPairFlags(*box3, *sphere3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
}

void InitContactReportsPerGroup()
{
	box1->setGroup(1);
	box2->setGroup(1);
	box3->setGroup(1);

	capsule1->setGroup(2);
	capsule2->setGroup(2);
	capsule3->setGroup(2);

	gScene->setActorGroupPairFlags(1, 2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
}

void DisableCollisionResponse()
{
	box1->raiseBodyFlag(NX_BF_KINEMATIC);
	box2->raiseBodyFlag(NX_BF_KINEMATIC);
	box3->raiseBodyFlag(NX_BF_KINEMATIC);

	box1->raiseActorFlag(NX_AF_DISABLE_RESPONSE);
	box2->raiseActorFlag(NX_AF_DISABLE_RESPONSE);
	box3->raiseActorFlag(NX_AF_DISABLE_RESPONSE);
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
	sceneDesc.userContactReport     = &gContactReport;
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

	box1 = CreateBox(NxVec3(6,0,1), NxVec3(0.5,1,0.5), 20);
	box2 = CreateBox(NxVec3(5,0,-0.25), NxVec3(0.5,1,0.5), 20);
	box3 = CreateBox(NxVec3(4,0,1), NxVec3(0.5,1,0.5), 20);

	capsule1 = CreateCapsule(NxVec3(1,0,6), 1.5, 0.5, 20);
	capsule2 = CreateCapsule(NxVec3(0,0,5), 1.5, 0.5, 20);
	capsule3 = CreateCapsule(NxVec3(-1,0,6), 1.5, 0.5, 20);

	sphere1 = CreateSphere(NxVec3(-6,0,1), 0.8, 20);
	sphere2 = CreateSphere(NxVec3(-5,0,-0.25), 0.8, 20);
	sphere3 = CreateSphere(NxVec3(-4,0,1), 0.8, 20);

	AddUserDataToActors(gScene);

	InitActorCollisionGroups();
	InitContactReportsPerPair();

#if 0
	InitContactReportsPerGroup();
	DisableCollisionResponse();
#endif  // 0

	gSelectedActor = box3;
	gForceStrength = 60000;

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
	InitGlut(argc, argv, "Lesson 109: Collision Groups");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

