// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					              LESSON 407: TANK
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Terrain/Heightfield.h"
#include "Lesson407.h"

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

// Motor globals
NxMotorDesc mNeutralDesc;
NxMotorDesc mForwardDesc;
NxMotorDesc mReverseDesc;
NxMotorDesc mBrakeDesc;

NxMotorDesc mLeftDesc;
NxMotorDesc mRightDesc;
NxMotorDesc mStopDesc;


class Tread
{
public:
	Tread(const NxVec3& pos, const NxReal density)
	{
		NxReal c = -13;
		NxReal s = 0;
		NxReal h = 5;
		NxReal stepWidth = 0.84;
		NxReal b = 0.1;
		NxReal numTeeth = 15;
		NxReal radius = 2;
		NxReal height = 1;

		NxU32 i;

		wheel[0] = CreateWheel(pos+NxVec3(c+0.1,h,s),1.75,2.0,1.8,14,density);  // pos, minRadius, maxRadius, height, numTeeth, density
		wheel[0]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,0,0)));

		wheel[1] = CreateWheel(pos+NxVec3(c+15,h,s),1.75,2.0,1.8,14,density);  // pos, minRadius, maxRadius, height, numTeeth, density
		wheel[1]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,0,0)));

		wheel[2] = CreateWheel(pos+NxVec3(c+5,h,s),1.75,2.0,1.8,14,density);  // pos, minRadius, maxRadius, height, numTeeth, density
		wheel[2]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,0,0)));

		wheel[3] = CreateWheel(pos+NxVec3(c+10,h,s),1.75,2.0,1.8,14,density);  // pos, minRadius, maxRadius, height, numTeeth, density
		wheel[3]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,0,0)));

		frame = CreateFrame(pos+NxVec3(-5.5,h,s),density);

		wheelJoint[0] = CreateWheelJoint(wheel[0],frame,pos+NxVec3(c+0.1,h,s),NxVec3(0,0,-1));
		wheelJoint[1] = CreateWheelJoint(wheel[1],frame,pos+NxVec3(c+15,h,s),NxVec3(0,0,-1));
		wheelJoint[2] = CreateWheelJoint(wheel[2],frame,pos+NxVec3(c+5,h,s),NxVec3(0,0,-1));
		wheelJoint[3] = CreateWheelJoint(wheel[3],frame,pos+NxVec3(c+10,h,s),NxVec3(0,0,-1));

		for (i = 0; i < 18; i++) 
		{
			step[i] = CreateStep(pos+NxVec3(c+stepWidth*i+(i-1)*b-0.6f,h+2.3,s), NxVec3(5, stepWidth, 0.1), density);
			step[i]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-90,90,0)));
		}

		/*step[i] = CreateStep(pos+NxVec3(c+stepWidth*i+(i-1)*b-0.8f,h+2.3,s), NxVec3(5, stepWidth*0.7, 0.1), density);
		step[i]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-90,90,0)));*/

		// 18 to 23
			step[18] = CreateStep(pos+NxVec3(c+16.1,7.1,s), NxVec3(5, stepWidth, 0.1), density);
			step[18]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-70,90,0)));

			step[19] = CreateStep(pos+NxVec3(c + 16.8, 6.5, s), NxVec3(5, stepWidth, 0.1), density);
			step[19]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-40,90,0)));

			step[20] = CreateStep(pos+NxVec3(c + 17.2, 5.7, s), NxVec3(5, stepWidth, 0.1), density);
			step[20]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(0,90,0)));

			step[21] = CreateStep(pos+NxVec3(c + 17.1, 4.7, s), NxVec3(5, stepWidth, 0.1), density);
			step[21]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(15,90,0)));

			step[22] = CreateStep(pos+NxVec3(c + 16.8, 3.8, s), NxVec3(5, stepWidth, 0.1), density);
			step[22]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(40,90,0)));

			step[23] = CreateStep(pos+NxVec3(c + 16.3, 3.1, s), NxVec3(5, stepWidth, 0.1), density);
			step[23]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(60,90,0)));

			step[24] = CreateStep(pos+NxVec3(c + 15, 2.9, s), NxVec3(5, stepWidth, 0.1), density);
			step[24]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(70,90,0)));

		for (i = 1; i < 17; i++) 
		{
			step[i+24] = CreateStep(pos+NxVec3(c + 15.4 - stepWidth*i-(i-1)*b-0.5 , 2.7, s), NxVec3(5, stepWidth, 0.1), density);
			step[i+24]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,90,0)));
		}

		step[i+24] = CreateStep(pos+NxVec3(c + 15.4 - stepWidth*i-(i-1)*b + 0.8f, 2.7, s), NxVec3(5, stepWidth*0.8f, 0.1), density);
		step[i+24]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(90,90,0)));



		// 42 to 48
			step[42] = CreateStep(pos+NxVec3(c -0.5, 2.8, s), NxVec3(5, stepWidth, 0.1), density);
			step[42]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(110,90,0)));

			step[43] = CreateStep(pos+NxVec3(c -1.2, 3.2, s), NxVec3(5, stepWidth, 0.1), density);
			step[43]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(140,90,0)));

			step[44] = CreateStep(pos+NxVec3(c -1.8, 3.9, s), NxVec3(5, stepWidth, 0.1), density);
			step[44]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(150,90,0)));

			step[45] = CreateStep(pos+NxVec3(c-2.1, 4.8, s), NxVec3(5, stepWidth, 0.1), density);
			step[45]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(170,90,0)));

			step[46] = CreateStep(pos+NxVec3(c-2.1, 5.8, s), NxVec3(5, stepWidth, 0.1), density);
			step[46]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-170,90,0)));

			step[47] = CreateStep(pos+NxVec3(c-1.5, 6.7, s), NxVec3(5, stepWidth, 0.1), density);
			step[47]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-140,90,0)));

			step[48] = CreateStep(pos+NxVec3(c-0.8, 7.1, s), NxVec3(5, stepWidth, 0.1), density);
			step[48]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-110,90,0)));

			step[49] = CreateStep(pos+NxVec3(c-0.2, 7.3, s), NxVec3(5, stepWidth, 0.1), density);
			step[49]->setGlobalOrientationQuat(AnglesToQuat(NxVec3(-90,90,0)));


		for (i = 0; i < 49; i++) 
		{
			NxVec3 globalAnchor = 0.5*(step[i]->getCMassGlobalPosition() + step[i+1]->getCMassGlobalPosition());
			stepJoint[i] = CreateStepJoint(step[i],step[i+1], globalAnchor, NxVec3(0,0,-1));		
		}	

		NxVec3 ga = 0.5*(step[49]->getCMassGlobalPosition() + step[1]->getCMassGlobalPosition());
		stepJoint[49] = CreateStepJoint(step[1],step[49], ga, NxVec3(0,0,-1));

		for (i = 0; i < 50; i++) 
		{
			SetActorMaterial(step[i], 0);
			SetActorCollisionGroup(step[i], 1);
			gScene->setGroupCollisionFlag(1,1,false);
		}
	}

    NxActor* wheel[4];
    NxActor* frame;
    NxActor* step[50];

    NxRevoluteJoint* wheelJoint[4];
    NxRevoluteJoint* stepJoint[50];
};

