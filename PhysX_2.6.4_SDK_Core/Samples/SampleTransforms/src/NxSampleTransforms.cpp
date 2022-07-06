// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Transforms Sample
// Description: This sample program shows the reference frames of the AGEIA PhysX
//              SDK objects.
//
// Originally written by: Adam Moravanszky
//
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

// Physics code
#undef random
#include "NxPhysics.h"
#include "ErrorStream.h"
#include "cooking.h"
#include "Stream.h"
#include "DebugRenderer.h"
#include "Timing.h"
#include "UserAllocator.h"
#include "GLFontRenderer.h"
#include "MediaPath.h"

static ErrorStream		gErrorStream;
// Bunny data
extern unsigned int		BUNNY_NBVERTICES;
extern unsigned int		BUNNY_NBFACES;
extern float			gBunnyVertices[];
extern int				gBunnyTriangles[];
//slide show
extern void				tick(NxReal dt, bool click = false);

char displayString[512] = "...";

DebugRenderer gDebugRenderer;
//bunny related SDK variables
 NxTriangleMesh	* bunnyTriangleMesh;

bool				gPause = false;
NxPhysicsSDK*	gPhysicsSDK = NULL;
NxScene*			gScene = NULL;
NxVec3			gDefaultGravity(0.0f, -9.81f, 0.0f);
NxVec3*			gBunnyNormals = NULL;

int				gMainHandle;
bool				gShadows = true;
NxVec3			Eye(3.0f, 1.0f, 3.0f);
NxVec3			Dir(-0.6,-0.2,-0.7);
NxVec3			N;
int				mx = 0;
int				my = 0;

NxU32 getFileSize(const char* name)
{
	#ifndef SEEK_END
	#define SEEK_END 2
	#endif
	
	FILE* File = fopen(name, "rb");
	if(!File)
		return 0;

	fseek(File, 0, SEEK_END);
	NxU32 eof_ftell = ftell(File);
	fclose(File);
	return eof_ftell;
}

static void RenderBunny()
{

	static float* pVertList = NULL;
	static float* pNormList = NULL;

	if(pVertList == NULL && pNormList == NULL)
	{
		pVertList = new float[BUNNY_NBFACES*3*3];
		pNormList = new float[BUNNY_NBFACES*3*3];

		int vertIndex = 0;
		int normIndex = 0;
		const int* indices = gBunnyTriangles;
		for(unsigned int i=0;i<BUNNY_NBFACES;i++)
		{
			for(int j=0;j<3;j++)
			{
				int vref0 = *indices++;
						
				pVertList[vertIndex++] = gBunnyVertices[vref0*3+0];
				pVertList[vertIndex++] = gBunnyVertices[vref0*3+1];
				pVertList[vertIndex++] = gBunnyVertices[vref0*3+2];

				pNormList[normIndex++] = gBunnyNormals[vref0].x;
				pNormList[normIndex++] = gBunnyNormals[vref0].y;
				pNormList[normIndex++] = gBunnyNormals[vref0].z;
			}
		}
	}

	if(pVertList != NULL && pNormList != NULL)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT, 0, pVertList);
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, pNormList);
	

		glDrawArrays(GL_TRIANGLES, 0, BUNNY_NBFACES*3);
	
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		
	}
}


// Base wrapper object kept in NxActor's user-data field
class MyNXWrapper
{
		public:
							MyNXWrapper(NxActor* actor) : owner(actor)	{}
							~MyNXWrapper()								{}

		virtual		void	render()	= 0;

		NxActor*	owner;
};

// Cube wrapper
class MyNXCube : public MyNXWrapper
{
		public:
							MyNXCube(NxActor* actor, NxF32 s) : MyNXWrapper(actor), size(s)	{}
							~MyNXCube()														{}

