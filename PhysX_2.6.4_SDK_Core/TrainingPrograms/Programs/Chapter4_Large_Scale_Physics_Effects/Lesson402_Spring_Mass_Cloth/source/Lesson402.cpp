// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					        LESSON 402: SPRING-MASS CLOTH
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson402.h"

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
NxActor** clothMasses = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Spring and Damper globals
const NxReal kSpringCompressSaturateRatio = 0.1;
const NxReal kSpringStretchSaturateRatio = 2;
const NxReal kSpringCompressForce = 1;  // 100;  
const NxReal kSpringStretchForce = 4;  // 400;  

const NxReal kDamperCompressSaturateVel = -0.1;
const NxReal kDamperStretchSaturateVel = 0.1;
const NxReal kDamperCompressForce = 0.25;  // 25;  
const NxReal kDamperStretchForce = 0.25;   // 25;

class ClothSpring
{
public:
	NxSpringAndDamperEffector* horizSpring;
	NxSpringAndDamperEffector* vertSpring;
	NxSpringAndDamperEffector* crossUpSpring;
	NxSpringAndDamperEffector* crossDownSpring;
};

ClothSpring** clothSprings = NULL;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n ---------\n Simulation Starts Paused - Hit p to Begin\n");
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

	RenderCloth();

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

void RenderCloth()
{
	NxActor** clothMassPtr = clothMasses;

	NxU32 i,j;
	NxU32 height = 16;
	NxU32 width = 16;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			// Hoizontal Spring
			if (j < width - 1)
			{
				NxActor* mass1 = clothMassPtr[j + i*width];
				NxActor* mass2 = clothMassPtr[j + i*width + 1];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				DrawLine(pos1, pos2, NxVec3(0,1,0));
			}

			// Vertical Spring
			if (i < height - 1)
			{
				NxActor* mass1 = clothMassPtr[j + i*width];
				NxActor* mass2 = clothMassPtr[j + (i+1)*width];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				DrawLine(pos1, pos2, NxVec3(0,1,0));
			}

			// Cross Up Spring
			if ((j < width - 1) && (i < height - 1))
			{
				NxActor* mass1 = clothMassPtr[j + i*width + 1];
				NxActor* mass2 = clothMassPtr[j + (i+1)*width];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				DrawLine(pos1, pos2, NxVec3(1,0,0));
			}

			// Cross Down Spring
			if ((j < width - 1) && (i < height - 1))
			{
				NxActor* mass1 = clothMassPtr[j + (i+1)*width + 1];
				NxActor* mass2 = clothMassPtr[j + i*width];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				DrawLine(pos1, pos2, NxVec3(1,0,0));
			}
		}
	}
}

NxSpringAndDamperEffector* CreateSpring()
{
	NxSpringAndDamperEffectorDesc sadeDesc;
	return gScene->createSpringAndDamperEffector(sadeDesc);
}

