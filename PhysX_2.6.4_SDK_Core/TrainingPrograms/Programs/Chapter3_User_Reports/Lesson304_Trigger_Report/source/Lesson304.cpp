// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						     LESSON 304: TRIGGER REPORT
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson304.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;
extern TriggerReport     gTriggerReport;

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

bool bForcefieldOn = false;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;
NxActor* triggerBox = NULL;

extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------\n v = Toggle Trigger Forcefield\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		if (((ActorUserData*)(actor->userData))->flags & UD_IS_TRIGGER)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
			if (gNbTouchedBodies)	
				glColor4f(1.0f, 0.1f, 0.1f, 0.8f);
			else					
				glColor4f(0.1f, 1.0f, 0.8f, 0.8f);

 			DrawActor(actor, gSelectedActor, true);

			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else
		{
 			DrawActor(actor, gSelectedActor, true);

			// Handle shadows
            if (shadows)
            {
		        DrawActorShadow(actor, true);
            }
		}
    }
}

void ApplyForcefieldToActors(const NxVec3 force)
{
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    for (NxU32 i = 0; i < nbActors; i++)
    {
		ActorUserData* ud = (ActorUserData*)(actors[i]->userData);
        if (ud->flags & UD_IS_INSIDE_TRIGGER)
        {
            actors[i]->addForce(force);
            ud->flags &= ~UD_IS_INSIDE_TRIGGER;
        }
    }
}

void ProcessInputs()
{
    ProcessForceKeys();

	// Apply an upward force of 1000 to actors caught in the trigger
	if (bForcefieldOn)
	{
        ApplyForcefieldToActors(NxVec3(0,1000,0));
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
        case 'v': { bForcefieldOn = !bForcefieldOn; break; }
	}
}

// Create a static trigger
NxActor* CreateTriggerBox(const NxVec3& pos, const NxVec3& boxDim)
{
	NxActorDesc actorDesc;

	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = boxDim;
	boxDesc.shapeFlags |= NX_TRIGGER_ENABLE;

	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.globalPose.t = pos + NxVec3(0, boxDim.y, 0);
	 
	return gScene->createActor(actorDesc);
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

	box = CreateBox(NxVec3(-5,0,0), NxVec3(0.5,1,0.5), 10);
	box->setLinearDamping(0.5);
	sphere = CreateSphere(NxVec3(0,0,5), 1, 10);
	sphere->setLinearDamping(0.5);
	capsule = CreateCapsule(NxVec3(5,0,5), 1, 0.5, 10);
	capsule->setLinearDamping(0.5);
	pyramid = CreatePyramid(NxVec3(5,0,0), NxVec3(1,0.5,1), 10);
	pyramid->setLinearDamping(0.5);

	triggerBox = CreateTriggerBox(NxVec3(0,0,0), NxVec3(2,2,2));
	gScene->setUserTriggerReport(&gTriggerReport);

	AddUserDataToActors(gScene);
	((ActorUserData*)(triggerBox->userData))->flags |= UD_IS_TRIGGER;

	gSelectedActor = box;
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
	InitGlut(argc, argv, "Lesson 304: Trigger Report");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}


