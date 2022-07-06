// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: AI Sensor
// Description: This sample program shows the use of a sensor for guiding a simple
//              AI when moving around with an actor.
//
// ===============================================================================

#include <stdio.h>
#include <GL/glut.h>

// Physics code
#include "NxPhysics.h"
#include "ErrorStream.h"
#include "Timing.h"
#include "DebugRenderer.h"

static DebugRenderer	gDebugRenderer;
static bool				gPause = false;
static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static NxVec3			gDefaultGravity(0.0f, -98.1f, 0.0f);
static ErrorStream		gErrorStream;

// Render code
static int gMainHandle;
static bool gShadows = true;
static NxVec3 Eye(50.0f, 50.0f, 50.0f);
static NxVec3 Dir(-0.6,-0.2,-0.7);
static NxVec3 N;
static int mx = 0;
static int my = 0;

struct MyObject
{
	int		size;
	NxU32	nbTouched;
};

class SensorReport : public NxUserTriggerReport
{
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		NX_ASSERT(triggerShape.getFlag(NX_TRIGGER_ENABLE));
		if(status & NX_TRIGGER_ON_ENTER)
		{
			NxActor& actor = otherShape.getActor();
			if(actor.userData)
			{
				NxActor& triggerActor = triggerShape.getActor();
				if(triggerActor.isDynamic() && triggerActor.userData != NULL)
				{
					MyObject* Object = (MyObject*)triggerActor.userData;
					Object->nbTouched++;
				}
			}
		}
		if(status & NX_TRIGGER_ON_LEAVE)
		{
			NxActor& actor = otherShape.getActor();
			if(actor.userData)
			{
				NxActor& triggerActor = triggerShape.getActor();
				if(triggerActor.isDynamic() && triggerActor.userData != NULL)
				{
					MyObject* Object = (MyObject*)triggerActor.userData;
					Object->nbTouched--;
				}
			}
		}
	}
}gMySensorReport;

static void CreateCube(const NxVec3& pos, int size=2, const NxVec3* initial_velocity=NULL, bool isStatic=false)
{
	// Create body
	NxBodyDesc BodyDesc;
	BodyDesc.angularDamping	= 0.5f;
	if(initial_velocity)	BodyDesc.linearVelocity = *initial_velocity;

	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions		= NxVec3(float(size), float(size), float(size));

	MyObject* Object = new MyObject;
	Object->size			= size;
	Object->nbTouched		= 0;

	NxActorDesc ActorDesc;
	ActorDesc.userData		= Object;
	ActorDesc.shapes.pushBack(&BoxDesc);

	NxBoxShapeDesc SensorDesc;
	if(!isStatic)
	{
		BodyDesc.flags			|= NX_BF_FROZEN_ROT_X|NX_BF_FROZEN_ROT_Z;
		BodyDesc.linearDamping	= 1.0f;
		BodyDesc.angularDamping	= 10.0f;

		SensorDesc.dimensions	= NxVec3(float(size), float(size), 10.0f);
		SensorDesc.localPose.t	= NxVec3(0.0f, 0.0f, -10.0f);
		SensorDesc.shapeFlags	|= NX_TRIGGER_ENABLE;
		ActorDesc.shapes.pushBack(&SensorDesc);
	}

	if(!isStatic)	ActorDesc.body	= &BodyDesc;
	ActorDesc.density		= 10.0f;
	ActorDesc.globalPose.t  = pos;
	NxActor* newActor = gScene->createActor(ActorDesc);
}

