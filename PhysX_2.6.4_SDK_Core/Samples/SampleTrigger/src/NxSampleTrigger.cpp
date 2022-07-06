// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Trigger Sample
// Description: This sample program shows how to use triggers in an application.
//
// Originally written by: Pierre Terdiman (01.01.04)
//
// ===============================================================================

#include <stdio.h>
#include <GL/glut.h>

// Physics code
#undef random
#include "NxPhysics.h"
#include "ErrorStream.h"

static bool				gPause = false;
static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static NxVec3			gDefaultGravity(0.0f, -9.81f, 0.0f);
static ErrorStream		gErrorStream;
static NxArray<NxI32>	gNbTouchedBodies;
static NxI32			gNbTriggers = 0;
struct KinematicActor {
	NxActor* actor;
	NxVec3 vel;
};
NxArray<KinematicActor>	gKinematicActors;

class TriggerCallback : public NxUserTriggerReport
	{
	public:
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		// other actor is a trigger too?
		if ((NxI32)(otherShape.getActor().userData) < 0)
			return;

		NxActor& triggerActor = triggerShape.getActor();
		NxI32 triggerNumber = -(NxI32)triggerActor.userData;
		NxI32 triggerIndex = triggerNumber - 1;

		if(status & NX_TRIGGER_ON_ENTER)
			{
			// A body entered the trigger area for the first time
			gNbTouchedBodies[triggerIndex]++;
			}
		if(status & NX_TRIGGER_ON_LEAVE)
			{
			// A body left the trigger area
			gNbTouchedBodies[triggerIndex]--;
			}
		NX_ASSERT(gNbTouchedBodies[triggerIndex]>=0);	//Shouldn't go negative
	}

	}myTriggerCallback;


// Create a static trigger
static void CreateTrigger(const NxVec3& pos, NxF32 size = 2, const NxVec3* initial_velocity=NULL, bool kinematic = 
false)
{
	// Our trigger is a cube
	NxBodyDesc triggerBody;
	NxBoxShapeDesc dummyShape;
	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions		= NxVec3(size, size, size);
	BoxDesc.shapeFlags		|= NX_TRIGGER_ENABLE;

	NxActorDesc ActorDesc;

	if(initial_velocity || kinematic) {
		if (initial_velocity) {
			triggerBody.linearVelocity = *initial_velocity;
		}
		if (kinematic) {
			triggerBody.flags |= NX_BF_KINEMATIC;
		}
		triggerBody.mass = 1;
		ActorDesc.body = &triggerBody;
		NxF32 sizeinc = 1.01f;
		dummyShape.dimensions.set(size*sizeinc, size*sizeinc, size*sizeinc);
		dummyShape.group = 1;
		ActorDesc.shapes.pushBack(&dummyShape);
	}

	ActorDesc.shapes.pushBack(&BoxDesc);
	ActorDesc.globalPose.t = pos;
	int thisNb = ++gNbTriggers;
	gNbTouchedBodies.pushBack(0);
	NX_ASSERT(gNbTouchedBodies.size() == gNbTriggers);
	gScene->setGroupCollisionFlag(1,0, false);
	gScene->setGroupCollisionFlag(1,1, false);
	gScene->setGroupCollisionFlag(1,2, true);
	ActorDesc.userData = (void*)(-thisNb);
	if (!ActorDesc.isValid()) {
		printf("Invalid ActorDesc\n");
		return;
	}
	NxActor* actor = gScene->createActor(ActorDesc);	// This is just a quick-and-dirty way to identify the trigger for rendering
	NX_ASSERT(actor != NULL);

	if (kinematic) {
		KinematicActor k;
		k.actor = actor;
		if (initial_velocity) {
			k.vel = *initial_velocity;
		} else {
			k.vel.set(0,0,0);
		}
		gKinematicActors.pushBack(k);
	}

	gScene->setUserTriggerReport(&myTriggerCallback);
}

static void CreateCube(const NxVec3& pos, int size=2, const NxVec3* initial_velocity=NULL, bool kinematic = false, bool Static = false)
{
	// Create body
	NxBodyDesc BodyDesc;
	BodyDesc.angularDamping	= 0.5f;
//	BodyDesc.maxAngularVelocity	= 10.0f;
	if(initial_velocity)	BodyDesc.linearVelocity = *initial_velocity;

	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions		= NxVec3(float(size), float(size), float(size));

	NxActorDesc ActorDesc;
//	ActorDesc.userData		= (void*)size;
	ActorDesc.shapes.pushBack(&BoxDesc);
	if (!Static)
		ActorDesc.body			= &BodyDesc;
	ActorDesc.density		= 10.0f;
	ActorDesc.globalPose.t  = pos;
	ActorDesc.userData = (void*)size;
	
	NxActor* actor = gScene->createActor(ActorDesc);

	if (kinematic) {
		KinematicActor k;
		k.actor = actor;
		actor->raiseBodyFlag(NX_BF_KINEMATIC);
		if (initial_velocity) {
			k.vel = *initial_velocity;
		} else {
			k.vel.set(0,0,0);
		}
		gKinematicActors.pushBack(k);
	}

}