NxActor* CreateClothMass(const NxVec3& pos, const NxReal radius, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	bodyDesc.flags |= NX_BF_FROZEN_ROT;

//	bodyDesc.flags |= NX_BF_DISABLE_GRAVITY;
	bodyDesc.linearDamping = 0.5;
//	bodyDesc.solverIterationCount = 16;

	// The actor has one shape, a sphere
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.localPose.t = NxVec3(0,radius,0);
	actorDesc.shapes.pushBack(&sphereDesc);

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

void CreateCloth(const NxVec3& pos, int width, int height, NxReal patchHeight, NxReal patchWidth)
{
	// Create cloth masses
	clothMasses = new NxActor*[width*height]; 
	NxActor** clothMassPtr = clothMasses; 

	NxI32 i, j;

	NxVec3 clothOffset;

	clothOffset.x = -width * patchWidth / 2;
	clothOffset.y = 5;
	clothOffset.z = 0;
	
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			NxVec3 massOffset;
			massOffset.x = j * patchWidth;
			massOffset.y = i * patchHeight;
			massOffset.z = 0;
			massOffset += clothOffset;
			*clothMassPtr = CreateClothMass(massOffset, 0.2, 1);
			NxShape*const* massShapes = (*clothMassPtr)->getShapes();
			massShapes[0]->setGroup(1);
			gScene->setGroupCollisionFlag(1,1,true);
			clothMassPtr++;
		}
	}

	// Create Cloth Spring Meshes
	clothMassPtr = clothMasses;
	clothSprings = new ClothSpring*[width*height];
	ClothSpring** clothSpringPtr = clothSprings;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			*clothSpringPtr = new ClothSpring;
			(*clothSpringPtr)->horizSpring = NULL;
			(*clothSpringPtr)->vertSpring = NULL;
			(*clothSpringPtr)->crossUpSpring = NULL;
			(*clothSpringPtr)->crossDownSpring = NULL;

			// Hoizontal Spring
			if (j < width - 1)
			{
				(*clothSpringPtr)->horizSpring = CreateSpring();
				NxActor* mass1 = clothMassPtr[j + i*width];
				NxActor* mass2 = clothMassPtr[j + i*width + 1];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				NxReal distRelaxed = springVec.magnitude();
				NxReal distCompressSaturate = kSpringCompressSaturateRatio*distRelaxed;
				NxReal distStretchSaturate = kSpringStretchSaturateRatio*distRelaxed;

				NxReal maxCompressForce = kSpringCompressForce;
				NxReal maxStretchForce = kSpringStretchForce;

				(*clothSpringPtr)->horizSpring->setLinearSpring(distCompressSaturate, distRelaxed, distStretchSaturate,
								maxCompressForce, maxStretchForce);

				NxReal velDamperCompressSaturate = kDamperCompressSaturateVel;
				NxReal velDamperStretchSaturate = kDamperStretchSaturateVel;
				NxReal maxDamperCompressForce = kDamperCompressForce;
				NxReal maxDamperStretchForce = kDamperStretchForce;

				(*clothSpringPtr)->horizSpring->setLinearDamper(velDamperCompressSaturate, velDamperStretchSaturate,
								maxDamperCompressForce, maxDamperStretchForce);

				(*clothSpringPtr)->horizSpring->setBodies(mass1, pos1, mass2, pos2);

				mass1->wakeUp(1e30);
				mass2->wakeUp(1e30);
			}

			// Vertical Spring
			if (i < height - 1)
			{
				(*clothSpringPtr)->vertSpring = CreateSpring();
				NxActor* mass1 = clothMassPtr[j + i*width];
				NxActor* mass2 = clothMassPtr[j + (i+1)*width];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				NxReal distRelaxed = springVec.magnitude();
				NxReal distCompressSaturate = kSpringCompressSaturateRatio*distRelaxed;
				NxReal distStretchSaturate = kSpringStretchSaturateRatio*distRelaxed;

				NxReal maxCompressForce = kSpringCompressForce;
				NxReal maxStretchForce = kSpringStretchForce;

				(*clothSpringPtr)->vertSpring->setLinearSpring(distCompressSaturate, distRelaxed, distStretchSaturate,
								maxCompressForce, maxStretchForce);

				NxReal velDamperCompressSaturate = kDamperCompressSaturateVel;
				NxReal velDamperStretchSaturate = kDamperStretchSaturateVel;
				NxReal maxDamperCompressForce = kDamperCompressForce;
				NxReal maxDamperStretchForce = kDamperStretchForce;

				(*clothSpringPtr)->vertSpring->setLinearDamper(velDamperCompressSaturate, velDamperStretchSaturate,
								maxDamperCompressForce, maxDamperStretchForce);

				(*clothSpringPtr)->vertSpring->setBodies(mass1, pos1, mass2, pos2);

				mass1->wakeUp(1e30);
				mass2->wakeUp(1e30);
			}


			// Cross Up Spring
			if ((j < width - 1) && (i < height - 1))
			{
				(*clothSpringPtr)->crossUpSpring = CreateSpring();
				NxActor* mass1 = clothMassPtr[j + i*width + 1];
				NxActor* mass2 = clothMassPtr[j + (i+1)*width];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				NxReal distRelaxed = springVec.magnitude();
				NxReal distCompressSaturate = kSpringCompressSaturateRatio*distRelaxed;
				NxReal distStretchSaturate = kSpringStretchSaturateRatio*distRelaxed;

				NxReal maxCompressForce = kSpringCompressForce;
				NxReal maxStretchForce = kSpringStretchForce;

				(*clothSpringPtr)->crossUpSpring->setLinearSpring(distCompressSaturate, distRelaxed, distStretchSaturate,
								maxCompressForce, maxStretchForce);

				NxReal velDamperCompressSaturate = kDamperCompressSaturateVel;
				NxReal velDamperStretchSaturate = kDamperStretchSaturateVel;
				NxReal maxDamperCompressForce = kDamperCompressForce;
				NxReal maxDamperStretchForce = kDamperStretchForce;

				(*clothSpringPtr)->crossUpSpring->setLinearDamper(velDamperCompressSaturate, velDamperStretchSaturate,
								maxDamperCompressForce, maxDamperStretchForce);

				(*clothSpringPtr)->crossUpSpring->setBodies(mass1, pos1, mass2, pos2);

				mass1->wakeUp(1e30);
				mass2->wakeUp(1e30);
			}

			// Cross Down Spring
			if ((j < width - 1) && (i < height - 1))
			{
				(*clothSpringPtr)->crossDownSpring = CreateSpring();
				NxActor* mass1 = clothMassPtr[j + (i+1)*width + 1];
				NxActor* mass2 = clothMassPtr[j + i*width];
				NxVec3 pos1 = mass1->getCMassGlobalPosition();
				NxVec3 pos2 = mass2->getCMassGlobalPosition();

				NxVec3 springVec = pos2 - pos1;

				NxReal distRelaxed = springVec.magnitude();
				NxReal distCompressSaturate = kSpringCompressSaturateRatio*distRelaxed;
				NxReal distStretchSaturate = kSpringStretchSaturateRatio*distRelaxed;

				NxReal maxCompressForce = kSpringCompressForce;
				NxReal maxStretchForce = kSpringStretchForce;

				(*clothSpringPtr)->crossDownSpring->setLinearSpring(distCompressSaturate, distRelaxed, distStretchSaturate,
								maxCompressForce, maxStretchForce);

				NxReal velDamperCompressSaturate = kDamperCompressSaturateVel;
				NxReal velDamperStretchSaturate = kDamperStretchSaturateVel;
				NxReal maxDamperCompressForce = kDamperCompressForce;
				NxReal maxDamperStretchForce = kDamperStretchForce;

				(*clothSpringPtr)->crossDownSpring->setLinearDamper(velDamperCompressSaturate, velDamperStretchSaturate,
								maxDamperCompressForce, maxDamperStretchForce);

				(*clothSpringPtr)->crossDownSpring->setBodies(mass1, pos1, mass2, pos2);

				mass1->wakeUp(1e30);
				mass2->wakeUp(1e30);
			}

			clothSpringPtr++;
		}
	}

	clothMassPtr[width*(height-1)]->raiseBodyFlag(NX_BF_KINEMATIC);
	clothMassPtr[width*height - 1]->raiseBodyFlag(NX_BF_KINEMATIC);
}

void InitNx()
{
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 0.5);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity * 0.1;
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

	CreateCloth(NxVec3(0,0,0), 16, 16, 0.5, 0.5);

	gSelectedActor = *clothMasses;
	gForceStrength = 250;
	gCameraSpeed = 20;
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
	InitGlut(argc, argv, "Lesson 402: Spring-Mass Cloth");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
