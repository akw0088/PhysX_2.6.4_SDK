// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					    LESSON 403: JOINTED RIGID BODY CLOTH
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson403.h"

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
NxActor** patches = NULL;

// Focus actor
extern NxActor* gSelectedActor;

class PatchLink
{
public:
	NxSphericalJoint* xJoint;
	NxSphericalJoint* zJoint;
};

PatchLink** patchLinks = NULL;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------\n Simulation Starts Paused - Hit p to Begin\n");
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

NxActor* CreatePatch(const NxVec3& pos, const NxVec3& patchDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box 
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(patchDim.x,patchDim.y,patchDim.z);
	actorDesc.shapes.pushBack(&boxDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

void CreateMattress(const NxVec3& pos, const int width, const int height, const NxVec3& patchDim)
{
	NxI32 i, j;

	patches = new NxActor*[width*height];
	NxActor** patchPtr = patches; 

	NxVec3 mattressOffset = pos + NxVec3(-width * patchDim.x, patchDim.y, -height * patchDim.z);

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			NxVec3 patchPos;
			patchPos.x = i * 2 * patchDim.x;
			patchPos.y = patchDim.y;
			patchPos.z = j * 2 * patchDim.z;
			patchPos += mattressOffset;
			*patchPtr = CreatePatch(patchPos, patchDim, 10);
//			NxShape **patchShapes = (*patchPtr)->getShapes();
//			patchShapes[0]->setGroup(1);
//			gPhysicsSDK->setGroupCollisionFlag(1,1,false);
			patchPtr++;
		}
	}

	// Create Patch Links
	patchLinks = new PatchLink*[width*height];
	PatchLink** patchLinkPtr = patchLinks; 

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			NxVec3 patchPos;
			patchPos.x = i * 2 * patchDim.x;
			patchPos.y = patchDim.y;
			patchPos.z = j * 2 * patchDim.z;
			patchPos += mattressOffset;
			*patchLinkPtr = new PatchLink;
			(*patchLinkPtr)->xJoint = NULL;
			(*patchLinkPtr)->zJoint = NULL;
			// Hoizontal Spring
			if (i < width - 1)
			{
				NxActor* patch1 = patches[i + j*width];
				NxActor* patch2 = patches[i + j*width + 1];
				NxVec3 xJointPos = patchPos + NxVec3(patchDim.x,0,0); 
				(*patchLinkPtr)->xJoint = CreateClothSphericalJoint(patch1, patch2, xJointPos, NxVec3(0,1,0) );

				patch1->wakeUp(1e10);
				patch2->wakeUp(1e10);
			}
			// Vertical Spring
			if (j < height - 1)
			{
				NxActor* patch1 = patches[i + j*width];
				NxActor* patch2 = patches[i + (j+1)*width];
				NxVec3 zJointPos = patchPos + NxVec3(0,0,patchDim.z); 
				(*patchLinkPtr)->zJoint = CreateClothSphericalJoint(patch1, patch2, zJointPos, NxVec3(0,1,0) );

				patch1->wakeUp(1e10);
				patch2->wakeUp(1e10);
			}
			patchLinkPtr++;
		}
	}

//	patches[width*(height-1)]->raiseBodyFlag(NX_BF_KINEMATIC);
//	patches[width*height - 1]->raiseBodyFlag(NX_BF_KINEMATIC);
}

void InitNx()
{
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
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

	CreateMattress(NxVec3(0,30,0), 16, 16, NxVec3(1,0.2,1));
//	CreateMattress(NxVec3(0,30,0), 20, 20, NxVec3(1,0.2,1));

	gSelectedActor = *patches;
    gCameraPos = NxVec3(0,15,-50);
	gCameraSpeed = 20;
	gForceStrength = 5000000;
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
	InitGlut(argc, argv, "Lesson 403: Jointed Rigid Body Cloth");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