static void CreateStack(int size)
{
	float CubeSize = 2.0f;
//	float Spacing = 0.01f;
	float Spacing = 0.0001f;
	NxVec3 Pos(0.0f, CubeSize, 0.0f);
	float Offset = -size * (CubeSize * 2.0f + Spacing) * 0.5f;
	while(size)
	{
		for(int i=0;i<size;i++)
		{
			Pos.x = Offset + float(i) * (CubeSize * 2.0f + Spacing);
			CreateCube(Pos, (int)CubeSize);
		}
		Offset += CubeSize;
		Pos.y += (CubeSize * 2.0f + Spacing);
		size--;
	}
}

static void CreateTower(int size)
{
	float CubeSize = 2.0f;
	float Spacing = 0.01f;
	NxVec3 Pos(0.0f, CubeSize, 0.0f);
	while(size)
	{
		CreateCube(Pos, (int)CubeSize);
		Pos.y += (CubeSize * 2.0f + Spacing);
		size--;
	}
}

static void InitNx()
{
	// Initialize PhysicsSDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, &gErrorStream);
	if(!gPhysicsSDK)	return;

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.025f);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity				= gDefaultGravity;

	gScene = gPhysicsSDK->createScene(sceneDesc);

	// Create ground plane
	NxPlaneShapeDesc PlaneDesc;
	PlaneDesc.group = 2;
	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&PlaneDesc);
	gScene->createActor(ActorDesc);


	NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);

	// Create trigger
	CreateTrigger(NxVec3(0,10,0), 10);
	CreateTrigger(NxVec3(40,10,0), 10, NULL, true);
	CreateTrigger(NxVec3(-40,10,0), 10, &NxVec3(0,0,0), false);
	CreateCube(NxVec3(-40,10,40), 10, NULL, false, true);
	CreateCube(NxVec3(-40,10,80), 10, NULL, true, false);
}


// Render code
static int gMainHandle;
static bool gShadows = true;
static NxVec3 Eye(90.0f, 70.0f, 80.0f);
static NxVec3 Dir(-0.81,-0.37,-0.45);
static NxVec3 N;
static int mx = 0;
static int my = 0;

static void KeyboardCallback(unsigned char key, int x, int y)
{
	bool trigger = false;
	
	switch (key)
	{
	case 27:	exit(0); break;
	case ' ':			CreateCube(NxVec3(0.0f, 30.0f, 0.0f), 1+(rand()&3)); break;
	case 's':			CreateStack(10); break;
	case 'b':			CreateStack(30); break;
	case 't':			CreateTower(30); break;
	case 'x':			gShadows = !gShadows; break;
	case 'p':			gPause = !gPause; break;
	case 201: case '8':	Eye += Dir * 2.0f; break;
	case 203: case '2':	Eye -= Dir * 2.0f; break;
	case 200: case '4':	Eye -= N * 2.0f; break;
	case 202: case '6':	Eye += N * 2.0f; break;
	case '1':
	case 'q':
		trigger = true;
	case 'a':
	case 'w':
		{
		bool kinematic = (key == '1' || key == 'a');
		NxVec3 t = Eye;
		NxVec3 Vel = Dir;
		Vel.normalize();
		Vel*=200.0f;
		if (trigger)
			{
			CreateTrigger(t, 8 ,&Vel, kinematic);
			}
		else
			{
			CreateCube(t, 8, &Vel, kinematic);
			}
		}
		break;
	}
}

static void ArrowKeyCallback(int key, int x, int y)
	{
	KeyboardCallback(100+key,x,y);
	}

static void MouseCallback(int button, int state, int x, int y)
{
	mx = x;
	my = y;
}

static void MotionCallback(int x, int y)
{
	int dx = mx - x;
	int dy = my - y;
	
	Dir.normalize();
	N.cross(Dir,NxVec3(0,1,0));

	NxQuat qx(NxPiF32 * dx * 20/ 180.0f, NxVec3(0,1,0));
	qx.rotate(Dir);
	NxQuat qy(NxPiF32 * dy * 20/ 180.0f, N);
	qy.rotate(Dir);

	mx = x;
	my = y;
}

