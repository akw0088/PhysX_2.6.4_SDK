// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Threading Sample
// Description: SampleThreading, a sample demonstrating the use of the threading
//              interface available in the PhysX SDK 2.4+
//
// This sample demonstrates 4 different approaches to threading:
//
// * Application supplied scheduler
// * Polling, ie the SDK manages the work queue and the application can poll a
//   function to perform work
// * SDK managed threading (At present this is likely to yield the best performance)
// * No threading
//
// See below for defines to control which method this sample uses.
//
// NOTE: Fine grained threading is only available (at present) on the XBox 360.
// 	  While these interfaces are available on other platforms, they will not
// 	  provide any performance gains.
//
// NOTE: The graphics wrapper used on the Xbox 260 is quite slow and performance
//		  comparisons should not be made with graphics enabled.
//
// NOTE: This sample expects to be able to find a serialized scene named pdump.ssb,
// 	  which is a scene with lots of objects for the purpose of testing the threading.
//
//
// ===============================================================================

#include <stdio.h>
#include <GL/glut.h>
#include <deque>
#include <vector>


#include "NXU_Helper.h"

#define NOMINMAX

#ifdef _XBOX
	// Xbox 360 XDK 3215 (June 06) requires that this version of min and max
	// be defined in order for xmconvert.inl to build
	inline unsigned int max(unsigned int a, unsigned int b) { (((a) > (b)) ? (a) : (b)); }
	inline unsigned int min(unsigned int a, unsigned int b) { (((a) < (b)) ? (a) : (b)); }
#	include <xtl.h>
#else
#	include <windows.h>
#endif

#include "NxPhysics.h"
#include "ErrorStream.h"
#include "PerfRenderer.h"
#include "DebugRenderer.h"
#include "DrawObjects.h"
#include "Timing.h"
#include "MediaPath.h"
#include "cooking.h"

// include exporter here
#include "NXU_Helper.h"

#include "CustomScheduler.h"
#include "PollingThreads.h"

//#define TRACERECORDING
#if defined(_XBOX) && defined(TRACERECORDING)
#include <tracerecording.h>
#pragma comment(lib, "tracerecording.lib")
#pragma comment(lib, "xbdm.lib")
#endif

//#define THREAD_CUSTOM_SCHEDULER
//#define THREAD_POLLING
#define THREAD_SDK
//else no threading

// Physics
static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static PerfRenderer    gPerfRenderer;
static DebugRenderer	gDebugRenderer;

// Rendering
static NxVec3	gEye(26.0f, 21.0f, -23.0f);
static NxVec3	gDir(-0.8f,-0.32f,0.49f);
static NxVec3	gViewY;
static int		gMouseX = 0;
static int		gMouseY = 0;
static bool		gRendering = true;

static CustomScheduler gCustomScheduler;
static PollingThreads gPollingThreads;

static void DeleteTriangleMeshDesc(NxTriangleMeshDesc *desc);
static NxTriangleMeshDesc *CloneTriangleMeshDesc(const NxTriangleMeshDesc &desc);

static void InitPerfTiming();
static void StartPerfTiming();
static void EndPerfTiming();


class ThreadedUserScene: public NXU_userNotify, public NXU_errorReport
{
public:

	virtual	void NXU_errorMessage(bool isError, const char	*str)
	{
		if ( isError )
			printf("NXU: (ERROR) %s\r\n", str);
		else
			printf("NXU: (WARNING) %s\r\n", str );
	}

  virtual NxScene *NXU_preNotifyScene(unsigned	int	sno, NxSceneDesc &mySceneDesc, const char	*userProperties)
	{
		mySceneDesc.flags					&= ~NX_SF_SIMULATE_SEPARATE_THREAD;

#if defined(THREAD_CUSTOM_SCHEDULER)

		mySceneDesc.flags					|= NX_SF_ENABLE_MULTITHREAD;
		mySceneDesc.customScheduler=&gCustomScheduler;

#elif defined(THREAD_POLLING)

		mySceneDesc.flags					|= NX_SF_ENABLE_MULTITHREAD;

#elif defined(THREAD_SDK)

		mySceneDesc.flags					|= NX_SF_ENABLE_MULTITHREAD;
		mySceneDesc.internalThreadCount   = 2;

#else
		  //No threading
#endif


    return 0;
	}

	virtual void NXU_notifyScene(unsigned int sno, NxScene *scene, const char *userProperties)
	{
		if(gScene != NULL)
			assert(!"Only one scene can be loaded");
		gScene = scene;
		gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
	}

	virtual bool NXU_preNotifyActor(NxActorDesc &actor, const char *userProperties)
	{
		for (NxU32 i=0; i<actor.shapes.size(); i++)
		{
			NxShapeDesc *s = actor.shapes[i];
			s->shapeFlags|=NX_SF_VISUALIZATION; // make sure the shape visualization flags are on so we can see things!
		}
		return true;
	};



