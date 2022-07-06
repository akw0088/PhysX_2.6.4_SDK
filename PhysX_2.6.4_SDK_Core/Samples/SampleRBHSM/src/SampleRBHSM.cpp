// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Sample of RB HSM
// Description: This sample program shows how to create some simple dynamic objects
//				in a scene and in an attached managed hardware scene, and how they
//				interact
//
// ===============================================================================

#include <stdio.h>
#include <GL/glut.h>

#include "NxPhysics.h"
#include "ErrorStream.h"
#include "PerfRenderer.h"
#include "DebugRenderer.h"

// Physics
static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static NxCompartment * gCompartment = NULL;
static PerfRenderer		gPerfRenderer;

// Rendering
static NxVec3	gEye(50.0f, 50.0f, 50.0f);
static NxVec3	gDir(-0.6f,-0.2f,-0.7f);
static NxVec3	gViewY;
static int		gMouseX = 0;
static int		gMouseY = 0;
static const unsigned int HW_ACTOR = (1<<31); //Flag set in userdata to identify HW actors

static bool		gWarnedAboutHardware = false;

static void InitNx()
{
	// Initialize PhysicsSDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, new ErrorStream());
	if(gPhysicsSDK == NULL) return;

	NxHWVersion hwCheck = gPhysicsSDK->getHWVersion();
	if (hwCheck == NX_HW_VERSION_NONE) {
		printf("Unable to find any AGEIA PhysX hardware, this sample requires hardware physics to run. Please make sure that you have correctly installed the latest version of the AGEIA PhysX SDK.\n");
		exit(1);
	}

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05f);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity				= NxVec3(0.0f, -9.81f, 0.0f);

	gScene = gPhysicsSDK->createScene(sceneDesc);

	if(gScene == NULL) return;

	//Create a managed hardware scene attached to this scene
	NxCompartmentDesc cdesc;
	cdesc.type = NX_SCT_RIGIDBODY;
	cdesc.deviceCode = NX_DC_PPU_AUTO_ASSIGN;
	gCompartment = gScene->createCompartment(cdesc);

	// Set default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0);
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);

	// Create ground plane
	NxPlaneShapeDesc planeDesc;
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);
	gScene->createActor(actorDesc);
}

static void ExitNx()
{
	if(gPhysicsSDK != NULL)
	{
		if(gScene != NULL) gPhysicsSDK->releaseScene(*gScene);

		gPhysicsSDK->release();
	}

	exit(0);
}

static void CreateCube(const NxVec3& pos, int size=2, const NxVec3* initialVelocity=NULL, bool hwScene = false)
{
	if(gScene == NULL) return;	

	// Create body
	NxBodyDesc bodyDesc;
	bodyDesc.angularDamping	= 0.5f;
	if(initialVelocity) bodyDesc.linearVelocity = *initialVelocity;

	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = NxVec3((float)size, (float)size, (float)size);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 10.0f;
	actorDesc.globalPose.t  = pos;

	size_t userData = size_t(size);

	if(hwScene)
	{
		// If flagged to be created in hw-scene, set the actor flag
		actorDesc.compartment = gCompartment;

		// To identify the actor when render it, set a bit in the userdata
		userData |= HW_ACTOR;
	}

	NxActor* actor = gScene->createActor(actorDesc);
	if (!actor) 
	{
		if (hwScene) 
		{
			if (!gWarnedAboutHardware) printf("\nUnable to create a hardware scene actor. Please make sure that you have correctly installed the latest version of the AGEIA PhysX SDK.\n\n");
			gWarnedAboutHardware = true;
		}
		return;
	}
	actor->userData = (void*)userData;
	//printf("Total: %d actors\n", gScene->getNbActors());
}

static void CreateCubeFromEye(int size, bool hwScene = false)
{
	NxVec3 t = gEye;
	NxVec3 vel = gDir;
	vel.normalize();
	vel*=200.0f;
	CreateCube(t, size, &vel, hwScene);
}

static void CreateStack(int size, const NxVec3& offset, bool hwScene = false)
{
	const float cubeSize = 2.0f;
	const float spacing = 0.0001f;
	NxVec3 pos(0.0f, cubeSize, 0.0f);
	float boxoffset = -size * (cubeSize * 2.0f + spacing) * 0.5f;
	while(size)
	{
		for(int i=0;i<size;i++)
		{
			pos.x = boxoffset + (float)i * (cubeSize * 2.0f + spacing);
			CreateCube(pos + offset, (int)cubeSize, NULL, hwScene);
		}

		boxoffset += cubeSize;
		pos.y += (cubeSize * 2.0f + spacing);
		size--;
	}
}

static void CreateTower(int size, const NxVec3& offset, bool hwScene = false)
{
	const float cubeSize = 2.0f;
	const float spacing = 0.01f;
	NxVec3 pos(0.0f, cubeSize, 0.0f);
	while(size)
	{
		CreateCube(pos + offset, (int)cubeSize, NULL, hwScene);
		pos.y += (cubeSize * 2.0f + spacing);
		size--;
	}
}

