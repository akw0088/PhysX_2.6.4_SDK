// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                 LESSON 704: ALL WHEEL DRIVE
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "Lesson704.h"
#include "UpdateTime.h"

#include "NxWheel.h"

#include "UserData.h"
#include "HUD.h"

#include "GLTexture.h"
#include "Model_3DS.h"

#include "CookASE.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;
NxVec3            gDefaultGravity(0,-9.8,0);

// User report globals
DebugRenderer     gDebugRenderer;
UserAllocator*	  gAllocator = NULL;
ErrorStream       gErrorStream;

// HUD globals
HUD hud;
bool bSetCurrentSteerAngle = false;
bool bSetCurrentMotorTorque = false;
bool bSetCurrentBrakeTorque = false;
NxReal gCurrentSteerAngle = 0;
int gCurrentMotorTorque = 0;
int gCurrentBrakeTorque = 0;

// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,5,-15);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
NxReal gCameraSpeed = 10;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 50000;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Simulation globals
NxReal gDeltaTime = 1.0/60.0;
bool bHardwareScene = false;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

// Home Directory
char* rdir;
char returnDir[] = "../../Chapter7_Vehicles/Lesson704_All_Wheel_Drive/compiler/vc7";
char returnDir1[] = "../Chapter7_Vehicles/Lesson704_All_Wheel_Drive/compiler/vc7";
char returnDir2[] = "../../../../bin/win32";
char returnDir3[] = "../../../..";

// ASE Data Directory Paths (solution, binary, install)
char ASEfname[] = "../../../../Data/ase";
char ASEfname1[] = "../../../Data/ase";
char ASEfname2[] = "../../TrainingPrograms/Programs/Data/ase";
char ASEfname3[] = "TrainingPrograms/Programs/Data/ase";

// 3DS Data Directory Paths (solution, binary, install)
char TDSfname[] = "../3ds";

// Actor globals
NxActor* groundPlane = NULL;
NxActor* box = NULL;
NxActor* dome = NULL;
NxActor* buggy = NULL;

// Wheel shape globals
NxWheelShape* wheel1 = NULL;
NxWheelShape* wheel2 = NULL;
NxWheelShape* wheel3 = NULL;
NxWheelShape* wheel4 = NULL;

// Wheel material globals
NxMaterial* wsm = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n");
	printf("\n Special:\n --------\n 1 = Steer Left, 2 = Center Steering Wheel, 3 = Steer Right\n 8 = Forward, 9 = Brake, 0 = Reverse\n");
}

bool IsSelectable(NxActor* actor)
{
   NxShape*const* shapes = gSelectedActor->getShapes();
   NxU32 nShapes = gSelectedActor->getNbShapes();
   while (nShapes--)
   {
       if (shapes[nShapes]->getFlag(NX_TRIGGER_ENABLE)) 
       {           
           return false;
       }
   }

   if (actor == groundPlane)
       return false;

   return true;
}

void SelectNextActor()
{
   NxU32 nbActors = gScene->getNbActors();
   NxActor** actors = gScene->getActors();
   for (NxU32 i = 0; i < nbActors; i++)
   {
       if (actors[i] == gSelectedActor)
       {
           NxU32 j = 1;
           gSelectedActor = actors[(i+j)%nbActors];
           while (!IsSelectable(gSelectedActor))
           {
               j++;
               gSelectedActor = actors[(i+j)%nbActors];
           }
           break;
       }
   }
}

void ProcessCameraKeys()
{
	// Process camera keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Camera controls
			case 'w':{ gCameraPos += gCameraForward*gCameraSpeed*gDeltaTime; break; }
			case 's':{ gCameraPos -= gCameraForward*gCameraSpeed*gDeltaTime; break; }
			case 'a':{ gCameraPos -= gCameraRight*gCameraSpeed*gDeltaTime; break; }
			case 'd':{ gCameraPos += gCameraRight*gCameraSpeed*gDeltaTime; break; }
			case 'z':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed*gDeltaTime; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed*gDeltaTime; break; }
		}
	}
}

void SetupCamera()
{
	// Setup camera
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;
	    Draw3DSMeshActor(actor);
//		DrawActor(actor, gSelectedActor, true);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, true);
        }
    }
}