		virtual		void	render()
							{
							float glmat[16];
							glPushMatrix();
							owner->getGlobalPose().getColumnMajor44(glmat);
							glMultMatrixf(glmat);
							glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
							glutSolidCube(size*2.0f);
							glPopMatrix();

							// Handle shadows
							if(gShadows)
								{
								glPushMatrix();

								const  static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

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

		NxF32			size;
};

// Sphere wrapper
class MyNXSphere : public MyNXWrapper
{
		public:
							MyNXSphere(NxActor* actor, NxF32 s) : MyNXWrapper(actor), size(s)	{}
							~MyNXSphere()														{}

		virtual		void	render()
							{
							float glmat[16];
							glPushMatrix();
							owner->getGlobalPose().getColumnMajor44(glmat);
							glMultMatrixf(glmat);
							glColor4f(0.8f, 1.0f, 0.8f, 1.0f);
							glutSolidSphere(size, 10, 10);
							glPopMatrix();

							// Handle shadows
							if(gShadows)
								{
								glPushMatrix();

								const  static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

								glMultMatrixf(ShadowMat);
								glMultMatrixf(glmat);

								glDisable(GL_LIGHTING);
								glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
								glutSolidSphere(size, 10, 10);
								glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
								glEnable(GL_LIGHTING);

								glPopMatrix();
								}
							}

		NxF32			size;
};

// Bunny wrapper
class MyNXBunny : public MyNXWrapper
{
		public:
							MyNXBunny(NxActor* actor) : MyNXWrapper(actor)					{}
							~MyNXBunny()													{}

		virtual		void	render()
							{
							float glmat[16];
							glPushMatrix();
							owner->getShapes()[0]->getGlobalPose().getColumnMajor44(glmat);
							glMultMatrixf(glmat);
							glColor4f(235.0f/255.0f, 159.0f/255.0f, 177.0f/255.0f, 1.0f);
							RenderBunny();
							glPopMatrix();

							// Handle shadows
							if(gShadows)
								{
								glPushMatrix();

								const  static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

								glMultMatrixf(ShadowMat);
								glMultMatrixf(glmat);

								glDisable(GL_LIGHTING);
								glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
								RenderBunny();
								glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
								glEnable(GL_LIGHTING);

								glPopMatrix();
								}
							}
};


void CreateCube(const NxVec3& pos, int size=2, const NxVec3* initial_velocity=NULL)
{
	// Create body
	NxBodyDesc BodyDesc;
	BodyDesc.angularDamping	= 0.5f;
	if(initial_velocity)	BodyDesc.linearVelocity = *initial_velocity;

	NxF32 coeff = NxF32(size)*0.1f;

	// Create box shape
	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions		= NxVec3(coeff, coeff, coeff);

	// Create actor
	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&BoxDesc);
	ActorDesc.body			= &BodyDesc;
	ActorDesc.density		= 1.0f;
	ActorDesc.globalPose.t  = pos;
	NxActor* newActor = gScene->createActor(ActorDesc);

	// Create app-controlled wrapper
	MyNXCube* newCube = new MyNXCube(newActor, coeff);

	// Bind our class to NX class
	newActor->userData = newCube;
}

void CreateSphere(const NxVec3& pos, int size=2, const NxVec3* initial_velocity=NULL)
{
	// Create body
	NxBodyDesc BodyDesc;
	BodyDesc.angularDamping	= 0.5f;
	if(initial_velocity)	BodyDesc.linearVelocity = *initial_velocity;

	NxF32 radius = NxF32(size)*0.1f;

	// Create sphere shape
	NxSphereShapeDesc SphereDesc;
	SphereDesc.radius		= radius;

	// Create actor
	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&SphereDesc);
	ActorDesc.body			= &BodyDesc;
	ActorDesc.density		= 1.0f;
	ActorDesc.globalPose.t  = pos;
	NxActor* newActor = gScene->createActor(ActorDesc);

	// Create app-controlled wrapper
	MyNXSphere* newSphere = new MyNXSphere(newActor, radius);

	// Bind our class to NX class
	newActor->userData = newSphere;
}

