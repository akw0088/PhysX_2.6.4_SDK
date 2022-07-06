// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					            LESSON 404: RAGDOLLS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson404.h"

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

class Ragdoll
{
public:
	Ragdoll(const NxVec3& pos)
	{
		NxQuat qRotLeft, qRotRight, qRotAround;
		qRotLeft.fromAngleAxis(90, NxVec3(0,0,1));
		qRotRight.fromAngleAxis(-90, NxVec3(0,0,1));
		qRotAround.fromAngleAxis(180, NxVec3(0,0,1));

		NxMat33 mRotLeft, mRotRight, mRotAround;
		mRotLeft.fromQuat(qRotLeft);
		mRotRight.fromQuat(qRotRight);
		mRotAround.fromQuat(qRotAround);

		// Create body parts
		head = CreateSphere(NxVec3(0,8.8,0), 0.5, 10);
		torso = CreateSphere(NxVec3(0,7,0), 0.95, 10);
		pelvis = CreateSphere(NxVec3(0,5.8,0), 0.7, 10);

		leftUpperArm = CreateCapsule(NxVec3(0.5,8.5,0), 1, 0.4, 10);
		leftUpperArm->setGlobalOrientationQuat(qRotRight);
		leftForeArm = CreateCapsule(NxVec3(2,8.5,0), 1, 0.3, 10);
		leftForeArm->setGlobalOrientationQuat(qRotRight);
		leftHand = CreateBox(NxVec3(3.5,8.5,0), NxVec3(0.3,0.3,0.1), 10);
		leftHand->setGlobalOrientationQuat(qRotRight);

		rightUpperArm = CreateCapsule(NxVec3(-0.5,8.5,0), 1, 0.4, 10);
		rightUpperArm->setGlobalOrientationQuat(qRotLeft);
		rightForeArm = CreateCapsule(NxVec3(-2,8.5,0), 1, 0.3, 10);
		rightForeArm->setGlobalOrientationQuat(qRotLeft);
		rightHand = CreateBox(NxVec3(-3.5,8.5,0), NxVec3(0.3,0.3,0.1), 10);
		rightHand->setGlobalOrientationQuat(qRotLeft);

		leftThigh = CreateCapsule(NxVec3(0.6,6,0), 1.5, 0.5, 10);
		leftThigh->setGlobalOrientationQuat(qRotAround);
		leftCalf = CreateCapsule(NxVec3(0.6,3.5,0), 1.5, 0.35, 10);
		leftCalf->setGlobalOrientationQuat(qRotAround);
		leftFoot = CreateBox(NxVec3(0.6,1.5,0.2), NxVec3(0.4,0.2,0.75), 10);
		leftFoot->setGlobalOrientationQuat(qRotAround);

		rightThigh = CreateCapsule(NxVec3(-0.6,6,0), 1.5, 0.5, 10);
		rightThigh->setGlobalOrientationQuat(qRotAround);
		rightCalf = CreateCapsule(NxVec3(-0.6,3.5,0), 1.5, 0.35, 10);
		rightCalf->setGlobalOrientationQuat(qRotAround);
		rightFoot = CreateBox(NxVec3(-0.6,1.5,0.2), NxVec3(0.4,0.2,0.75), 10);
		rightFoot->setGlobalOrientationQuat(qRotAround);

		// Joint body parts together
		neck = CreateBodySphericalJoint(head,torso,NxVec3(0,8.8,0),NxVec3(0,1,0));
		leftShoulder = CreateBodySphericalJoint(leftUpperArm,torso,NxVec3(0.5,8.5,0),NxVec3(1,0,0));
		rightShoulder = CreateBodySphericalJoint(rightUpperArm,torso,NxVec3(-0.5,8.5,0),NxVec3(-1,0,0));
		spine = CreateBodySphericalJoint(torso,pelvis,NxVec3(0,7,0),NxVec3(0,-1,0));
		leftHip = CreateBodySphericalJoint(leftThigh,pelvis,NxVec3(0.6,6,0),NxVec3(0,-1,0));
		rightHip = CreateBodySphericalJoint(rightThigh,pelvis,NxVec3(-0.6,6,0),NxVec3(0,-1,0));

		leftElbow = CreateRevoluteJoint(leftForeArm,leftUpperArm,NxVec3(2,8.5,0),NxVec3(0,0,-1));
		rightElbow = CreateRevoluteJoint(rightForeArm,rightUpperArm,NxVec3(-2,8.5,0),NxVec3(0,0,-1));

		leftWrist = CreateRevoluteJoint2(leftHand,leftForeArm,NxVec3(0,-0.15,0),NxVec3(0,1.3,0),NxVec3(0,1,0),NxVec3(0,1,0));
		rightWrist = CreateRevoluteJoint2(rightHand,rightForeArm,NxVec3(0,-0.15,0),NxVec3(0,1.3,0),NxVec3(0,1,0),NxVec3(0,1,0));

		leftKnee = CreateRevoluteJoint(leftCalf,leftThigh,NxVec3(0.6,3.5,0),NxVec3(1,0,0));
		rightKnee = CreateRevoluteJoint(rightCalf,rightThigh,NxVec3(-0.6,3.5,0),NxVec3(-1,0,0));

		leftAnkle = CreateRevoluteJoint(leftFoot,leftCalf,NxVec3(0.6,1.3,0),NxVec3(1,0,0));
		rightAnkle = CreateRevoluteJoint(rightFoot,rightCalf,NxVec3(-0.6,1.3,0),NxVec3(-1,0,0));
	};

	NxActor* head;
	NxActor* torso;
	NxActor* pelvis;
	NxActor* leftUpperArm;
	NxActor* rightUpperArm;
	NxActor* leftForeArm;
	NxActor* rightForeArm;
	NxActor* leftHand;
	NxActor* rightHand;
	NxActor* leftThigh;
	NxActor* rightThigh;
	NxActor* leftCalf;
	NxActor* rightCalf;
	NxActor* leftFoot;
	NxActor* rightFoot;

	NxSphericalJoint* neck;
	NxSphericalJoint* leftShoulder;
	NxSphericalJoint* rightShoulder;
	NxSphericalJoint* spine;
	NxSphericalJoint* leftHip;
	NxSphericalJoint* rightHip;

	NxRevoluteJoint* leftElbow;
	NxRevoluteJoint* rightElbow;
	NxRevoluteJoint* leftWrist;
	NxRevoluteJoint* rightWrist;
	NxRevoluteJoint* leftKnee;
	NxRevoluteJoint* rightKnee;
	NxRevoluteJoint* leftAnkle;
	NxRevoluteJoint* rightAnkle;
};

// Actor globals
extern NxActor* groundPlane;
Ragdoll* guy = NULL;

// Focus actor
extern NxActor* gSelectedActor;

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

	guy = new Ragdoll(NxVec3(0,0,0));

	gSelectedActor = guy->head;
    gCameraPos = NxVec3(0,10,-20);
	gCameraSpeed = 20;
	gForceStrength = 200000;
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
	InitGlut(argc, argv, "Lesson 404: Ragdolls");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
