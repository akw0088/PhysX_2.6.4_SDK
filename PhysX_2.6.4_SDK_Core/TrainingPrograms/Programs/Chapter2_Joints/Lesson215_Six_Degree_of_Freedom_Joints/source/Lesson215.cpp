// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					  LESSON 215: SIX DEGREE OF FREEDOM JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

// SIMULATE ABILITY TO SWITCH FROM ONE TYPE OF JOINT TO ANOTHER...

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson215.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

// HUD globals
extern HUD hud;
NxU32 gJointType = 0;
const NxU32 gNumJointConfigurations = 3;
char* gJointTypeString[gNumJointConfigurations] = {"Fixed","Revolute","Spherical"};
const NxU32 gNumJointDegreesOfFreedom = 6;
NxD6JointMotion gJointMotion[gNumJointDegreesOfFreedom] = 
{ 
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED
};
const NxU32 gNumJointTypesOfFreedom = 3;
char* gJointMotionString[gNumJointTypesOfFreedom] = {"Locked","Limited","Free"}; 

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
NxActor* capsule1 = NULL;
NxActor* capsule2 = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Joint globals
NxD6Joint* d6Joint = NULL;

bool bReconfigureD6Joint = false;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------------\n t = Reconfigure Joint\n");
}

// Reconfigure joint, a.k.a., roll joint
void ReconfigureD6Joint()
{
	NxActor* a0 = capsule1;
	NxActor* a1 = capsule2;

    NxD6JointDesc oldD6Desc, d6Desc;
    NxVec3 localAnchor[2], localAxis[2], localNormal[2], localBinormal[2];

    d6Joint->saveToDesc(oldD6Desc);

	localAnchor[0] = oldD6Desc.localAnchor[0];
	localAnchor[1] = oldD6Desc.localAnchor[1];

    localAxis[0] = oldD6Desc.localAxis[0];
    localNormal[0] = oldD6Desc.localNormal[0];
    localBinormal[0] = localNormal[0].cross(localAxis[0]);

    localAxis[1] = oldD6Desc.localAxis[1];
    localNormal[1] = oldD6Desc.localNormal[1];
    localBinormal[1] = localNormal[1].cross(localAxis[1]);

	switch (gJointType) 
	{
		case 0:  // Fixed Joint 
		{  
			// Coming from spherical joint, so reset actor #1
			NxMat33 orient;
			orient.id();
			a1->raiseBodyFlag(NX_BF_KINEMATIC);
			a1->setGlobalOrientation(orient);
			a1->setGlobalPosition(NxVec3(0,3,0));
			a1->clearBodyFlag(NX_BF_KINEMATIC);

			d6Desc.actor[0] = a0;
			d6Desc.actor[1] = a1;

			// Reset anchor and axis
			NxVec3 globalAnchor = NxVec3(0,5,0);
			NxVec3 globalAxis = NxVec3(0,0,-1);

			d6Desc.setGlobalAnchor(globalAnchor);
			d6Desc.setGlobalAxis(globalAxis);

			d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		}
		break;

		case 1:  // Revolute Joint 
		{  
			d6Desc.actor[0] = a0;
			d6Desc.actor[1] = a1;
			d6Desc.localAnchor[0] = localAnchor[0];
			d6Desc.localAnchor[1] = localAnchor[1];
			d6Desc.localAxis[0] = localAxis[0];
			d6Desc.localNormal[0] = localNormal[0];
			d6Desc.localAxis[1] = localAxis[1];
			d6Desc.localNormal[1] = localNormal[1];

			d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		}
		break;

		case 2:  // Spherical Joint 
		{  
			d6Desc.actor[0] = a0;
			d6Desc.actor[1] = a1;

			d6Desc.localAnchor[0] = localAnchor[0];
			d6Desc.localAnchor[1] = localAnchor[1];
			d6Desc.localAxis[0] = localBinormal[0];
			d6Desc.localNormal[0] = localNormal[0];
			d6Desc.localAxis[1] = localBinormal[1];
			d6Desc.localNormal[1] = localNormal[1];

			d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_FREE;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_FREE;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		}
		break;

	};

	d6Desc.projectionMode = NX_JPM_NONE;

	// Set joint motion display values
	gJointMotion[3] = d6Desc.twistMotion;
	gJointMotion[4] = d6Desc.swing1Motion;
	gJointMotion[5] = d6Desc.swing2Motion;

	gJointMotion[0] = d6Desc.xMotion;
	gJointMotion[1] = d6Desc.yMotion;
	gJointMotion[2] = d6Desc.zMotion;

	char ds[512];

	// Set joint type in HUD
	sprintf(ds, "JOINT TYPE: %s", gJointTypeString[gJointType]);
	hud.SetDisplayString(2, ds, 0.015f, 0.92f);	

	// Set rotation motions in HUD
    sprintf(ds, "   Axis: %s", gJointMotionString[gJointMotion[3]]);
	hud.SetDisplayString(4, ds, 0.015f, 0.82f); 
    sprintf(ds, "   Normal: %s", gJointMotionString[gJointMotion[4]]);
	hud.SetDisplayString(5, ds, 0.015f, 0.77f); 
    sprintf(ds, "   Binormal: %s", gJointMotionString[gJointMotion[5]]);
	hud.SetDisplayString(6, ds, 0.015f, 0.72f); 

	// Set translation motions in HUD
    sprintf(ds, "   Axis: %s", gJointMotionString[gJointMotion[0]]);
	hud.SetDisplayString(8, ds, 0.015f, 0.62f); 
    sprintf(ds, "   Normal: %s", gJointMotionString[gJointMotion[1]]);
	hud.SetDisplayString(9, ds, 0.015f, 0.57f); 
    sprintf(ds, "   Binormal: %s", gJointMotionString[gJointMotion[2]]);
	hud.SetDisplayString(10, ds, 0.015f, 0.52f);

	d6Joint->loadFromDesc(d6Desc);
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

	if (bReconfigureD6Joint)
	{
		bReconfigureD6Joint = false;
        gJointType = (gJointType+1)%gNumJointConfigurations;
		ReconfigureD6Joint(); 
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
	    case 't': { bReconfigureD6Joint = true; break; }
	}
}