class Tank
{
public:
	Tank(const NxVec3& pos, const NxReal density)
	{
		tread[0] = new Tread(pos + NxVec3(0,0,-7.5), density);
		tread[1] = new Tread(pos + NxVec3(0,0,7.5), density);
		//tread[1] = tread[0];

		chassis = CreateChassis(pos+NxVec3(-6,3.5,0),NxVec3(7,1,4.5),density);
		turret = CreateTurret(pos+NxVec3(-6,3.5,0),NxVec3(7,1,4.5),density);
		cannon = CreateCannon(pos+NxVec3(-6,3.5,0),NxVec3(7,1,4.5),density);

		NxVec3 globalAnchor1 = 0.5*(chassis->getCMassGlobalPosition() + tread[0]->frame->getCMassGlobalPosition());
		chassisJoint[0] = CreateChassisJoint(chassis,tread[0]->frame,globalAnchor1,NxVec3(0,0,-1));

		NxVec3 globalAnchor2 = 0.5*(chassis->getCMassGlobalPosition() + tread[1]->frame->getCMassGlobalPosition());
		chassisJoint[1] = CreateChassisJoint(chassis,tread[1]->frame,globalAnchor2,NxVec3(0,0,-1));

		NxVec3 globalAnchor3 = turret->getCMassGlobalPosition();
		turretJoint = CreateRevoluteJoint(chassis,turret,globalAnchor3,NxVec3(0,1,0));

		NxVec3 globalAnchor4 = turret->getCMassGlobalPosition();
		cannonJoint = CreateCannonJoint(turret,cannon,globalAnchor4,NxVec3(0,0,-1));
	}

	Tread* tread[2];
	NxActor* chassis;
	NxActor* turret;
	NxActor* cannon;
	NxRevoluteJoint* chassisJoint[2];
	NxRevoluteJoint* turretJoint;
	NxFixedJoint* cannonJoint;
};