void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color)
{
	// Draw only if the force is large enough
	NxReal force = forceVec.magnitude();
	if (force < 0.1)  return;

	forceVec = 3*forceVec/force;

	NxVec3 pos = actor->getCMassGlobalPosition();
	DrawArrow(pos, pos + forceVec, color);
}

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode)
{
	NxVec3 forceVec = forceStrength*forceDir*gDeltaTime;

	if (forceMode)
		actor->addForce(forceVec);
	else 
		actor->addTorque(forceVec);

	return forceVec;
}

void ProcessForceKeys()
{
	// Process force keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Force controls
			case 'i': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,+1),gForceStrength,bForceMode); break; }
			case 'k': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
			case 'j': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(+1,0,0),gForceStrength,bForceMode); break; }
			case 'l': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'u': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,+1,0),gForceStrength,bForceMode); break; }
			case 'm': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }

	        // Front wheel
		    case '1':
            { 
                gCurrentSteerAngle += NxPi*0.00015;
				bSetCurrentSteerAngle = true;
                break; 
            }

	    	case '2':
            { 
                gCurrentSteerAngle = 0;
 				bSetCurrentSteerAngle = true;
                break; 
            }

	    	case '3':
            { 
                gCurrentSteerAngle -= NxPi*0.00015;
				bSetCurrentSteerAngle = true;
                break; 
            }

	        // Rear wheels
	    	case '8':
            { 
                gCurrentMotorTorque += 1;
				gCurrentBrakeTorque = 0;
 				bSetCurrentMotorTorque = true;
 				bSetCurrentBrakeTorque = true;
                break; 
            }

		    case '9':
            { 
                gCurrentMotorTorque = 0;
 				gCurrentBrakeTorque = 650;
 				bSetCurrentMotorTorque = true;
 				bSetCurrentBrakeTorque = true;
                break; 
            }

	    	case '0':
            { 
                gCurrentMotorTorque -= 1;
 				gCurrentBrakeTorque = 0;
 				bSetCurrentMotorTorque = true;
 				bSetCurrentBrakeTorque = true;
                break; 
            }
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
					}
					else
					{
						pose.t = p0 + dir * st;
					}

					NxMat33 rot, axisRot;
					rot.rotY(steerAngle);
					axisRot.rotY(0);

//					NxReal rollAngle = ((ShapeUserData*)(wheel->userData))->rollAngle;

					NxMat33 rollRot;
					rollRot.rotX(rollAngle);

					pose.M = rot * pose.M * axisRot * rollRot;

					sud->wheelShapePose = pose;
				}
			}
		}
	}
}

void ProcessInputs()
{
	UpdateWheelShapeUserData();

	ProcessForceKeys();

	if (bSetCurrentSteerAngle)
	{
		bSetCurrentSteerAngle = false;
        wheel1->setSteerAngle(gCurrentSteerAngle);
        wheel2->setSteerAngle(gCurrentSteerAngle);
		// Add front wheel steer angle to HUD
		char ds[512];
		sprintf(ds, "Front Wheel Steer Angle: %d", (int)(gCurrentSteerAngle*180.0/NxPi));
		hud.SetDisplayString(2, ds, 0.015f, 0.92f);
	}

	if (bSetCurrentMotorTorque)
	{
		bSetCurrentMotorTorque = false;
        wheel3->setMotorTorque(gCurrentMotorTorque);
        wheel4->setMotorTorque(gCurrentMotorTorque);
		// Add rear wheel motor torque to HUD
		char ds[512];
		sprintf(ds, "Rear Wheel Motor Torque: %d", gCurrentMotorTorque);
		hud.SetDisplayString(3, ds, 0.015f, 0.87f);
	}

	if (bSetCurrentBrakeTorque)
	{
		bSetCurrentBrakeTorque = false;
		wheel3->setBrakeTorque(gCurrentBrakeTorque);
		wheel4->setBrakeTorque(gCurrentBrakeTorque);				
		// Add rear wheel brake torque to HUD
		char ds[512];
		sprintf(ds, "Rear Wheel Brake Torque: %d", gCurrentBrakeTorque);
		hud.SetDisplayString(4, ds, 0.015f, 0.82f);
	}

    // Show debug wireframes
    if (bDebugWireframeMode) 
	{
		if (gScene)
		{
			gDebugRenderer.renderData(*gScene->getDebugRenderable());
		}
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
 	RenderActors(false);
	//RenderActors(bShadows);

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

void ReshapeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
    gCameraAspectRatio = float(width)/float(height);
}

void IdleCallback()
{
    glutPostRedisplay();
}

void KeyboardCallback(unsigned char key, int x, int y)
{
	gKeys[key] = true;

	switch (key)
	{
		case 'r': { SelectNextActor(); break; }
		default: { break; }
	}
}

void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

	switch (key)
	{
		case 'p': { bPause = !bPause; 
					if (bPause)
						hud.SetDisplayString(1, "Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
					else
						hud.SetDisplayString(1, "", 0.0f, 0.0f);	
					UpdateTime(); 
					break; }
		//case 'x': { bShadows = !bShadows; break; }
		case 'b': { bDebugWireframeMode = !bDebugWireframeMode; break; }		
		case 'f': { bForceMode = !bForceMode; break; }
		case 27 : { exit(0); break; }
		default : { break; }
	}
}

void SpecialCallback(int key, int x, int y)
{
	switch (key)
    {
		// Reset PhysX
		case GLUT_KEY_F10: ResetNx(); return; 
	}
}

void MouseCallback(int button, int state, int x, int y)
{
    mx = x;
    my = y;
}

void MotionCallback(int x, int y)
{
    int dx = mx - x;
    int dy = my - y;
    
    gCameraForward.normalize();
    gCameraRight.cross(gCameraForward,NxVec3(0,1,0));
    NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));

    qx.rotate(gCameraForward);
    NxQuat qy(NxPiF32 * dy * 20 / 180.0f, gCameraRight);
    qy.rotate(gCameraForward);

    mx = x;
    my = y;
}

