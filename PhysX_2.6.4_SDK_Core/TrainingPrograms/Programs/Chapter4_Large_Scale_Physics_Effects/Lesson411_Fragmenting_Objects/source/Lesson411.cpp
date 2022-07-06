// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					       LESSON 411: FRAGMENTING OBJECTS
//
//						  Written by Sam Thompson, 3-15-05
//						    Edited by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson411.h"

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

// Fracturing Globals
const int iNumBoxesOnSide = 4;
const float fCubeSide = 1.5f;
const float fBoxDensity = 10.0f;

// Force globals
NxReal gForceStrengthPerObject = fCubeSide * fCubeSide * fCubeSide * fBoxDensity * 200.0f;
int iNumberObjects = 0;

bool bReleaseRandomShape = false;

// Actor globals
extern NxActor* groundPlane;
NxActor* mainBox = NULL;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Fracturing:\n -----------\n g = Break off piece\n");
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

	if (bReleaseRandomShape)
	{
		bReleaseRandomShape = false;
	    ReleaseRandomShape();
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
		case 'g': { bReleaseRandomShape = true;  break; }
	}
}

// Release one of the shapes from our compound. At the moment
// we are just releasing the first object in the array.
// As long as the index is below the number of shapes (uiNumShapes)
// it should work, but be careful about releasing an object inside
// other objects (for performance reasons)
void ReleaseRandomShape()
{
	// Get the array of pointers to shapes
	NxShape*const* ppShapeArray = mainBox->getShapes();

	// Find out how many shapes there are left
	NxU32 uiNumShapes = mainBox->getNbShapes();

	// Don't remove the last shape (or we'll have a shapeless actor)
	if (uiNumShapes < 2)
		return;

	// Record the pose of the shape we intend to release, for later
	NxShape* pShapeToRelease = ppShapeArray[0];
	NxMat34 mNewBoxPose = pShapeToRelease->getGlobalPose();

	// Release the object, and make a note of the new total number of objects
	mainBox->releaseShape(*pShapeToRelease);
	--iNumberObjects;

	// We need to update the mass and intertial tensors. We choose to base it
	// on the density of the objects, which are all fBoxDensity
	mainBox->updateMassFromShapes(fBoxDensity, 0.0f);

	// Create the new box, in the pose we recorded earlier
	CreateBoxPiece(mNewBoxPose);
}

// This is for creating the box that replaces the piece we pulled off of the main object
// It takes a global pose so it knows where to start it. It starts with no velocity
// at the moment, but could easily have one set.
NxActor* CreateBoxPiece(NxMat34& mGlobalPose)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box, fCubeSide meters on a side
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set( fCubeSide, fCubeSide, fCubeSide );
	actorDesc.shapes.pushBack(&boxDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = fBoxDensity;
	actorDesc.globalPose = mGlobalPose;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateMainObject()
{
	// Calculate starting height for our object based on it's size
	NxReal boxStartHeight = iNumBoxesOnSide * fCubeSide * 2.0f;

	// We need to add the main object to our scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor here is a compound: a cube with iNumBoxesOnSide cubes on a side, and thus is made of iNumBoxesOnSide^3 objects.
	NxBoxShapeDesc boxDesc[iNumBoxesOnSide*iNumBoxesOnSide*iNumBoxesOnSide]; // it's a cube with n cubes on a side, so we need n^3 boxes

	// Loop through all of the cubes
	for ( int i=0; i<iNumBoxesOnSide; ++i )
	{
		for ( int j=0; j<iNumBoxesOnSide; ++j )
		{
			for ( int k=0; k<iNumBoxesOnSide; ++k )
			{
				// The dimensions are determined by fCubeSide
				int iCurrentIndex = i * (iNumBoxesOnSide * iNumBoxesOnSide) + j * iNumBoxesOnSide + k;
				boxDesc[ iCurrentIndex ].dimensions.set( fCubeSide, fCubeSide, fCubeSide );

				// We need to place it in the right spot, relative to the main actor
				boxDesc[ iCurrentIndex ].localPose.t.set(
														 NxVec3(
																((i-1) * fCubeSide * 2.0f),
																((j-1) * fCubeSide * 2.0f),
																((k-1) * fCubeSide * 2.0f)
															   )
														);

				// Push it on to our shapes array
				actorDesc.shapes.pushBack(&boxDesc[iCurrentIndex]);
				
				// Track the number of objects globally so we can apply an appropriate force to the object
				++iNumberObjects;
			}
		}
	}

	actorDesc.body = &bodyDesc;
	actorDesc.density = 10;
	actorDesc.globalPose.t = NxVec3(0,boxStartHeight,0);

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
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 2);
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

	mainBox = CreateMainObject();

	gSelectedActor = mainBox;
	gCameraPos = NxVec3(0,15,-50);
	gCameraSpeed = 20;
    gForceStrength = 50000000;

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
	InitGlut(argc, argv, "Lesson 411: Fragmenting Objects");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