static void InitNx()
{
	// Initialize PhysicsSDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, &gErrorStream);
	if(!gPhysicsSDK) return;

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.025f);

	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 2.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_POINT, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_NORMAL, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_FORCE, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_LIN_VELOCITY, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_ANG_FORCE, 1);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity				= gDefaultGravity;
	sceneDesc.userTriggerReport		= &gMySensorReport;
	gScene = gPhysicsSDK->createScene(sceneDesc);
	gScene->setTiming(1.0f/60.0f, 1, NX_TIMESTEP_FIXED);


	NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.0f);
	defaultMaterial->setDynamicFriction(0.0f);

	// Create ground plane
	NxPlaneShapeDesc PlaneDesc;
	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&PlaneDesc);
	gScene->createActor(ActorDesc);

	// Create walls
	const int NB=20;
	const int SIZE=3;
	const float MAXSIZE=NB*SIZE*2.0f;
	unsigned int i;
	for(i=0;i<NB;i++)
	{
		CreateCube(NxVec3((NxF32(i)/NxF32(NB-1))*MAXSIZE - MAXSIZE*0.5f, NxF32(SIZE), MAXSIZE*0.5f), SIZE, NULL, true);
	}

	for(i=0;i<NB;i++)
	{
		CreateCube(NxVec3((NxF32(i)/NxF32(NB-1))*MAXSIZE - MAXSIZE*0.5f, NxF32(SIZE), -MAXSIZE*0.5f), SIZE, NULL, true);
	}
	for(i=0;i<NB;i++)
	{
		CreateCube(NxVec3(-MAXSIZE*0.5f, NxF32(SIZE), (NxF32(i)/NxF32(NB-1))*MAXSIZE - MAXSIZE*0.5f), SIZE, NULL, true);
	}

	for(i=0;i<NB;i++)
	{
		CreateCube(NxVec3(MAXSIZE*0.5f, NxF32(SIZE), (NxF32(i)/NxF32(NB-1))*MAXSIZE - MAXSIZE*0.5f), SIZE, NULL, true);
	}

	CreateCube(NxVec3(0.0f, 2.0f, 0.0f), 2);
}

static void ExitCallback()
{
	if(gPhysicsSDK && gScene) gPhysicsSDK->releaseScene(*gScene);
	gPhysicsSDK->release();
}

static void KeyboardCallback(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27:
			exit(0);
			break;
		case ' ':
			CreateCube(NxVec3(0.0f, 20.0f, 0.0f), 1+(rand()&3));
			break;
		case 'p':
		case 'P':
			gPause = !gPause;
			break;
		case 101: case '8':	Eye += Dir * 2.0f; break;
		case 103: case '2':	Eye -= Dir * 2.0f; break;
		case 100: case '4':	Eye -= N * 2.0f; break;
		case 102: case '6':	Eye += N * 2.0f; break;
	}
}

static void ArrowKeyCallback(int key, int x, int y)
{
	KeyboardCallback(key,x,y);
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
	if(!gScene)	return;

	NxF32 elapsedTime = getElapsedTime();
	
	// Physics code
	if(!gPause)	
		{
		if (elapsedTime != 0.0f)
			{
			gScene->simulate(elapsedTime);
			gScene->flushStream();
			gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
			}
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
	int nbActors = gScene->getNbActors();
	NxActor** actors = gScene->getActors();
	while(nbActors--)
	{
		NxActor* actor = *actors++;
		if(actor->userData != NULL)
		{

			MyObject* Object = (MyObject*)actor->userData;
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			if(actor->isDynamic() && !gPause)
			{
				if(!Object->nbTouched)
				{
					actor->wakeUp();
					actor->addLocalForceAtLocalPos(NxVec3(0.0f, 0.0f, -100.0f), NxVec3(0.0f, 0.0f, 0.0f), NX_IMPULSE);
					actor->setLinearDamping(1.0f);
				}
				else
				{
					NxVec3 localPos(0.0f, 0.0f, 10.0f);
					NxVec3 localForce(10000.0f, 0.0f, 0.0f);
					actor->addLocalForceAtLocalPos(localForce, localPos);
					actor->setLinearDamping(5.0f);

					glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				}
			}

			glPushMatrix();
			float glmat[16];
			actor->getGlobalPose().getColumnMajor44(glmat);
			glMultMatrixf(glmat);
			glutSolidCube(float(Object->size)*2.0f);
			glPopMatrix();

			// Handle shadows
			if(gShadows)
			{
				glPushMatrix();

				const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

				glMultMatrixf(ShadowMat);
				glMultMatrixf(glmat);

				glDisable(GL_LIGHTING);
				glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
				glutSolidCube(float(Object->size)*2.0f);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glEnable(GL_LIGHTING);

				glPopMatrix();
			}
		}
	}
	gDebugRenderer.renderData(*gScene->getDebugRenderable());

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

int main(int argc, char** argv)
{
	printf("Use the arrow keys or 2, 4, 6 and 8 to move the camera.\n");
	printf("Use the mouse to rotate the camera.\n");
	printf("Press p to pause the simulation.\n");
	printf("Press spacebar to create a new sensor.\n");

	// Initialize Glut
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("SampleAISensor");
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
	float AmbientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	float DiffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	float SpecularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[]		= { 100.0f, 100.0f, 400.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	// Physics code
	InitNx();
	// ~Physics code

	// Run
	glutMainLoop();
	
	return 0;
}