void InitBunny()
{
	// Scale the bunny a bit
//	for(NxU32 i=0;i<BUNNY_NBVERTICES*3;i++)	gBunnyVertices[i] *= 10.0f;

	// Build vertex normals (used only for rendering)
	gBunnyNormals = new NxVec3[BUNNY_NBVERTICES];
	NxBuildSmoothNormals(BUNNY_NBFACES, BUNNY_NBVERTICES, (const NxVec3*)gBunnyVertices, (const NxU32*)gBunnyTriangles, NULL, gBunnyNormals, true);

	// Initialize PMap
	NxPMap bunnyPMap;
	bunnyPMap.dataSize	= 0;
	bunnyPMap.data		= NULL;

	// Build physical model
	NxTriangleMeshDesc bunnyDesc;
	bunnyDesc.numVertices				= BUNNY_NBVERTICES;
	bunnyDesc.numTriangles				= BUNNY_NBFACES;
	bunnyDesc.pointStrideBytes			= sizeof(NxVec3);
	bunnyDesc.triangleStrideBytes		= 3*sizeof(NxU32);
	bunnyDesc.points					= gBunnyVertices;
	bunnyDesc.triangles					= gBunnyTriangles;							
	bunnyDesc.flags						= 0;

//	bunnyTriangleMesh = gPhysicsSDK->createTriangleMesh(bunnyDesc);

	bool status = InitCooking();
	if (!status) {
		printf("Unable to initialize the cooking library. Please make sure that you have correctly installed the latest version of the AGEIA PhysX SDK.");
		exit(1);
	}
	MemoryWriteBuffer buf;
	status = CookTriangleMesh(bunnyDesc, buf);
	if (!status) {
		printf("Unable to cook a triangle mesh.");
		exit(1);
	}
	MemoryReadBuffer readBuffer(buf.data);
	bunnyTriangleMesh = gPhysicsSDK->createTriangleMesh(readBuffer);

/*
	bool status = CookTriangleMesh(bunnyDesc, UserStream("c:\\tmp.bin", false));
	bunnyTriangleMesh = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
*/


	// PMap stuff
	// Try loading PMAP from disk
	char PMapFilename[512];
	FindMediaFile("bunny.pmap", PMapFilename);
	FILE* fp = fopen(PMapFilename, "rb");
	if(!fp)
	{
		// Not found => create PMap
		printf("please wait while precomputing pmap...\n");
		if(CreatePMap(bunnyPMap, *bunnyTriangleMesh, 64))
		{
			// The pmap has successfully been created, save it to disk for later use
			char PMapFilename[512];
			GetTempFilePath(PMapFilename);
			strcat(PMapFilename, "bunny.pmap");
			fp = fopen(PMapFilename, "wb+");
			if(fp)
			{
				fwrite(bunnyPMap.data, bunnyPMap.dataSize, 1, fp);
				fclose(fp);
				fp = NULL;
			}

			//assign pmap to mesh
			bunnyTriangleMesh->loadPMap(bunnyPMap);

			// sdk created data => sdk deletes it
			ReleasePMap(bunnyPMap);
			}
		printf("...done\n");
	}
	else
	{
		// Found pmap file
		bunnyPMap.dataSize	= getFileSize("bunny.pmap");
		bunnyPMap.data		= new NxU8[bunnyPMap.dataSize];
		fread(bunnyPMap.data, bunnyPMap.dataSize, 1, fp);
		fclose(fp);

		//assign pmap to mesh
		bunnyTriangleMesh->loadPMap(bunnyPMap);

		//we created data => we delete it
		delete bunnyPMap.data;
	}

	CloseCooking();
}