NxD6Joint* CreateD6Joint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
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

void InitializeSpecialHUD()
{
	char ds[512];

	// Add joint type to HUD
    sprintf(ds, "JOINT TYPE: %s", gJointTypeString[gJointType]);
	hud.AddDisplayString(ds, 0.015f, 0.92f); 

	// Add rotation title to HUD
    sprintf(ds, "ROTATION:");
	hud.AddDisplayString(ds, 0.015f, 0.87f); 

	// Add rotation motions to HUD
    sprintf(ds, "   Axis: %s", gJointMotionString[gJointMotion[3]]);
	hud.AddDisplayString(ds, 0.015f, 0.82f); 
    sprintf(ds, "   Normal: %s", gJointMotionString[gJointMotion[4]]);
	hud.AddDisplayString(ds, 0.015f, 0.77f); 
    sprintf(ds, "   Binormal: %s", gJointMotionString[gJointMotion[5]]);
	hud.AddDisplayString(ds, 0.015f, 0.72f); 

	// Add translation title to HUD
	sprintf(ds, "TRANSLATION:");
	hud.AddDisplayString(ds, 0.015f, 0.67f); 

	// Add translation motions to HUD
    sprintf(ds, "   Axis: %s", gJointMotionString[gJointMotion[0]]);
	hud.AddDisplayString(ds, 0.015f, 0.62f); 
    sprintf(ds, "   Normal: %s", gJointMotionString[gJointMotion[1]]);
	hud.AddDisplayString(ds, 0.015f, 0.57f); 
    sprintf(ds, "   Binormal: %s", gJointMotionString[gJointMotion[2]]);
	hud.AddDisplayString(ds, 0.015f, 0.52f);
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
//	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1);

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

	capsule1 = CreateCapsule(NxVec3(0,5,0), 1, 0.5, 10);
	capsule1->raiseBodyFlag(NX_BF_KINEMATIC);
	capsule2 = CreateCapsule(NxVec3(0,3,0), 1, 0.5, 10);
	capsule2->setLinearDamping(0.5);

	NxVec3 globalAnchor = NxVec3(0,5,0);
	NxVec3 globalAxis = NxVec3(0,0,-1);
	d6Joint = CreateD6Joint(capsule1, capsule2, globalAnchor, globalAxis);
	
	gSelectedActor = capsule2;
	gForceStrength = 50000;
	gCameraSpeed = 10;

	// Initialize HUD
	InitializeHUD();
	InitializeSpecialHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 215: Six Degree of Freedom Joints");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

