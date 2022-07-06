// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			              LESSON 702: SUSPENSION AND TORQUE
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "Lesson702.h"
#include "UpdateTime.h"

#include "NxWheel.h"

#include "UserData.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;
NxVec3            gDefaultGravity(0,-9.8,0);

// User report globals
DebugRenderer     gDebugRenderer;
UserAllocator*	  gAllocator = NULL;
ErrorStream  	  gErrorStream;

// Actor globals
NxActor* board = NULL;

// Wheel shape globals
NxWheelShape* wheel1 = NULL;
NxWheelShape* wheel2 = NULL;

// Wheel material globals
NxMaterial* wsm = NULL;

// HUD globals
HUD hud;
bool bSetCurrentMotorTorque1 = false;
bool bSetCurrentMotorTorque2 = false;
int gCurrentMotorTorque1 = 0;
int gCurrentMotorTorque2 = 0;

// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;

NxVec3 gCameraPos(0,5,-15);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);

const NxReal gCameraSpeed = 0.01;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 10;
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

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/pml";
char fname1[] = "../../../Data/pml";
char fname2[] = "../../TrainingPrograms/Programs/Data/pml";
char fname3[] = "TrainingPrograms/Programs/Data/pml";

extern Array<const char *> nxContext;

class OutputStream : public NxUserOutputStream
{
public:
	void context(int &k)
	{
		if(nxContext.count)
		{
			String s("context: ");
			for(int i=0;i<nxContext.count;i++){
				if(i) s+="::";
				s+= nxContext[i];
			}
			printf("%s\n",(const char *)s);
		}
	}
	void reportError (NxErrorCode code, const char *message, const char *file, int line)
	{
		if (code < NXE_DB_INFO)
		{
			static int k=5;
			context(k);
			printf("Error: %s",message);
			k%=20;
		}
	}
	NxAssertResponse reportAssertViolation (const char *message,const char *file, int line)
	{
		assert(0);
		return NX_AR_CONTINUE;
	}
	void print (const char *message)
	{
		static int k=5;
		context(k);
		printf("%s\n", message);
		k%=20;
	}
} OutputStream;
NxUserOutputStream *gOutputStrem = &OutputStream;

// Actor globals
NxActor* groundPlane = NULL;
NxActor* box = NULL;
NxActor* sphere = NULL;
NxActor* capsule = NULL;
NxActor* pyramid = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
	printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Wheel Controls:\n --------------\n 1,2,3 = Left Wheel\n 8,9,0 = Right Wheel\n");
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
	for(NxU32 i = 0; i < nbActors; i++)
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
		case 'w':{ gCameraPos += gCameraForward*gCameraSpeed; break; }
		case 's':{ gCameraPos -= gCameraForward*gCameraSpeed; break; }
		case 'a':{ gCameraPos -= gCameraRight*gCameraSpeed; break; }
		case 'd':{ gCameraPos += gCameraRight*gCameraSpeed; break; }

		case 'z':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed; break; }
		case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed; break; }
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
		DrawActor(actor, gSelectedActor, true);

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
	NxVec3 forceVec = forceStrength*forceDir;

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
		case 'i': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,1),gForceStrength,bForceMode); break; }
		case 'k': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
		case 'j': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(1,0,0),gForceStrength,bForceMode); break; }
		case 'l': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
		case 'u': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,1,0),gForceStrength,bForceMode); break; }
		case 'm': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
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

	if (bSetCurrentMotorTorque1)
	{
		bSetCurrentMotorTorque1 = false;
		wheel1->setMotorTorque(gCurrentMotorTorque1);
		// Set motor torque #1 in HUD
		char ds[512];
		sprintf(ds, "Left wheel torque: %d", gCurrentMotorTorque1);
		hud.SetDisplayString(2, ds, 0.015f, 0.92f);
	}

	if (bSetCurrentMotorTorque2)
	{
		bSetCurrentMotorTorque2 = false;
		wheel2->setMotorTorque(gCurrentMotorTorque2);
		// Set motor torque #2 in HUD
		char ds[512];
		sprintf(ds, "Right wheel torque: %d", gCurrentMotorTorque2);
		hud.SetDisplayString(3, ds, 0.015f, 0.87f);
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
	case 'x': { bShadows = !bShadows; break; }
	case 'b': { bDebugWireframeMode = !bDebugWireframeMode; break; }		
	case 'f': { bForceMode = !bForceMode; break; }

			  // Left wheel
	case '1':
		{ 
			gCurrentMotorTorque1 += 100;
			bSetCurrentMotorTorque1 = true;
			break; 
		}

	case '2':
		{ 
			gCurrentMotorTorque1 = 0;
			bSetCurrentMotorTorque1 = true;
			break; 
		}

	case '3':
		{ 
			gCurrentMotorTorque1 -= 100;
			bSetCurrentMotorTorque1 = true;
			break; 
		}

		// Right wheel
	case '8':
		{ 
			gCurrentMotorTorque2 += 100;
			bSetCurrentMotorTorque2 = true;
			break; 
		}

	case '9':
		{ 
			gCurrentMotorTorque2 = 0;
			bSetCurrentMotorTorque2 = true;
			break; 
		}

	case '0':
		{ 
			gCurrentMotorTorque2 -= 100;
			bSetCurrentMotorTorque2 = true;
			break; 
		}

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
	wheelShapeDesc.suspension.damper = wheelDesc->springDamping*heightModifier;
	wheelShapeDesc.suspension.targetValue = wheelDesc->springBias*heightModifier;

	wheelShapeDesc.radius = wheelDesc->wheelRadius;
	wheelShapeDesc.suspensionTravel = wheelDesc->wheelSuspension; 
	wheelShapeDesc.inverseWheelMass = 0.1;	//not given!? TODO

	// wheelShapeDesc.lateralTireForceFunction = ;	//TODO
	// wheelShapeDesc.longitudalTireForceFunction = ;	//TODO

	NxWheelShape* wheelShape = NULL;
	wheelShape = static_cast<NxWheelShape *>(actor->createShape(wheelShapeDesc));
	return wheelShape;
}