void ExitCallback()
{
	ReleaseNx();
}

void InitGlut(int argc, char** argv, char* lessonTitle)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow(lessonTitle);
    glutSetWindow(gMainHandle);
    glutDisplayFunc(RenderCallback);
    glutReshapeFunc(ReshapeCallback);
    glutIdleFunc(IdleCallback);
    glutKeyboardFunc(KeyboardCallback);
    glutKeyboardUpFunc(KeyboardUpCallback);
	glutSpecialFunc(SpecialCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MotionCallback);
    MotionCallback(0,0);
	atexit(ExitCallback);

    // Setup default render states
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);

    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

NxWheelShape* AddWheelToActor(NxActor* actor, NxWheelDesc* wheelDesc)
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
	wheelShapeDesc.suspension.damper = 0;//wheelDesc->springDamping*heightModifier;
	wheelShapeDesc.suspension.targetValue = wheelDesc->springBias*heightModifier;

	wheelShapeDesc.radius = wheelDesc->wheelRadius;
	wheelShapeDesc.suspensionTravel = wheelDesc->wheelSuspension; 
	wheelShapeDesc.inverseWheelMass = 0.1;	//not given!? TODO

	wheelShapeDesc.lateralTireForceFunction.stiffnessFactor *= wheelDesc->frictionToSide;	
	wheelShapeDesc.longitudalTireForceFunction.stiffnessFactor *= wheelDesc->frictionToFront;	

    NxWheelShape* wheelShape = NULL;
	wheelShape = static_cast<NxWheelShape *>(actor->createShape(wheelShapeDesc));
	return wheelShape;
}

