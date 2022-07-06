// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			     LESSON 216: 6DOF JOINT LIMITS, SPRINGS, AND MOTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

// SIMULATE ABILITY TO SWITCH FROM ONE TYPE OF JOINT TO ANOTHER...

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson216.h"

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
const NxU32 gNumJointConfigurations = 6;
char* gJointTypeString[gNumJointConfigurations] = {"Translation Limited","Translation Soft Limited",
												   "Rotation Limited","Rotation Soft Limited",
												   "Translation Motored", "Rotation Motored"};
const NxU32 gNumJointDegreesOfFreedom = 6;
NxD6JointMotion gJointMotion[gNumJointDegreesOfFreedom] = 
{ 
    NX_D6JOINT_MOTION_LIMITED,
    NX_D6JOINT_MOTION_LIMITED,
    NX_D6JOINT_MOTION_LIMITED,
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED,
    NX_D6JOINT_MOTION_LOCKED
};
const NxU32 gNumJointTypesOfFreedom = 3;
char* gJointMotionString[gNumJointTypesOfFreedom] = {"Locked","Limited","Free"};
char* gOnOffString[2] = {"Off","On"};

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
bool bToggleLowerActorGravity = false;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------------\n t = Reconfigure Joint\n g = Toggle Lower Actor Gravity\n");
}