void CreateBunny(const NxVec3 & pos)
{
	//create triangle mesh instance
	NxTriangleMeshShapeDesc bunnyShapeDesc;
	bunnyShapeDesc.meshData	= bunnyTriangleMesh;
	bunnyShapeDesc.localPose.t.set(0,0.65f,0);

	// Create body
	NxBodyDesc BodyDesc;
	BodyDesc.angularDamping	= 0.5f;
	BodyDesc.maxAngularVelocity	= 10.0f;

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&bunnyShapeDesc);
	ActorDesc.body			= &BodyDesc;
	ActorDesc.density		= 1.0f;
	ActorDesc.globalPose.t  =  pos;
	NxActor* newActor = gScene->createActor(ActorDesc);
	
	// Create app-controlled wrapper
	MyNXBunny* newBunny = new MyNXBunny(newActor);

	// Bind our class to NX class
	newActor->userData = newBunny;
}

 void InitNx()
{
	// Initialize PhysicsSDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, &gErrorStream);
	if(!gPhysicsSDK)	return;

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.005f);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity				= gDefaultGravity;

	gScene = gPhysicsSDK->createScene(sceneDesc);


	NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);

	// Create ground plane
	NxPlaneShapeDesc PlaneDesc;
	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&PlaneDesc);
	gScene->createActor(ActorDesc);

	// Initialize bunny data
	InitBunny();
}

 void KeyboardCallback(unsigned char key, int x, int y)
{

switch (key)
	{
	case 27:
		exit(0);
		break;
	case ' ':
		tick(0, true);
		break;

	case 201: case '8':	case 'w': Eye += Dir * 2.0f; break;
	case 203: case '2':	case 's': Eye -= Dir * 2.0f; break;
	case 200: case '4':	case 'a': Eye -= N * 2.0f; break;
	case 202: case '6':	case 'd': Eye += N * 2.0f; break;
	}
}

 void ArrowKeyCallback(int key, int x, int y)
	{
	KeyboardCallback(key+100,x,y);
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
	
	Dir.normalize();
	N.cross(Dir,NxVec3(0,1,0));

	NxQuat qx(NxPiF32 * dx * 20/ 180.0f, NxVec3(0,1,0));
	qx.rotate(Dir);
	NxQuat qy(NxPiF32 * dy * 20/ 180.0f, N);
	qy.rotate(Dir);

	mx = x;
	my = y;
}

void DisplayText()
{
	float y = 0.95f;
	int len = strlen(displayString);
	len = (len < 512)?len:511;
	int start = 0;
	char textBuffer[512];
	for(int i=0;i<len;i++)
	{
		if(displayString[i] == '\n' || i == len-1)
		{
			int offset = i;
			if(i == len-1) offset= i+1;
			memcpy(textBuffer, displayString+start, offset-start);
			textBuffer[offset-start]=0;
			GLFontRenderer::print(0.01, y, 0.03f, textBuffer);
			y -= 0.035f;
			start = offset+1;
		}
	}
}

void RenderCallback()
{
	static unsigned int PreviousTime = timeGetTime();
	unsigned int CurrentTime = timeGetTime();
	unsigned int ElapsedTime = CurrentTime - PreviousTime;
	PreviousTime = CurrentTime;
	NxF32 elapsedTime = NxF32(ElapsedTime)*0.001f;

	// Physics code
	if(gScene && !gPause)	
		{
		tick(elapsedTime);
		gScene->simulate(elapsedTime);
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		}
	// ~Physics code

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, ((float)glutGet(GLUT_WINDOW_WIDTH))/((float)glutGet(GLUT_WINDOW_HEIGHT)), 1.0f, 10000.0f);
	gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Keep physics & graphics in sync
	int nbActors = gScene->getNbActors();
	NxActor** actors = gScene->getActors();
	while(nbActors--)
	{
		NxActor* actor = *actors++;

		MyNXWrapper* wrapper = (MyNXWrapper*)actor->userData;
		if(!wrapper)	continue;	// Happens for ground plane

		wrapper->render();
	}

	glClear(GL_DEPTH_BUFFER_BIT);	//clear z for visualization.

	gDebugRenderer.renderData(*gScene->getDebugRenderable());

	DisplayText();


	glFlush();
	glutSwapBuffers();
}

 void ReshapeCallback(int width, int height)
{
	glViewport(0, 0, width, height);
}

 void IdleCallback()
{
	glutPostRedisplay();
}

 void ExitCallback()
{
	if (gPhysicsSDK)
	{
		if (gScene) gPhysicsSDK->releaseScene(*gScene);
		gPhysicsSDK->release();
	}
}

int main(int argc, char** argv)
{
	printf("Use the mouse to rotate the camera.\n");
	printf("Use the arrow keys or 2, 4, 6 and 8 to move the camera.\n");
	printf("Press spacebar to advance the tutorial.\n");

	// Initialize Glut
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("SampleTransforms");
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
	float AmbientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };		glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	float DiffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	float SpecularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[]		= { 100.0f, 100.0f, 400.0f, 1.0f };		glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	// Physics code
	InitNx();
	// ~Physics code

	// Run
	glutMainLoop();
}