static void RenderCallback()
{
	float glmat[16];
	// Physics code
	if(gScene && !gPause)	
		{
		for (NxU32 i = 0; i < gKinematicActors.size(); i++)
			{
			NxActor* actor = gKinematicActors[i].actor;
			NxVec3 pos = actor->getGlobalPosition();
			pos += gKinematicActors[i].vel * 1.f/60.f;
			actor->moveGlobalPosition(pos);
			}
		gScene->simulate(1.0f/60.0f);	//Note: a real application would compute and pass the elapsed time here.
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		}
	// ~Physics code

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT), 1.0f, 10000.0f);
	gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Keep physics & graphics in sync
	for (NxU32 pass = 0; pass < 2; pass++) {
		int nbActors = gScene->getNbActors();
		NxActor** actors = gScene->getActors();
		actors += nbActors;
		while(nbActors--)
		{
			NxActor* actor = *--actors;

			float size;
			bool isTrigger = false;
			bool isKinematic = actor->isDynamic() && actor->readBodyFlag(NX_BF_KINEMATIC);
			NxVec3 color;
			NxF32 alpha = 1;
			if (actor->isDynamic()) {
				if (actor->readBodyFlag(NX_BF_KINEMATIC)) {
					color.set(1,0,0);
				} else {
					color.set(0,1,0);
				}
			} else {
				color.set(0.2f,0.2f,0.2f);
			}

			if(int(actor->userData)<0)
			{
				NxI32 triggerNumber = -(NxI32)actor->userData;
				NxI32 triggerIndex = triggerNumber - 1;
				// This is our trigger
				isTrigger = true;

				size = 10.0f;
				color.z = gNbTouchedBodies[triggerIndex] > 0.5f ? 1.0f:0.0f;
				alpha = 0.5f;
				if (pass == 0)
					continue;
			}
			else
			{
				// This is a normal object
				size = float(int(actor->userData));
				if (pass == 1)
					continue;
			}

			glPushMatrix();
			actor->getGlobalPose().getColumnMajor44(glmat);
			glMultMatrixf(glmat);
			glColor4f(color.x, color.y, color.z, 1.0f);
			glutSolidCube(size*2.0f);
			glPopMatrix();

			// Handle shadows
			if(gShadows && !isTrigger)
			{
				glPushMatrix();

				const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

				glMultMatrixf(ShadowMat);
				glMultMatrixf(glmat);

				glDisable(GL_LIGHTING);
				glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
				glutSolidCube(size*2.0f);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glEnable(GL_LIGHTING);

				glPopMatrix();
			}
		}
	}

	glutSwapBuffers();
}

static void ReshapeCallback(int width, int height)
{
	glViewport(0, 0, width, height);
}

static void IdleCallback()
{
	glutPostRedisplay();
}

static void ExitCallback()
{
	if (gPhysicsSDK)
	{
		if (gScene) gPhysicsSDK->releaseScene(*gScene);
		gPhysicsSDK->release();
	}
}

int main(int argc, char** argv)
{
	printf("Press the keys space,s,b, and t to create various things.\nThe trigger box changes color if anything touches it.\n");
	printf("Use the arrow keys or 2, 4, 6 and 8 to move the camera, mouse to look around.\n");
	printf("Press p to pause the simulation.\n");
	printf("Press the following keys for shooting different Cubes:\n");
	printf(" w - dynamic cube\n");
	printf(" a - kinematic cube\n");
	printf(" q - dynamic trigger\n");
	printf(" 1 - kinematic trigger\n");
#if defined(_XBOX) | defined(__CELLOS_LV2__)
	glutRemapButtonExt(0, 'w', false); // A/X to dynamic cube
	glutRemapButtonExt(1, 'a', false); // B/O to kinematic cube
	glutRemapButtonExt(2, 'q', false); // X/S to dynamic trigger
	glutRemapButtonExt(3, '1', false); // Y/T to kinematic trigger
#endif
	// Initialize Glut
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("SampleTrigger");
	glutSetWindow(gMainHandle);
	glutDisplayFunc(RenderCallback);
	glutReshapeFunc(ReshapeCallback);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(KeyboardCallback);
	glutSpecialFunc(ArrowKeyCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
	MotionCallback(0,0);

	atexit(ExitCallback);

	// Setup default render states
	glClearColor(0.3f, 0.4f, 0.5f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);

	// Setup lighting
	glEnable(GL_LIGHTING);
	float AmbientColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	float DiffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	float SpecularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[] = { 100.0f, 100.0f, 400.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	// Physics code
	InitNx();
	// ~Physics code

	// Run
	glutMainLoop();
	return 0;
}