  virtual	void NXU_notifyActor(NxActor *actor,const char *userProperties)
	{
		NxU32 nbShapes=actor->getNbShapes();

		NxShape *const*shapes=actor->getShapes();
		for(NxU32 i=0;i<nbShapes;i++)
		{


			NxTriangleMeshShape *triShape=shapes[i]->isTriangleMesh();

			if(triShape!=NULL)
			{
				NxTriangleMesh &triMesh=triShape->getTriangleMesh();

				NxTriangleMeshDesc desc;
				triMesh.saveToDesc(desc);

				triShape->userData=CloneTriangleMeshDesc(desc);
			}
		}
	}


};

static ThreadedUserScene gUserCreate;

static void InitNx()
{
	// Initialize PhysicsSDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, new ErrorStream());
	if(gPhysicsSDK == NULL) return;

	// This sample needs the cooking library inside of the importer, if it can't be found, then exit
	bool status = InitCooking();
	if (!status) {
		printf("Unable to initialize the cooking library. Please make sure that you have correctly installed the latest version of the AGEIA PhysX SDK.");
		exit(1);
	}

	char buff[_MAX_PATH];

	FindMediaFile("SampleThreading.xml", buff);

		NXU::setErrorReport(&gUserCreate);

  NXU::NxuPhysicsCollection *c = NXU::loadCollection( buff, NXU::FT_XML );
  if ( c )
  {
  	NXU::instantiateCollection(c, *gPhysicsSDK, 0, 0, &gUserCreate );
  	NXU::releaseCollection(c);
	}
	else
	{
		assert(!"Cannot find input file");
	}

	// kick off the threading
#if defined(THREAD_CUSTOM_SCHEDULER)

	gCustomScheduler.CreateThreads(2);

#elif defined(THREAD_POLLING)

	gPollingThreads.CreateThreads(2,gScene);

#elif defined(THREAD_SDK)
	//Nothing to do, SDK manages threads
#endif

}

static void ExitNx()
{
#if defined(THREAD_CUSTOM_SCHEDULER)

	gCustomScheduler.KillThreads();

#elif defined(THREAD_POLLING)

	gPollingThreads.KillThreads();

#elif defined(THREAD_SDK)
	//Nothing to do, SDK manages threads
#endif

	if(gPhysicsSDK != NULL)
	{
		if(gScene != NULL) 
			{
			//we must clean up the descriptors we copied into the user data field.
			NxU32 nbActors=gScene->getNbActors();
			NxActor **actors=gScene->getActors();

			for(NxU32 i=0;i<nbActors;i++)
				{

				NxU32 nbShapes=actors[i]->getNbShapes();
				NxShape *const* shapes=actors[i]->getShapes();

				for(NxU32 j=0;j<nbShapes;j++)
					{
					NxTriangleMeshShape *triShape=shapes[j]->isTriangleMesh();
					if(triShape!=NULL)
						DeleteTriangleMeshDesc((NxTriangleMeshDesc *)triShape->userData);
					}

				}

			gPhysicsSDK->releaseScene(*gScene);
			}

    NXU::releasePersistentMemory();
		gPhysicsSDK->release();
	}

	exit(0);
}

static void CreateCube(const NxVec3& pos, float size, const NxVec3* initialVelocity=NULL)
{
	if(gScene == NULL) return;	

	// Create body
	NxBodyDesc bodyDesc;
	//bodyDesc.angularDamping	= 0.5f;
	if(initialVelocity) bodyDesc.linearVelocity = *initialVelocity;

	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = NxVec3(size, size, size);

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body			= &bodyDesc;
	actorDesc.density		= 0.25f;
	actorDesc.globalPose.t  = pos;
	gScene->createActor(actorDesc);
}

static void CreateCubeFromEye(float size)
{
	NxVec3 t = gEye;
	NxVec3 vel = gDir;
	vel.normalize();
	vel*=25.0f;
	CreateCube(t, size, &vel);
}

static void KeyboardCallback(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27:	ExitNx(); break;
		case '0':	gPerfRenderer.toggleEnable(); break;
		case 'w':	CreateCubeFromEye(0.2f); break;
		case 't':	gRendering=!gRendering; break;
	
		case GLUT_KEY_UP:	case '8':	gEye += gDir*2.0f; break;
		case GLUT_KEY_DOWN: case '2':	gEye -= gDir*2.0f; break;
		case GLUT_KEY_LEFT:	case '4':	gEye -= gViewY*2.0f; break;
		case GLUT_KEY_RIGHT: case '6':	gEye += gViewY*2.0f; break;
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

#if defined(THREAD_POLLING)

	//We must reset the polling threads so they are ready for the next run
	gPollingThreads.ResetPollForWork();
#endif


	StartPerfTiming();

	// Start simulation
	gScene->simulate(1.0f/60.0f);
	
	// Fetch simulation results
	gScene->flushStream();
	gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);

	EndPerfTiming();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT), 1.0f, 10000.0f);

	// Setup modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(gEye.x, gEye.y, gEye.z, gEye.x + gDir.x, gEye.y + gDir.y, gEye.z + gDir.z, 0.0f, 1.0f, 0.0f);

	// Setup lighting

	if(gRendering)
	{
		// Render all actors
		int nbActors = gScene->getNbActors();
		NxActor** actors = gScene->getActors();
		while(nbActors--)
		{
			NxActor* actor = *actors++;

			// Render actor
			DrawActor(actor);
		}

		int nbCloths = gScene->getNbCloths();
		NxCloth **cloths = gScene->getCloths();
		while(nbCloths--)
		{
			NxCloth *cloth = *cloths++;

			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawCloth(cloth, false);
		}
		glColor4f(1.0f,1.0f,1.0f,1.0f);
	}



	if(gRendering)
	{
		//render visualization
		gDebugRenderer.renderData(*gScene->getDebugRenderable());
	}

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
	printf("Press w to throw a box into the scene\n");
	printf("      t to toggle rendering\n");
	printf("      0 to toggle performance information\n");

	// Initialize glut
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	int mainHandle = glutCreateWindow("SampleThreading");
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

	InitPerfTiming();

	// Init Physics
	InitNx();

	// Run
	glutMainLoop();

	return 0;
}