static void KeyboardCallback(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27:	
			ExitNx(); 
			break;
		case '0':
			gPerfRenderer.toggleEnable();
			break;
		case 't':	
		case 'T':
			CreateCube(NxVec3(0.0f, 20.0f, 0.0f),1); 
			break;
		case 'g':
		case 'G':
			CreateCube(NxVec3(0.0f, 20.0f, 0.0f),1, NULL, true);
			break;
		case 'y':
		case 'Y':
			CreateStack(10, NxVec3(0.0f, 0.0f, 10.0f)); 
			break;
		case 'h':
		case 'H':
			CreateStack(10, NxVec3(0.0f, 0.0f, -10.0f), true); 
			break;
		
		case 'u':
		case 'U':
			CreateStack(30, NxVec3(0.0f, 0.0f, 10.0f)); 
			break;
		case 'j':
		case 'J':
			CreateStack(30, NxVec3(0.0f, 0.0f, -10.0f), true); 
			break;

		case 'i':
		case 'I':
			CreateTower(10, NxVec3(0.0f, 0.0f, 10.0f)); 
			break;
		case 'k':
		case 'K':
			CreateTower(10, NxVec3(0.0f, 0.0f, -10.0f), true); 
			break;

		case 'o':
		case 'O':
			CreateCubeFromEye(8);
			break;

		case 'l':
		case 'L':
			CreateCubeFromEye(8, true);
			break;

		case 'q':
		case 'Q':
			{
				NxActor** actors = gScene->getActors();
				if(gScene->getNbActors() > 1){
					gScene->releaseActor(*actors[gScene->getNbActors()-1]);
				}
			}
			break;	

		//Navigation keys
		case 'w':	
		case '8':	
			gEye += gDir*2.0f; 
			break;
		case 's': 
		case '2':
			gEye -= gDir*2.0f; 
			break;
		case 'a':	
		case '4':	
			gEye -= gViewY*2.0f; 
			break;
		case 'd': 
		case '6':	
			gEye += gViewY*2.0f; 
			break;
	}
}

static void ArrowKeyCallback(int key, int x, int y)
{
	KeyboardCallback(key,x,y);
}

static void MouseCallback(int button, int state, int x, int y)
{
	gMouseX = x;
	gMouseY = y;
}

static void MotionCallback(int x, int y)
{
	int dx = gMouseX - x;
	int dy = gMouseY - y;
	
	gDir.normalize();
	gViewY.cross(gDir, NxVec3(0,1,0));

	NxQuat qx(NxPiF32 * dx * 20/ 180.0f, NxVec3(0,1,0));
	qx.rotate(gDir);
	NxQuat qy(NxPiF32 * dy * 20/ 180.0f, gViewY);
	qy.rotate(gDir);

	gMouseX = x;
	gMouseY = y;
}

static void RenderCallback()
{
	if(gScene == NULL) return;
	
	// Start simulation (non blocking)
	gScene->simulate(1.0f/60.0f);

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT), 1.0f, 10000.0f);
	gluLookAt(gEye.x, gEye.y, gEye.z, gEye.x + gDir.x, gEye.y + gDir.y, gEye.z + gDir.z, 0.0f, 1.0f, 0.0f);

	// Setup modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Render all actors
	int nbActors = gScene->getNbActors();
	NxActor** actors = gScene->getActors();
	while(nbActors--)
	{
		NxActor* actor = *actors++;
		if(!actor->userData) continue;

		size_t userData = size_t(actor->userData);

		if (userData & HW_ACTOR)
		{
			// Make HW-actors green
			glColor4f(0.5f, 1.0f, 0.5f, 1.0f);	
			userData &= ~HW_ACTOR; //clear flag for rendering code below
		}
		else
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Render actor
		glPushMatrix();
		float glMat[16];
		actor->getGlobalPose().getColumnMajor44(glMat);
		glMultMatrixf(glMat);
		glutSolidCube(float(userData)*2.0f);
		glPopMatrix();

		// Render shadow
		glPushMatrix();
		const static float shadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
		glMultMatrixf(shadowMat);
		glMultMatrixf(glMat);
		glDisable(GL_LIGHTING);
		glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
		glutSolidCube(float(userData)*2.0f);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	// Fetch simulation results
	gScene->flushStream();
	gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
	
	// Print profile results (if enabled)
	gPerfRenderer.render(gScene->readProfileData(true), glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

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
	// Initialize glut
	printf("HSM RB Sample\n"
		"Create software objects: \n"
		"[t] Single cube\n"
		"[y] Pyramid with 55 cubes\n"
		"[u] Pyramid with 465 cubes\n"
		"[i] Tower with 10 cubes\n"
		"[o] Shoot cube from camera into scene\n"
		"\n"
		"Create hardware objects: \n"
		"[g] Single cube\n"
		"[h] Pyramid with 55 cubes\n"
		"[j] Pyramid with 465 cubes\n"
		"[k] Tower with 10 cubes\n"
		"[l] Shoot cube from camera into scene\n"
		"\n"
		"[0] Toggle profiler\n"
		"\n"
		);
	printf("Use the w, a, s, d or 8, 4, 2 and 6 to move the camera.\n");
	printf("Use the mouse to rotate the camera.\n");
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	int mainHandle = glutCreateWindow("SampleRBHSM");
	glutSetWindow(mainHandle);
	glutDisplayFunc(RenderCallback);
	glutReshapeFunc(ReshapeCallback);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(KeyboardCallback);
	glutSpecialFunc(ArrowKeyCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
	MotionCallback(0,0);

	// Setup default render states
	glClearColor(0.3f, 0.4f, 0.5f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	// Setup lighting
	glEnable(GL_LIGHTING);
	float ambientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };
	float diffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };		
	float specularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };		
	float position[]		= { 100.0f, 100.0f, 400.0f, 1.0f };		
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHT0);

	// Init Physics
	InitNx();

	// Run
	glutMainLoop();

	return 0;
}
