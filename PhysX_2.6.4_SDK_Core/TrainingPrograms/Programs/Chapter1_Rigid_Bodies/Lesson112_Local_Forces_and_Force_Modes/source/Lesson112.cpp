// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					   LESSON 112: LOCAL FORCES AND FORCE MODES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson112.h"

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

extern bool bShapeSelectMode;

// Actor globals
extern NxActor* groundPlane;
NxActor* table = NULL;

// Focus actor and shape
extern NxActor* gSelectedActor;
extern NxShape* gSelectedShape;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
	printf("\n Special:\n ----------------\n v = Toggle Shape Selection Mode\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		if (actor == gSelectedActor)
			DrawActorShapeSelect(gSelectedActor, gSelectedShape, bShapeSelectMode, gSelectedActor, false);
		else
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
    ProcessForceAtShapeKeys();

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
		DrawForceAtShape(gSelectedActor, gSelectedShape, gForceVec, NxVec3(1,1,0));
	else
		DrawForceAtShape(gSelectedActor, gSelectedShape, gForceVec, NxVec3(0,1,1));
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
        case 'v': { bShapeSelectMode = !bShapeSelectMode; break; }
	}
}

NxActor* CreateWoodenTable(const NxVec3& pos, const NxVec3& tableDim, const NxReal topRatio, const NxReal legRatio, const NxReal density)
{
	// Add a multi-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has five shapes, a top and four legs
	NxBoxShapeDesc topDesc;
	topDesc.dimensions.set(tableDim.x,topRatio*tableDim.y,tableDim.z);
	topDesc.localPose.t = NxVec3(0,(2-topRatio)*tableDim.y,0);
	actorDesc.shapes.pushBack(&topDesc);

	NxBoxShapeDesc ulLegDesc; 
	ulLegDesc.dimensions.set(legRatio*tableDim.x,tableDim.y,legRatio*tableDim.z);
	ulLegDesc.localPose.t = NxVec3((1-legRatio)*tableDim.x,tableDim.y,(1-legRatio)*tableDim.z);
	actorDesc.shapes.pushBack(&ulLegDesc);

	NxBoxShapeDesc urLegDesc; 
	urLegDesc.dimensions.set(legRatio*tableDim.x,tableDim.y,legRatio*tableDim.z);
	urLegDesc.localPose.t = NxVec3(-(1-legRatio)*tableDim.x,tableDim.y,(1-legRatio)*tableDim.z);
	actorDesc.shapes.pushBack(&urLegDesc);

	NxBoxShapeDesc llLegDesc; 
	llLegDesc.dimensions.set(legRatio*tableDim.x,tableDim.y,legRatio*tableDim.z);
	llLegDesc.localPose.t = NxVec3((1-legRatio)*tableDim.x,tableDim.y,-(1-legRatio)*tableDim.z);
	actorDesc.shapes.pushBack(&llLegDesc);

	NxBoxShapeDesc lrLegDesc; 
	lrLegDesc.dimensions.set(legRatio*tableDim.x,tableDim.y,legRatio*tableDim.z);
	lrLegDesc.localPose.t = NxVec3(-(1-legRatio)*tableDim.x,tableDim.y,-(1-legRatio)*tableDim.z);
	actorDesc.shapes.pushBack(&lrLegDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = density;
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
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
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AXES, 1);
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

	table = CreateWoodenTable(NxVec3(0,0,0), NxVec3(2,1,2), 0.2, 0.05, 10);

	gSelectedActor = table;
	gSelectedShape = table->getShapes()[0];
	gForceStrength = 100000;

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
	InitGlut(argc, argv, "Lesson 112: Local Forces and Force Modes");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}