static void DeleteTriangleMeshDesc(NxTriangleMeshDesc *desc)
	{
	if(desc==NULL)
		return;

	if(desc->points!=NULL)
		delete[] desc->points;
	if(desc->triangles!=NULL)
		delete[] desc->triangles;
	delete desc;
	}

// NOTE: we dont copy materialIndices or pmaps.
static NxTriangleMeshDesc *CloneTriangleMeshDesc(const NxTriangleMeshDesc &desc)
	{
	NxTriangleMeshDesc *newDesc=new NxTriangleMeshDesc();

	*newDesc=desc;
	newDesc->materialIndices=NULL;
	newDesc->pmap=NULL;
				
	NxReal *newPoints=new NxReal[3*desc.numVertices];
	NxU8 *oldPointPtr=(NxU8 *)desc.points;
								
	for(NxU32 j=0;j<desc.numVertices;j++)
		{
		NxReal *srcPtr=(NxReal *)(oldPointPtr+desc.pointStrideBytes*j);

		newPoints[j*3]=srcPtr[0];
		newPoints[j*3+1]=srcPtr[1];
		newPoints[j*3+2]=srcPtr[2];
		}

	newDesc->pointStrideBytes=sizeof(NxReal)*3;
	newDesc->points=newPoints;

	if((desc.triangles!=NULL)&&(desc.numTriangles>0))
		{
		if(desc.flags & NX_MF_16_BIT_INDICES)
			{
			NxU16 *newIndices=new NxU16[desc.numTriangles*3];

			NxU8 *oldIdxPtr=(NxU8 *)desc.triangles;

			for(NxU32 j=0;j<desc.numTriangles;j++)
				{
				NxU16 *srcTri=(NxU16 *)(oldIdxPtr+desc.triangleStrideBytes*j);

				newIndices[j*3]=srcTri[0];
				newIndices[j*3+1]=srcTri[1];
				newIndices[j*3+2]=srcTri[2];
				}

			newDesc->triangleStrideBytes=sizeof(NxU16)*3;
			newDesc->triangles=newIndices;			
			}
		else
			{
			NxU32 *newIndices=new NxU32[desc.numTriangles*3];

			NxU8 *oldIdxPtr=(NxU8 *)desc.triangles;

			for(NxU32 j=0;j<desc.numTriangles;j++)
				{
				NxU32 *srcTri=(NxU32 *)(oldIdxPtr+desc.triangleStrideBytes*j);

				newIndices[j*3]=srcTri[0];
				newIndices[j*3+1]=srcTri[1];
				newIndices[j*3+2]=srcTri[2];
				}

			newDesc->triangleStrideBytes=sizeof(NxU32)*3;
			newDesc->triangles=newIndices;
			}
		}
	
	return newDesc;
	}

// timing
/////////////////////////////////////////////////////////////////////////////////////

LARGE_INTEGER before, after, freq;
static float sumA = 0;
static unsigned int numA = 0;
static bool doTraceRecording = false;

static void InitPerfTiming()
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&before);
}

static void StartPerfTiming()
{
	QueryPerformanceCounter(&before);

#if defined(_XBOX) && defined(TRACERECORDING)
	
	if(doTraceRecording)
	{
		XTraceStartRecording("e:\\trace.bin");
	}

#endif

}

static void EndPerfTiming()
{
#if defined(_XBOX) && defined(TRACERECORDING)
	if(doTraceRecording)
	{
		XTraceStopRecording();
		doTraceRecording = false;
	}
#endif

	QueryPerformanceCounter(&after);

	double a = (double)(after.QuadPart - before.QuadPart);
	static int frameCount =0;
	a = a / (double)freq.QuadPart;
	sumA += ((float)a);
	numA++;
	//printf("\n== frame %d (%f ms) (%f ms)==\n\n", frameCount++, (float)a * 1000, (sumA*1000) / numA);

}