NxActor* CreateBuggy(const NxVec3& pos)
{
	NxActor* actor = CreateBox(pos + NxVec3(0,0.5,0), NxVec3(2,0.25,1), 10);

	NxTireFunctionDesc lngTFD;
	lngTFD.extremumSlip = 1.0f;
	lngTFD.extremumValue = 0.02f;
	lngTFD.asymptoteSlip = 2.0f;
	lngTFD.asymptoteValue = 0.01f;	
	lngTFD.stiffnessFactor = 1000000.0f;

	NxTireFunctionDesc latTFD;
	latTFD.extremumSlip = 1.0f;
	latTFD.extremumValue = 0.02f;
	latTFD.asymptoteSlip = 2.0f;
	latTFD.asymptoteValue = 0.01f;	
	latTFD.stiffnessFactor = 1000000.0f;

	NxTireFunctionDesc slipTFD;
	slipTFD.extremumSlip = 1.0f;
	slipTFD.extremumValue = 0.02f;
	slipTFD.asymptoteSlip = 2.0f;
	slipTFD.asymptoteValue = 0.01f;	
	slipTFD.stiffnessFactor = 100.0f;  

	NxTireFunctionDesc medTFD;
	medTFD.extremumSlip = 1.0f;
	medTFD.extremumValue = 0.02f;
	medTFD.asymptoteSlip = 2.0f;
	medTFD.asymptoteValue = 0.01f;	
	medTFD.stiffnessFactor = 10000.0f;  

	// Front left wheel
	NxWheelDesc wheelDesc;
    wheelDesc.wheelApproximation = 10;
	wheelDesc.wheelRadius = 0.5;
	wheelDesc.wheelWidth = 0.3;  // 0.1
	wheelDesc.wheelSuspension = 1.0;  
	wheelDesc.springRestitution = 100;
	wheelDesc.springDamping = 0.5;
	wheelDesc.springBias = 0;  
	wheelDesc.maxBrakeForce = 1;
	wheelDesc.position = NxVec3(1.5,0.5,-1.5);
	wheelDesc.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		                   NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT; 
    wheel1 = AddWheelToActor(actor, &wheelDesc);

	// Front right wheel
	NxWheelDesc wheelDesc2;
    wheelDesc2.wheelApproximation = 10;
	wheelDesc2.wheelRadius = 0.5;
	wheelDesc2.wheelWidth = 0.3;  // 0.1
	wheelDesc2.wheelSuspension = 1.0;  
	wheelDesc2.springRestitution = 100;
	wheelDesc2.springDamping = 0.5;
	wheelDesc2.springBias = 0;  
	wheelDesc2.maxBrakeForce = 1;
	wheelDesc2.position = NxVec3(1.5,0.5,1.5);
	wheelDesc2.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		                    NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT; 
    wheel2 = AddWheelToActor(actor, &wheelDesc2);

	// Rear left wheel
	NxWheelDesc wheelDesc3;
    wheelDesc3.wheelApproximation = 10;
	wheelDesc3.wheelRadius = 0.5;
	wheelDesc3.wheelWidth = 0.3;  // 0.1
	wheelDesc3.wheelSuspension = 1.0;  
	wheelDesc3.springRestitution = 100;
	wheelDesc3.springDamping = 0.5;
	wheelDesc3.springBias = 0;  
	wheelDesc3.maxBrakeForce = 1;
	wheelDesc3.position = NxVec3(-1.5,0.5,-1.5);
	wheelDesc3.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		                    NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT; 
    wheel3 = AddWheelToActor(actor, &wheelDesc3);

	// Rear right wheel
	NxWheelDesc wheelDesc4;
    wheelDesc4.wheelApproximation = 10;
	wheelDesc4.wheelRadius = 0.5;
	wheelDesc4.wheelWidth = 0.3;  // 0.1
	wheelDesc4.wheelSuspension = 1.0;  
	wheelDesc4.springRestitution = 100;
	wheelDesc4.springDamping = 0.5;
	wheelDesc4.springBias = 0;  
	wheelDesc4.maxBrakeForce = 1;
	wheelDesc4.position = NxVec3(-1.5,0.5,1.5);
	wheelDesc4.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		                    NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT;
    wheel4 = AddWheelToActor(actor, &wheelDesc4);

    // LOWER THE CENTER OF MASS
    NxVec3 massPos = actor->getCMassLocalPosition();
    massPos.y -= 1;
    actor->setCMassOffsetLocalPosition(massPos);

	return actor;
}

void AttachModelToActor(NxActor* actor, char* file, NxU32 shapeNum, NxReal scale, NxReal angle = 0)
{
	NxShape*const* shapes = actor->getShapes();
	ShapeUserData* sud = (ShapeUserData*)(shapes[shapeNum]->userData);
	if (sud)
	{
	    sud->model = new Model_3DS;
		Model_3DS* m = (Model_3DS*)(sud->model);
	    m->Load(file);
	    m->scale = scale;
		Model_3DS::Vector v;
		v.x = 0; v.y = angle; v.z = 0;
		m->rot = v;
	}
}

