// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//		                     LESSON 303: CONTACT REPORT
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson303.h"

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
NxActor* box1 = NULL;
NxActor* box2 = NULL;

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
	while(nbActors--)
	{
		NxActor* actor = *actors++;

		// Get our object back
		ActorUserData* ud = (ActorUserData*)actor->userData;
		if (!ud)  continue;

		if (ud->contactEvents != 0)
		{
			// Code for every contact event: starting, ending, during
		}
		
		// Execute code according to events
		if (ud->contactEvents & NX_NOTIFY_ON_START_TOUCH)
		{
			// Code for the pair starting contact
		}
		if (ud->contactEvents & NX_NOTIFY_ON_END_TOUCH)
		{
			// Code for pair ending contact
		}
		if (ud->contactEvents & NX_NOTIFY_ON_TOUCH)
		{
			// Code for the pair in continual contact
		}
	}
}

void ProcessInputs()
{
    ProcessForceKeys();

    // Check for contacts
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

	// Draw contact points and normals
	NxU32 i;
	for (i = 0; i < cpArray.size(); i++)
	{
		ContactPoint point = cpArray[i];
		DrawContactPoint(point.p, 0.2, point.color);
	}
	for (i = 0; i < clArray.size(); i++)
	{
		ContactLine line = clArray[i];
		DrawArrow(line.p0, line.p1, line.color);
	}
	cpArray.clear();
	clArray.clear();
	ctArray.clear();

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

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_POINT, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_NORMAL, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_ERROR, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_FORCE, 1);

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

	box1 = CreateBox(NxVec3(-3,5,0), NxVec3(0.75,0.75,0.75), 5);
	box2 = CreateBox(NxVec3(3,0,0), NxVec3(1,1,1), 5);

	AddUserDataToActors(gScene);

	gScene->setActorPairFlags(*box1, *box2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);

	gSelectedActor = box1;
	
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
	InitGlut(argc, argv, "Lesson 303: Contact Report");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
