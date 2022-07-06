// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                  LESSON 701: WHEEL SHAPES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson701.h"

#include "UserData.h"

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
extern NxReal gDeltaTime;
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* cab;

// Wheel globals
NxWheel* wheel = NULL;

// Wheel material globals
NxMaterial* wsm = NULL;

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
	    DrawActor(actor, gSelectedActor, false);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, false);
        }
    }
}

void UpdateWheelShapeUserData()
{
    // Look for wheel shapes
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
		NxU32 nbShapes = actors[nbActors]->getNbShapes();
	    NxShape*const* shapes = actors[nbActors]->getShapes();
		while (nbShapes--)
		{
			NxShape* shape = shapes[nbShapes];
			if (shape->getType() == NX_SHAPE_WHEEL)
			{
				NxWheelShape* ws = (NxWheelShape*)shape;
				ShapeUserData* sud = (ShapeUserData*)(shape->userData);
				if (sud)
				{
					// Need to save away roll angle in wheel shape user data
					NxReal rollAngle = sud->wheelShapeRollAngle;
//					rollAngle += ws->getAxleSpeed() * 1.0f/60.0f;
					rollAngle += ws->getAxleSpeed() * gDeltaTime;
					while (rollAngle > NxTwoPi)	//normally just 1x
						rollAngle -= NxTwoPi;
					while (rollAngle < -NxTwoPi)	//normally just 1x
						rollAngle += NxTwoPi;

					// We have the roll angle for the wheel now
					sud->wheelShapeRollAngle = rollAngle;


					NxMat34 pose;
					pose = ws->getGlobalPose();

					NxWheelContactData wcd;
					NxShape* s = ws->getContact(wcd);	

					NxReal r = ws->getRadius();
					NxReal st = ws->getSuspensionTravel();
					NxReal steerAngle = ws->getSteerAngle();

//					NxWheelShapeDesc state;	
//					ws->saveToDesc(state);

					NxVec3 p0;
					NxVec3 dir;
					/*
					getWorldSegmentFast(seg);
					seg.computeDirection(dir);
					dir.normalize();
					*/
					p0 = pose.t;  //cast from shape origin
					pose.M.getColumn(1, dir);
					dir = -dir;	//cast along -Y.
					NxReal castLength = r + st;	//cast ray this long

//					renderer.addArrow(p0, dir, castLength, 1.0f);
	
					//have ground contact?
					// This code is from WheelShape.cpp in SDKs/core/common/src
					// if (contactPosition != NX_MAX_REAL)  
					if (s && wcd.contactForce > -1000)
					{
//						pose.t = p0 + dir * wcd.contactPoint;
//						pose.t -= dir * state.radius;	//go from contact pos to center pos.
						pose.t = wcd.contactPoint;
						pose.t -= dir * r;	//go from contact pos to center pos.

						NxMat33 rot, axisRot;
						rot.rotY(steerAngle);
						axisRot.rotY(0);

//						NxReal rollAngle = ((ShapeUserData*)(wheel->userData))->rollAngle;

						NxMat33 rollRot;
						rollRot.rotX(rollAngle);

						pose.M = rot * pose.M * axisRot * rollRot;

						sud->wheelShapePose = pose;

					}
					else
					{
						pose.t = p0 + dir * st;
						sud->wheelShapePose = pose;
					}
				}
			}
		}
	}
}

void ProcessInputs()
{
    UpdateWheelShapeUserData();

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

void AddWheelToActor(NxActor* actor, NxWheelDesc* wheelDesc)
{
	NxWheelShapeDesc wheelShapeDesc;

	// Create a shared car wheel material to be used by all wheels
	if (!wsm)
	{
		NxMaterialDesc m;
		m.flags |= NX_MF_DISABLE_FRICTION;
		wsm = gScene->createMaterial(m);
	}
	wheelShapeDesc.materialIndex = wsm->getMaterialIndex();

	wheelShapeDesc.localPose.t = wheelDesc->position;
	NxQuat q;
	q.fromAngleAxis(90, NxVec3(0,1,0));
	wheelShapeDesc.localPose.M.fromQuat(q);

	NxReal heightModifier = (wheelDesc->wheelSuspension + wheelDesc->wheelRadius) / wheelDesc->wheelSuspension;

	wheelShapeDesc.suspension.spring = wheelDesc->springRestitution*heightModifier;
	wheelShapeDesc.suspension.damper = 0;  //wheelDesc->springDamping*heightModifier;
	wheelShapeDesc.suspension.targetValue = wheelDesc->springBias*heightModifier;

	wheelShapeDesc.radius = wheelDesc->wheelRadius;
	wheelShapeDesc.suspensionTravel = wheelDesc->wheelSuspension; 
	wheelShapeDesc.inverseWheelMass = 0.1;	//not given!? TODO

	// wheelShapeDesc.lateralTireForceFunction = ;	//TODO
	// wheelShapeDesc.longitudalTireForceFunction = ;	//TODO

    NxWheelShape* wheelShape;
	wheelShape = static_cast<NxWheelShape *>(actor->createShape(wheelShapeDesc));
}

NxActor* CreateCab(const NxVec3& pos)
{
	NxActor* actor = CreateBox(pos + NxVec3(0,1,0), NxVec3(1,0.5,0.5), 10);
	actor->raiseBodyFlag(NX_BF_FROZEN_ROT_X);
	actor->raiseBodyFlag(NX_BF_FROZEN_ROT_Z);
	actor->setAngularDamping(0.5);

	NxWheelDesc wheelDesc;
	//wheelDesc.wheelAxis.set(0,0,1);
	//wheelDesc.downAxis.set(0,-1,0);
    wheelDesc.wheelApproximation = 10;
	wheelDesc.wheelRadius = 0.5;
	wheelDesc.wheelWidth = 0.1;
	wheelDesc.wheelSuspension = 0.2;
	wheelDesc.springRestitution = 7000;
	wheelDesc.springDamping = 800;
	wheelDesc.springBias = 0;
	wheelDesc.maxBrakeForce = 1;
	wheelDesc.frictionToFront = 0.1;
	wheelDesc.frictionToSide = 0.99;
	wheelDesc.position = NxVec3(0,-0.4,0);
	wheelDesc.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		                   NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT | NX_WF_BUILD_LOWER_HALF;

    AddWheelToActor(actor, &wheelDesc);

	return actor;
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
	if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);	
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);

	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_POINT, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_NORMAL, 1);

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

	box = CreateBox(NxVec3(5,0,0), NxVec3(0.5,1,0.5), 20);
	sphere = CreateSphere(NxVec3(0,0,5), 1, 10);
	capsule = CreateCapsule(NxVec3(-5,0,0), 2, 0.5, 10);

	// Reset wheel material
	wsm = NULL;

	cab = CreateCab(NxVec3(0,0,0));

	gSelectedActor = cab;
	gForceStrength = 35000;

	// Turn on all contact notifications
	gScene->setActorGroupPairFlags(0, 0, NX_NOTIFY_ON_TOUCH);

    AddUserDataToActors(gScene);

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
	InitGlut(argc, argv, "Lesson 701: Wheel Shapes");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}