// Actor globals
extern NxActor* groundPlane;
Tank* tank = NULL;
extern NxActor* heightfield;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Tank Controls:\n --------------\n i = foward, k = reverse, , = brake\n j = left, l = right \n u = hard left, m = hard right\n");
    printf("\n Turret Controls:\n ----------------\n c = swivel left, v = swivel right\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
}

void NewLight()
{
    // Setup lighting
    glEnable(GL_LIGHTING);
	float AmbientColor1[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientColor1);
    float DiffuseColor1[]    = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseColor1);
    float SpecularColor1[]   = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularColor1);
    float Position1[]        = { -25.0f, 55.0f, 15.0f, 1.0f };     glLightfv(GL_LIGHT1, GL_POSITION, Position1);
    glEnable(GL_LIGHT0);

    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { -400.0f, 100.0f, 100.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
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
			DrawActorShadow2(actor, true);
        }
    }
}

void SetMotorOnTread(int i, const NxMotorDesc& mDesc)
{
    tank->tread[i]->wheelJoint[0]->setMotor(mDesc);
    tank->tread[i]->wheelJoint[1]->setMotor(mDesc);
    tank->tread[i]->wheelJoint[2]->setMotor(mDesc);
    tank->tread[i]->wheelJoint[3]->setMotor(mDesc);
}

extern bool gKeys[256];

void ProcessMotorKeys()
{
	// Process force keys
	for (int i = 0; i < 256; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Motor controls
			case 'i': {SetMotorOnTread(0, mForwardDesc); SetMotorOnTread(1, mForwardDesc); break; }
			case 'k': {SetMotorOnTread(0, mReverseDesc);SetMotorOnTread(1, mReverseDesc); break; }
			case ',': {SetMotorOnTread(0, mBrakeDesc);SetMotorOnTread(1, mBrakeDesc); break; }
			case 'l': {SetMotorOnTread(0, mForwardDesc);SetMotorOnTread(1, mBrakeDesc); break; }
			case 'j': {SetMotorOnTread(0, mBrakeDesc);SetMotorOnTread(1, mForwardDesc); break; }
			case 'u': {SetMotorOnTread(0, mReverseDesc);SetMotorOnTread(1, mForwardDesc); break; }
			case 'm': {SetMotorOnTread(0, mForwardDesc);SetMotorOnTread(1, mReverseDesc); break; }

			case 'c': {tank->turretJoint->setMotor(mLeftDesc); break; }
			case 'v': {tank->turretJoint->setMotor(mRightDesc); break; }
		}
	}
}

void ProcessInputs()
{
	// Flush the motor inputs
	SetMotorOnTread(0, mNeutralDesc); 
	SetMotorOnTread(1, mNeutralDesc);
	tank->turretJoint->setMotor(mStopDesc);

    ProcessMotorKeys();

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

void InitMotors()
{
	// Neutral
	mNeutralDesc.maxForce = 0;
	mNeutralDesc.velTarget = 0;
	mNeutralDesc.freeSpin = 1;

	// Forward
	mForwardDesc.maxForce = 750;
	mForwardDesc.velTarget = 200;
	mForwardDesc.freeSpin = 1;

	// Reverse
	mReverseDesc.maxForce = 750;
	mReverseDesc.velTarget = -200;
	mReverseDesc.freeSpin = 1;

	// Brake
	mBrakeDesc.maxForce = 500;
	mBrakeDesc.velTarget = 0;
	mBrakeDesc.freeSpin = 0;

    // Left
	mLeftDesc.maxForce = 100;
	mLeftDesc.velTarget = -50;
	mLeftDesc.freeSpin = 0;

    // Right
	mRightDesc.maxForce = 100;
	mRightDesc.velTarget = 50;
	mRightDesc.freeSpin = 0;

    // Stop
	mStopDesc.maxForce = 500;
	mStopDesc.velTarget = 0;
	mStopDesc.freeSpin = 0;
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
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
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
	defaultMaterial->setStaticFriction(0.85);
	defaultMaterial->setDynamicFriction(0.85);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	gScene->setTiming(1.0/200.0, 2);  // timeStep, maxIter, TIMESTEP_FIXED
//	gScene->setTiming(1.0/200.0, 30);  // timeStep, maxIter, TIMESTEP_FIXED

    gCameraPos = NxVec3(0,10,-20);
	tank = new Tank(gCameraPos + NxVec3(0,-10,5), 50);

	AddUserDataToActors(gScene);

	InitMotors();

	gSelectedActor = tank->chassis;
 	gCameraSpeed = 20;
	gForceStrength = 15000000;

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

int main(int argc, char** argv)
{
	bPause = true;

	PrintControls();
	InitGlut(argc, argv, "Lesson 407: Tank");
	NewLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