void InitializeHUD()
{
	bHardwareScene = (gScene->getSimType() == NX_SIMULATION_HW);

	// Add hardware/software to HUD
	if (bHardwareScene)
	    hud.AddDisplayString("Hardware Scene", 0.74f, 0.92f);
	else
		hud.AddDisplayString("Software Scene", 0.74f, 0.92f);

	// Add pause to HUD
	if (bPause)  
		hud.AddDisplayString("Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
	else
		hud.AddDisplayString("", 0.0f, 0.0f);
}

void InitializeSpecialHUD()
{
	char ds[512];

	// Add front wheel steer angle to HUD
    sprintf(ds, "Front Wheel Steer Angle: %d", (int)(gCurrentSteerAngle*180.0/NxPi));
	hud.AddDisplayString(ds, 0.015f, 0.92f); 

	// Add rear wheel motor torque to HUD
    sprintf(ds, "Rear Wheel Motor Torque: %d", gCurrentMotorTorque);
	hud.AddDisplayString(ds, 0.015f, 0.87f);

	// Add rear wheel brake torque to HUD
    sprintf(ds, "Rear Wheel Brake Torque: %d", gCurrentBrakeTorque);
	hud.AddDisplayString(ds, 0.015f, 0.82f);
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator, &gErrorStream);
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
	NxMaterialDesc defaultMaterial;
	defaultMaterial.restitution = 0;
	defaultMaterial.staticFriction = 0.5;
	defaultMaterial.dynamicFriction	= 0.5;
	NxMaterial* m = gScene->getMaterialFromIndex(0);
	m->loadFromDesc(defaultMaterial);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	// Set the ASE directory
	int set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&ASEfname[0]); rdir = &returnDir[0];
	if (!set) { set = SetCurrentDirectory(&ASEfname1[0]); rdir = &returnDir1[0]; }
	if (!set) { set = SetCurrentDirectory(&ASEfname2[0]); rdir = &returnDir2[0]; }
	if (!set) { set = SetCurrentDirectory(&ASEfname3[0]); rdir = &returnDir3[0]; }
#elif LINUX
	set = chdir(&ASEfname[0]); rdir = &returnDir[0];
	if (set != 0) { set = chdir(&ASEfname2[0]); rdir = &returnDir2[0]; }
	if (set != 0) { set = chdir(&ASEfname3[0]); rdir = &returnDir3[0]; }
#endif

	NxInitCooking();
	dome = CookASE("dome.ase", gScene);

	// Page in the dome mesh to the hardware
	NxShape*const* shapes = dome->getShapes();
	for (unsigned int s = 0; s < dome->getNbShapes(); ++s )
	{
		NxShape* shape = (NxShape*)(shapes[s]);
		if(shape->is(NX_SHAPE_MESH))
		{
       		NxTriangleMeshShape* triMeshShape = (NxTriangleMeshShape *)shape;
       		NxTriangleMesh& triMesh	= triMeshShape->getTriangleMesh();
			int pageCnt = triMesh.getPageCount();
			for ( int t = 0; t < pageCnt; ++t )
			{
       			if (!triMeshShape->isPageInstanceMapped(t))
       			{
       				bool ok = triMeshShape->mapPageInstance(t);
     				assert(ok);
				}
			}
		}
	}

	// Set the 3DS directory
	set = 0;
#ifdef WIN32
	set = SetCurrentDirectory(&TDSfname[0]);
#elif LINUX
	set = chdir(&TDSfname[0]);
#endif

	box = CreateBox(NxVec3(5,0+30,0), NxVec3(0.75,0.75,0.75), 5);

	// Create the buggy actor
	buggy = CreateBuggy(NxVec3(0,3+30,0));
	buggy->wakeUp(1e30);
	gSelectedActor = buggy;

    AddUserDataToActors(gScene);

	// Set user data
	AttachModelToActor(box, "box.3ds", 0, 0.75);
	AttachModelToActor(dome, "dome.3ds", 0, 1);

	for (int i = 1; i < 5; i++)
	{
		// left, right, left, right - need to rotate left model
		if (i%2)
		    AttachModelToActor(buggy, "tire3.3ds", i, 0.5, 180);  // 0.2
		else
		    AttachModelToActor(buggy, "tire3.3ds", i, 0.5);   // 0.2
	}

	// Set current directory back to home directory
	SetCurrentDirectory(rdir);

    gCameraPos = NxVec3(0,10,-15);

	// Initialize HUD
	InitializeHUD();
	InitializeSpecialHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

void ReleaseNx()
{
	GetPhysicsResults();  // make sure to fetchResults() before shutting down
	if (gScene) gPhysicsSDK->releaseScene(*gScene);
	if (gPhysicsSDK)  gPhysicsSDK->release();
    NX_DELETE_SINGLE(gAllocator);
}

void ResetNx()
{
	ReleaseNx();
	InitNx();
}

void StartPhysics()
{
	// Update the time step
	gDeltaTime = UpdateTime();

	// Start collision and dynamics for delta time since the last frame
    gScene->simulate(gDeltaTime);
	gScene->flushStream();
}

void GetPhysicsResults()
{
	// Get results from gScene->simulate(gDeltaTime)
	while (!gScene->fetchResults(NX_RIGID_BODY_FINISHED, false));
}

int main(int argc, char** argv)
{
	PrintControls();
    InitGlut(argc, argv, "Lesson 704: All Wheel Drive");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