// Reconfigure joint, a.k.a., roll joint
void ReconfigureD6Joint()
{
	NxActor* a0 = capsule1;
	NxActor* a1 = capsule2;

    NxD6JointDesc d6Desc;

    // Reset actor #1
    NxMat33 orient;
    orient.id();
    a1->raiseBodyFlag(NX_BF_KINEMATIC);
    a1->setGlobalOrientation(orient);
    a1->setGlobalPosition(NxVec3(0,3,0));
    a1->clearBodyFlag(NX_BF_KINEMATIC);

    d6Desc.actor[0] = a0;
    d6Desc.actor[1] = a1;

    // Reset Anchor and Axis
    NxVec3 globalAnchor = NxVec3(0,5,0);
    NxVec3 globalAxis = NxVec3(0,1,0);

    d6Desc.setGlobalAnchor(globalAnchor);
    d6Desc.setGlobalAxis(globalAxis);

	switch (gJointType) 
	{
		case 0:  // Translation Limited Joint 
		{
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LIMITED;

	        d6Desc.linearLimit.value = 0.8;
	        d6Desc.linearLimit.restitution = 0;
	        d6Desc.linearLimit.spring = 0;
	        d6Desc.linearLimit.damping = 0;
		}
		break;

		case 1:  // Translation Soft Limited Joint 
		{ 
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LIMITED;

	        d6Desc.linearLimit.value = 0.8;
	        d6Desc.linearLimit.restitution = 0;
	        d6Desc.linearLimit.spring = 100;
	        d6Desc.linearLimit.damping = 0.1;
		}
		break;

		case 2:  // Rotation Limited Joint 
		{ 
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;

	        d6Desc.swing1Limit.value = 0.3*NxPi;
	        d6Desc.swing1Limit.restitution = 0;
	        d6Desc.swing1Limit.spring = 0;
	        d6Desc.swing1Limit.damping = 0;

	        d6Desc.swing2Limit.value = 0.3*NxPi;
	        d6Desc.swing2Limit.restitution = 0;
	        d6Desc.swing2Limit.spring = 0;
	        d6Desc.swing2Limit.damping = 0;

	        d6Desc.twistLimit.low.value = -0.05*NxPi;
	        d6Desc.twistLimit.low.restitution = 0;
	        d6Desc.twistLimit.low.spring = 0;
	        d6Desc.twistLimit.low.damping = 0;
			
			d6Desc.twistLimit.high.value = 0.05*NxPi;
	        d6Desc.twistLimit.high.restitution = 0;
	        d6Desc.twistLimit.high.spring = 0;
	        d6Desc.twistLimit.high.damping = 0;
		}
		break;

		case 3:  // Rotation Soft Limited Joint 
		{ 
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;

	        d6Desc.swing1Limit.value = 0.3*NxPi;
	        d6Desc.swing1Limit.restitution = 0;
	        d6Desc.swing1Limit.spring = 300;
	        d6Desc.swing1Limit.damping = 10;

	        d6Desc.swing2Limit.value = 0.3*NxPi;
	        d6Desc.swing2Limit.restitution = 0;
	        d6Desc.swing2Limit.spring = 300;
	        d6Desc.swing2Limit.damping = 10;

	        d6Desc.twistLimit.low.value = -0.05*NxPi;
	        d6Desc.twistLimit.low.restitution = 0;
	        d6Desc.twistLimit.low.spring = 300;
	        d6Desc.twistLimit.low.damping = 10;
			
			d6Desc.twistLimit.high.value = 0.05*NxPi;
	        d6Desc.twistLimit.high.restitution = 0;
		}
		break;

		case 4:  // Translation Motored Joint 
		{ 
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LIMITED;

	        d6Desc.linearLimit.value = 1;
	        d6Desc.linearLimit.restitution = 0;
	        d6Desc.linearLimit.spring = 0;
	        d6Desc.linearLimit.damping = 0;

			d6Desc.zDrive.driveType = NX_D6JOINT_DRIVE_POSITION;
			d6Desc.zDrive.spring = 100;
			d6Desc.zDrive.damping = 0;
			d6Desc.drivePosition.set(0,5,1);

//			d6Desc.zDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//			d6Desc.zDrive.forceLimit = FLT_MAX;
//			d6Desc.driveLinearVelocity.set(0,0,5);
		}
		break;

		case 5:  // Rotation Motored Joint 
		{ 
			d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
			d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;

			d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
			d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;

	        d6Desc.swing1Limit.value = 0.3*NxPi;
	        d6Desc.swing1Limit.restitution = 0;
	        d6Desc.swing1Limit.spring = 0;
	        d6Desc.swing1Limit.damping = 0;

	        d6Desc.swing2Limit.value = 0.3*NxPi;
	        d6Desc.swing2Limit.restitution = 0;
	        d6Desc.swing2Limit.spring = 0;
	        d6Desc.swing2Limit.damping = 0;

	        d6Desc.twistLimit.low.value = -0.05*NxPi;
	        d6Desc.twistLimit.low.restitution = 0;
	        d6Desc.twistLimit.low.spring = 0;
	        d6Desc.twistLimit.low.damping = 0;
			
			d6Desc.twistLimit.high.value = 0.05*NxPi;
	        d6Desc.twistLimit.high.restitution = 0;
	        d6Desc.twistLimit.high.spring = 0;
	        d6Desc.twistLimit.high.damping = 0;

			// Slerp Drive - Orientation Target
			d6Desc.flags = NX_D6JOINT_SLERP_DRIVE;
			d6Desc.slerpDrive.driveType = NX_D6JOINT_DRIVE_POSITION;
			d6Desc.slerpDrive.spring = 200;
			d6Desc.slerpDrive.damping = 0;
			NxQuat q;
			q.fromAngleAxis(90,NxVec3(0,1,0));
			d6Desc.driveOrientation = q;

//			// Slerp Drive - Angular Velocity Target
//			d6Desc.flags = NX_D6JOINT_SLERP_DRIVE;
//			d6Desc.slerpDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//			d6Desc.slerpDrive.forceLimit = FLT_MAX;
//			d6Desc.driveAngularVelocity = NxVec3(0,5,0);

//			// Swing Drive - Orientation Target
//			d6Desc.flags = 0;
//			d6Desc.swingDrive.driveType = NX_D6JOINT_DRIVE_POSITION;
//			d6Desc.swingDrive.spring = 200;
//			d6Desc.swingDrive.damping = 0;
//			NxQuat q;
//			q.fromAngleAxis(90,NxVec3(0,1,0));
//			d6Desc.driveOrientation = q;

//          // Swing Drive - Angular Velocity Target
//			d6Desc.flags = 0;
//			d6Desc.swingDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//			d6Desc.swingDrive.forceLimit = FLT_MAX;
//			d6Desc.driveAngularVelocity = NxVec3(0,5,0);

//			// Twist Drive - Orientation Target
//			d6Desc.flags = 0;
//			d6Desc.twistDrive.driveType = NX_D6JOINT_DRIVE_POSITION;
//			d6Desc.twistDrive.spring = 200;
//			d6Desc.twistDrive.damping = 0;
//			NxQuat q;
//			q.fromAngleAxis(90,NxVec3(1,0,0));
//			d6Desc.driveOrientation = q;

//          // Twist Drive - Angular Velocity Target
//			d6Desc.flags = 0;
//			d6Desc.twistDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//			d6Desc.twistDrive.forceLimit = FLT_MAX;
//			d6Desc.driveAngularVelocity = NxVec3(5,0,0);
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

void UpdateJointMotorTarget()
{
	if (gJointType == 4)  // Linear Motor 
	{
		NxD6JointDesc d6Desc;
        d6Joint->saveToDesc(d6Desc);
		if (d6Desc.zDrive.driveType == NX_D6JOINT_DRIVE_POSITION)  // Position Target
		{
		    NxVec3 jointPos = d6Joint->getGlobalAnchor();		    
			if ((jointPos.x > 0.4) || (jointPos.x < -0.4))
			{
			if (jointPos.x > 0.4)
		       d6Desc.drivePosition = NxVec3(0,5,-1);
			else if (jointPos.x < -0.4)
		       d6Desc.drivePosition = NxVec3(0,5,1);
			d6Joint->loadFromDesc(d6Desc);
			}
		} 
		else if (d6Desc.zDrive.driveType == NX_D6JOINT_DRIVE_VELOCITY)  // Velocity Target
		{
		    NxVec3 jointPos = d6Joint->getGlobalAnchor();	
			if ((jointPos.x > 0.4) || (jointPos.x < -0.4))
			{
			if (jointPos.x > 0.4)
		       d6Desc.driveLinearVelocity = NxVec3(0,0,-5);
			else if (jointPos.x < -0.4)
		       d6Desc.driveLinearVelocity = NxVec3(0,0,5);
			d6Joint->loadFromDesc(d6Desc);
			}
		}
	}
	else if (gJointType == 5)  // Rotational Motor 
	{
		NxD6JointDesc d6Desc;
        d6Joint->saveToDesc(d6Desc);

        NxVec3 localAnchor[2], localAxis[2], localNormal[2], localBinormal[2];
	    localAnchor[0] = d6Desc.localAnchor[0];
	    localAnchor[1] = d6Desc.localAnchor[1];
        localAxis[0] = d6Desc.localAxis[0];
        localNormal[0] = d6Desc.localNormal[0];
        localBinormal[0] = localNormal[0].cross(localAxis[0]);
        localAxis[1] = d6Desc.localAxis[1];
        localNormal[1] = d6Desc.localNormal[1];
        localBinormal[1] = localNormal[1].cross(localAxis[1]);

		NxMat34 pose1 = capsule1->getGlobalPose();
		NxVec3 axis1;
		pose1.M.getRow(1,axis1);

		NxMat34 pose2 = capsule2->getGlobalPose();
		NxVec3 axis2;
		pose2.M.getRow(1,axis2);

		if (d6Desc.flags == NX_D6JOINT_SLERP_DRIVE)  // Slerp Angular Drive
		{			
	        if (d6Desc.slerpDrive.driveType == NX_D6JOINT_DRIVE_POSITION)  // Orientation Target
		    {
			    if ((axis2.x > 0.5) || (axis2.x < -0.5))
			    {
			        if (axis2.x > 0.5)
			        {
			            NxQuat q;
			            q.fromAngleAxis(-90, NxVec3(0,1,0));
			            d6Desc.driveOrientation = q; 		       
			        }
			   	    else if (axis2.x < -0.5)
			        {
			            NxQuat q;
			            q.fromAngleAxis(90, NxVec3(0,1,0));
			            d6Desc.driveOrientation = q; 	       
			        }
			       	d6Joint->loadFromDesc(d6Desc);
			    }
		    }
	        else if (d6Desc.slerpDrive.driveType == NX_D6JOINT_DRIVE_VELOCITY)  // Angular Velocity Target
		    {
		        if ((axis2.x > 0.5) || (axis2.x < -0.5))
			    {
			        if (axis2.x > 0.5)
			        {
			            d6Desc.driveAngularVelocity = NxVec3(0,5,0); 
			        }
			     	else if (axis2.x < -0.5)
			        {
			            d6Desc.driveAngularVelocity = NxVec3(0,-5,0); 
			        }
			   	    d6Joint->loadFromDesc(d6Desc);
			    }
			}

		}
		else  // Swing-twist Angular Drive 
		{	
	        if (d6Desc.swingDrive.driveType == NX_D6JOINT_DRIVE_POSITION)  // Orientation Target
		    {
			    if ((axis2.x > 0.5) || (axis2.x < -0.5))
			    {
			        if (axis2.x > 0.5)
			        {
			            NxQuat q;
			            q.fromAngleAxis(-90, NxVec3(0,1,0));
			            d6Desc.driveOrientation = q; 		       
			        }
			   	    else if (axis2.x < -0.5)
			        {
			            NxQuat q;
			            q.fromAngleAxis(90, NxVec3(0,1,0));
			            d6Desc.driveOrientation = q; 	       
			        }
			       	d6Joint->loadFromDesc(d6Desc);
			    }
		    }
	        else if (d6Desc.swingDrive.driveType == NX_D6JOINT_DRIVE_VELOCITY)  // Angular Velocity Target
		    {
		        if ((axis2.x > 0.5) || (axis2.x < -0.5))
			    {
			        if (axis2.x > 0.5)
			        {
			            d6Desc.driveAngularVelocity = NxVec3(0,5,0); 
			        }
			     	else if (axis2.x < -0.5)
			        {
			            d6Desc.driveAngularVelocity = NxVec3(0,-5,0); 
			        }
			   	    d6Joint->loadFromDesc(d6Desc);
			    }
			}
		}
	}
}

void ProcessInputs()
{
    ProcessForceKeys();

	UpdateJointMotorTarget();

	if (bReconfigureD6Joint)
	{
		bReconfigureD6Joint = false;
		gJointType = (gJointType+1)%gNumJointConfigurations; 
		ReconfigureD6Joint();
	}

	if (bToggleLowerActorGravity)
	{
		char ds[512];

        bToggleLowerActorGravity = false;
        if (capsule2->readBodyFlag(NX_BF_DISABLE_GRAVITY))
            capsule2->clearBodyFlag(NX_BF_DISABLE_GRAVITY);   
        else
            capsule2->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);

		// Set lower actor gravity in HUD
		sprintf(ds, "Lower Actor Gravity: %s", gOnOffString[!capsule2->readBodyFlag(NX_BF_DISABLE_GRAVITY)]);
		hud.SetDisplayString(11, ds, 0.015f, 0.47f);
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
		case 'g': { bToggleLowerActorGravity = true; break; }
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

	d6Desc.xMotion = NX_D6JOINT_MOTION_LIMITED;
	d6Desc.yMotion = NX_D6JOINT_MOTION_LIMITED;
	d6Desc.zMotion = NX_D6JOINT_MOTION_LIMITED;

	d6Desc.projectionMode = NX_JPM_NONE;
//	d6Desc.projectionMode = NX_JPM_POINT_MINDIST;

//    d6Desc.flags = NX_D6JOINT_SLERP_DRIVE;

//    d6Desc.slerpDrive.driveType = NX_D6JOINT_DRIVE_POSITION;
//    d6Desc.slerpDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//    d6Desc.slerpDrive.forceLimit = FLT_MAX;

//    d6Desc.slerpDrive.spring = 200;
//    d6Desc.slerpDrive.damping = 100;


//	d6Desc.xDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//	d6Desc.yDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//	d6Desc.zDrive.driveType = NX_D6JOINT_DRIVE_VELOCITY;
//	d6Desc.zDrive.forceLimit = FLT_MAX;

//	d6Desc.xDrive.spring = 50;
//	d6Desc.xDrive.damping = 0.5;

//	d6Desc.yDrive.spring = 50;
//	d6Desc.yDrive.damping = 0.5;

//	d6Desc.zDrive.spring = 50;
//	d6Desc.zDrive.damping = 0.5;

//	d6Desc.drivePosition.set(0,0,1);
//	d6Desc.driveLinearVelocity.set(0,0,5);

//    NxQuat q;
//    q.fromAngleAxis(90, NxVec3(0,1,0));

//	d6Desc.driveOrientation = q; 	
//	d6Desc.driveAngularVelocity

    d6Desc.linearLimit.value = 0.8;
    d6Desc.linearLimit.restitution = 0;
    d6Desc.linearLimit.spring = 0;
    d6Desc.linearLimit.damping = 0;

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

	// Add lower actor gravity to HUD
	sprintf(ds, "Lower Actor Gravity: %s", gOnOffString[!capsule2->readBodyFlag(NX_BF_DISABLE_GRAVITY)]);
	hud.AddDisplayString(ds, 0.015f, 0.47f);
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
//	capsule2->setLinearDamping(0.1);
    capsule2->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);

	NxVec3 globalAnchor = NxVec3(0,5,0);
	NxVec3 globalAxis = NxVec3(0,1,0);
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
	InitGlut(argc, argv, "Lesson 216: 6DOF Joint Limits, Springs, and Motors");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