NxActor* CreateBoard(const NxVec3& pos)
{
	NxActor* actor = CreateBox(pos + NxVec3(0,0.5,0), NxVec3(1,0.25,0.5), 10);
	actor->raiseBodyFlag(NX_BF_FROZEN_ROT_X);
	actor->setAngularDamping(0.5);

	// Left wheel
	NxWheelDesc wheelDesc;
	//wheelDesc.wheelAxis.set(0,0,1);
	//wheelDesc.downAxis.set(0,-1,0);
	wheelDesc.wheelApproximation = 10;
	wheelDesc.wheelRadius = 0.5;
	wheelDesc.wheelWidth = 0.1;
	wheelDesc.wheelSuspension = 0.5;
	//	wheelDesc.wheelSuspension = 1.0;
	wheelDesc.springRestitution = 7000;
	wheelDesc.springDamping = 0;
	wheelDesc.springBias = 0;  
	//	wheelDesc.springRestitution = 20;
	//	wheelDesc.springDamping = 0.5;
	//	wheelDesc.springBias = 0; 
	wheelDesc.maxBrakeForce = 1;
	wheelDesc.frictionToFront = 0.1;
	wheelDesc.frictionToSide = 0.99;
	wheelDesc.position = NxVec3(1.5,0.5,0);
	wheelDesc.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT;

	wheel1 = AddWheelToActor(actor, &wheelDesc);

	// Right wheel
	NxWheelDesc wheelDesc2;
	//wheelDesc2.wheelAxis.set(0,0,1);
	//wheelDesc2.downAxis.set(0,-1,0);
	wheelDesc2.wheelApproximation = 10;
	wheelDesc2.wheelRadius = 0.5;
	wheelDesc2.wheelWidth = 0.1;
	wheelDesc2.wheelSuspension = 0.5;
	//	wheelDesc2.wheelSuspension = 1.0;  
	wheelDesc2.springRestitution = 7000;
	wheelDesc2.springDamping = 0;
	wheelDesc2.springBias = 0;
	//	wheelDesc2.springRestitution = 20;
	//	wheelDesc2.springDamping = 0.5;
	//	wheelDesc2.springBias = 0;  
	wheelDesc2.maxBrakeForce = 1;
	wheelDesc2.frictionToFront = 0.1;
	wheelDesc2.frictionToSide = 0.99;
	wheelDesc2.position = NxVec3(-1.5,0.5,0);
	wheelDesc2.wheelFlags = NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | 
		NX_WF_AFFECTED_BY_HANDBRAKE | NX_WF_STEERABLE_INPUT;

	wheel2 = AddWheelToActor(actor, &wheelDesc2);

	return actor;
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

	// Add motor torque #1 to HUD
	sprintf(ds, "Left wheel torque: %d", gCurrentMotorTorque1);
	hud.AddDisplayString(ds, 0.015f, 0.92f); 

	// Add motor torque #2 to HUD
	sprintf(ds, "Right wheel torque: %d", gCurrentMotorTorque2);
	hud.AddDisplayString(ds, 0.015f, 0.87f);
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
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AXES, 1);

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

	NxU32 set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname1[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	// Create the default material
	NxMaterialDesc defaultMaterial;
	defaultMaterial.restitution		= 0;
	defaultMaterial.staticFriction	= 0.5;
	defaultMaterial.dynamicFriction	= 0.5;
	NxMaterial* m = gScene->getMaterialFromIndex(0);
	m->loadFromDesc(defaultMaterial);

	// Load the ramp scene
	nxmlLoadScene("Ramp.pml", gPhysicsSDK, gScene);

	// Switch from Max Coordinate System to
	// Training Program Coordinate System
	NxMat34 mat;
	NxMat33 orient;
	orient.setColumn(0, NxVec3(-1,0,0));
	orient.setColumn(1, NxVec3(0,0,1));
	orient.setColumn(2, NxVec3(0,1,0));
	mat.M = orient;
	SwitchCoordinateSystem(gScene, mat);

	// Reset wheel material
	wsm = NULL;

	// Create board actor
	board = CreateBoard(NxVec3(0,3,0));
	board->wakeUp(1e30);

	AddUserDataToActors(gScene);

	gSelectedActor = board;

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
	InitGlut(argc, argv, "Lesson 702: Suspension and Torque");
	InitNx();
	glutMainLoop();
	ReleaseNx();
	return 0;
}
