// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: HeightField Sample
// Description: This sample program shows how to use the special HeightField 
//              geometry in a software scene.
//
// Originally written by: Peter Tchernev (2005.10.01)
// Additions by: 
//
// ===============================================================================

#include <stdio.h>
#include <GL/glut.h>

// Physics code
#undef random
#include "NxPhysics.h"
#include "UserAllocator.h"
#include "ErrorStream.h"
#include "NxHeightField.h"
#include "NxHeightFieldDesc.h"
#include "NxHeightFieldShapeDesc.h"
#include "NxHeightFieldSample.h"
#include "NxCooking.h"
#include "Stream.h"

#include "NxTriangle.h"

#include "DrawObjects.h"
#include "DebugRenderer.h"

static const int gMatrixSize = 3;
static const NxMaterialIndex gMatrix[9][3] =
{
// {tesselation, material0, material1}
{0,1,1}, {0,1,1}, {0,1,1}, 
{0,1,1}, {0,1,1}, {0,1,1}, 
{0,1,1}, {0,1,1}, {0,1,1} 
};

static bool				gProfile = true;
static bool				gPause = false;
static ErrorStream		gErrorStream;
static DebugRenderer	gDebugRenderer;
static UserAllocator*	gAllocator = NULL;
static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static NxVec3			gGravity = NxVec3(0.0f, -9.81f, 0.0f);
static NxActor*			gHeightField = NULL;
static NxArray<NxU32>	gTouchedTris;
static NxVec3			gSpawnPos = NxVec3(10, 3, 10);
#define ROT_SCALE	0.1f
#define ZOOM_SCALE	0.01f
#define PAN_SCALE	0.01f

// Render code
static int gMouseX = 0;
static int gMouseY = 0;
static int gMouseButtons = 0;
static int gMouseState = 0;
static float gRotX = -50;
static float gRotY = 35;
static NxVec3 gCameraLookAt = NxVec3(15, 0, 15);
static NxVec3 gCameraOrigin = NxVec3(30, 15, 30);
static NxVec3 gCameraX = NxVec3(1, 0, 0);
static NxVec3 gCameraY = NxVec3(0, 1, 0);
static NxVec3 gCameraZ = NxVec3(0, 0, 1);
static float gCameraDistance = 30;

class MyContactReport : public NxUserContactReport
	{
	public:
		virtual void onContactNotify(NxContactPair& pair, NxU32 events)
			{
			// Iterate through contact points
			NxContactStreamIterator i(pair.stream);
			//user can call getNumPairs() here
			while(i.goNextPair())
				{
				//user can also call getShape() and getNumPatches() here
				while(i.goNextPatch())
					{
					//user can also call getPatchNormal() and getNumPoints() here
					const NxVec3& contactNormal = i.getPatchNormal();
					while(i.goNextPoint())
						{
						//user can also call getPoint() and getSeparation() here
						const NxVec3& contactPoint = i.getPoint();

						NxU32 faceIndex = i.getFeatureIndex0();
						if(faceIndex==0xffffffff)	
							faceIndex = i.getFeatureIndex1();
						if(faceIndex!=0xffffffff)
							{
							gTouchedTris.pushBack(faceIndex);
							}
						}
					}
				}
			}

	} gMyContactReport;

static void CreateHeightField(const NxVec3& pos, NxVec3 size = NxVec3(10,1,10), NxU32 nbColumns=10, NxU32 nbRows=10)
	{
	static NxReal sixtyFourKb = 65536.0f;
	static NxReal thirtyTwoKb = 32767.5f;

	NxHeightFieldDesc heightFieldDesc;
	heightFieldDesc.nbColumns		= nbColumns;
	heightFieldDesc.nbRows			= nbRows;
	heightFieldDesc.verticalExtent	= -1000;
	heightFieldDesc.convexEdgeThreshold = 0;

	// allocate storage for samples
	heightFieldDesc.samples			= new NxU32[nbColumns*nbRows];
	heightFieldDesc.sampleStride	= sizeof(NxU32);

	char* currentByte = (char*)heightFieldDesc.samples;
	for (NxU32 row = 0; row < nbRows; row++)
		{
		for (NxU32 column = 0; column < nbColumns; column++)
			{
			NxReal s = NxReal(row) / NxReal(nbRows);
			NxReal t = NxReal(column) / NxReal(nbColumns);
			NxI16 height = (NxI32)(0.5 * thirtyTwoKb * (NxMath::sin(5.0f*NxPiF32*s) + NxMath::cos(5.0f*NxPiF32*t)));
			//NxI16 height = - (nbRows / 2 - row) * (nbRows / 2 - row) - (nbColumns / 2 - column) * (nbColumns / 2 - column);
			int matrixOffset = (row % gMatrixSize) * gMatrixSize + (column % gMatrixSize);
			
			NxHeightFieldSample* currentSample = (NxHeightFieldSample*)currentByte;
			currentSample->height = height;
			currentSample->materialIndex0 = gMatrix[matrixOffset][1];
			currentSample->materialIndex1 = gMatrix[matrixOffset][2];
			currentSample->tessFlag = gMatrix[matrixOffset][0];

			currentByte += heightFieldDesc.sampleStride;
			}
		}

	NxHeightField* heightField = gScene->getPhysicsSDK().createHeightField(heightFieldDesc);

	// data has been copied, we can free our buffer
	delete [] heightFieldDesc.samples;

	NxHeightFieldShapeDesc heightFieldShapeDesc;
	heightFieldShapeDesc.heightField	= heightField;
	heightFieldShapeDesc.shapeFlags		= NX_SF_FEATURE_INDICES | NX_SF_VISUALIZATION;
	heightFieldShapeDesc.heightScale	= size.y / sixtyFourKb;
	heightFieldShapeDesc.rowScale		= size.x / NxReal(nbRows-1);
	heightFieldShapeDesc.columnScale	= size.z / NxReal(nbColumns-1);
	//heightFieldShapeDesc.meshFlags	= NX_MESH_SMOOTH_SPHERE_COLLISIONS;
	heightFieldShapeDesc.materialIndexHighBits = 0;
	heightFieldShapeDesc.holeMaterial = 2;

	NxBodyDesc bodyDesc;
	bodyDesc.angularDamping	= 0.5f;
	bodyDesc.flags |= NX_BF_KINEMATIC;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&heightFieldShapeDesc);
	actorDesc.body		   = &bodyDesc;
	actorDesc.density	   = 1.0f;
	actorDesc.globalPose.t = pos;
	NxActor* newActor = gScene->createActor(actorDesc);

	gHeightField = newActor;
	}

static NxActor* CreateActorFromDesc(const NxShapeDesc& shapeDesc, const NxMat34& globalPose, bool body = true)
	{
	if (!gScene) return NULL;
	NxBodyDesc bodyDesc;
	NxActorDesc actorDesc;
	actorDesc.globalPose.t = globalPose.t;
	actorDesc.globalPose.M = globalPose.M;

	NxSphereShapeDesc sphereDesc;
	sphereDesc.localPose.t.x = 1;
	sphereDesc.radius = 1;
	sphereDesc.shapeFlags = NX_SF_VISUALIZATION | NX_TRIGGER_ENABLE;

	actorDesc.shapes.pushBack(&(NxShapeDesc&)shapeDesc);
	//actorDesc.shapes.pushBack(&(NxShapeDesc&)sphereDesc);
	if (body)
		{
		bodyDesc.angularDamping	= 0.5f;
		actorDesc.body			= &bodyDesc;
		actorDesc.density		= 10.0f;
		}
	return gScene->createActor(actorDesc);
	}

static NxActor* CreateCube(const NxVec3& pos, NxReal size = 2.0f, bool trigger = false)
	{
	NxBoxShapeDesc shapeDesc;
	shapeDesc.dimensions = NxVec3(float(size), float(size), float(size));
	shapeDesc.shapeFlags |= NX_SF_VISUALIZATION;
	if (trigger) shapeDesc.shapeFlags |= NX_TRIGGER_ENABLE;
	return CreateActorFromDesc(shapeDesc, NxMat34(NxMat33(NX_IDENTITY_MATRIX), pos), !trigger);
	}

static NxActor* CreateBall(const NxVec3& pos, NxReal size = 2.0f, bool trigger = false)
	{
	NxSphereShapeDesc shapeDesc;
	shapeDesc.radius = float(size);
	shapeDesc.shapeFlags |= NX_SF_VISUALIZATION;
	if (trigger) shapeDesc.shapeFlags |= NX_TRIGGER_ENABLE;
	return CreateActorFromDesc(shapeDesc, NxMat34(NxMat33(NX_IDENTITY_MATRIX), pos), !trigger);
	}

static NxActor* CreateCapsule(const NxVec3& pos, NxReal size = 2.0f, bool trigger = false)
	{
	NxCapsuleShapeDesc shapeDesc;
	shapeDesc.radius = size;
	shapeDesc.height = 2*size;
	shapeDesc.shapeFlags |= NX_SF_VISUALIZATION;
	if (trigger) shapeDesc.shapeFlags |= NX_TRIGGER_ENABLE;
	return CreateActorFromDesc(shapeDesc, NxMat34(NxMat33(NX_IDENTITY_MATRIX), pos), !trigger);
	}

static void GenerateCirclePts(NxU32 nb, NxVec3* pts, NxF32 scale, NxF32 z)
	{
	for(NxU32 i=0;i<nb;i++)
		{
		NxF32 angle = 2*NxPiF32*NxF32(i)/NxF32(nb);
		pts[i].x = NxMath::cos(angle)*scale;
		pts[i].y = z;
		pts[i].z = NxMath::sin(angle)*scale;
		}
	}

static NxActor* CreateConvex(const NxVec3& pos, int size=2, const NxVec3* initial_velocity=NULL, bool trigger = false)
	{

	NxU32 nbInsideCirclePts = (NxU32)NxMath::rand(3, 8);
	NxU32 nbOutsideCirclePts = (NxU32)NxMath::rand(3, 8);
	NxU32 nbVerts = nbInsideCirclePts + nbOutsideCirclePts;
	// Generate random vertices
	NxVec3* verts = (NxVec3*)NxAlloca(sizeof(NxVec3)*nbVerts);
	//			if (NxMath::rand(0, 100) > 50)	//two methods
	if (0)	//two methods
		{
		for(NxU32 i=0;i<nbVerts;i++)
			{
			verts[i].x = size * (NxReal)NxMath::rand(-1, 1);
			verts[i].y = size * (NxReal)NxMath::rand(-1, 1);
			verts[i].z = size * (NxReal)NxMath::rand(-1, 1);
			}
		}
	else
		{
		GenerateCirclePts(nbInsideCirclePts, verts, 0.5f*size, 0.0f);
		GenerateCirclePts(nbOutsideCirclePts, verts+nbInsideCirclePts, 1.0f*size, 2.0f*size);
		}

	// Create descriptor for convex mesh
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices			= nbVerts;
	convexDesc.pointStrideBytes		= sizeof(NxVec3);
	convexDesc.points				= verts;
	convexDesc.flags				= NX_CF_COMPUTE_CONVEX;

	NxConvexShapeDesc convexShapeDesc;

	// Cooking from memory
	MemoryWriteBuffer buf;
	bool status = NxCookConvexMesh(convexDesc, buf);
	//convexShapeDesc.scale = 10;
	convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));

	convexShapeDesc.shapeFlags |= NX_SF_VISUALIZATION;
	if (trigger) convexShapeDesc.shapeFlags |= NX_TRIGGER_ENABLE;

	if(convexShapeDesc.meshData)
		return CreateActorFromDesc(convexShapeDesc, NxMat34(NxMat33(NX_IDENTITY_MATRIX), pos), !trigger);

	return NULL;
	}

static void CreateGrid(NxVec3 pos, int size=1, int shapeType=0, int shapeSize=1)
	{
	float spacing = 1.1f * shapeSize;
	for (int z = 0; z<size; z++) 
		{
		for (int x = 0; x<size; x++) 
			{
			NxReal delta = +(shapeSize + spacing);
			NxActor* newActor = NULL;
			switch(shapeType)
				{
				case 0:
					newActor = CreateBall(pos + NxVec3((float)x*delta, 0, (float)z*delta), (NxReal)shapeSize);
					break;
				case 1:
					newActor = CreateCapsule(pos + NxVec3((float)x*delta, 0, (float)z*delta), (NxReal)shapeSize);
					break;
				case 2:
					newActor = CreateCube(pos + NxVec3((float)x*delta, 0, (float)z*delta), (NxReal)shapeSize);
					break;
				case 3:
					newActor = CreateConvex(pos + NxVec3((float)x*delta, 0, (float)z*delta), shapeSize);
					break;
				default:
					; //noop
					break;
				}

			if (gHeightField && newActor) 
				gScene->setActorPairFlags(*gHeightField, *newActor, NX_NOTIFY_ON_TOUCH);
			}
		}
	}

static void CreateScene(NxVec3 gravity = NxVec3(0, -9.81, 0), NxReal staticFriction = 0.8f, NxReal dynamicFriction = 0.6f, NxReal restitution = 0.1f)
	{
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = gravity;
	sceneDesc.userContactReport = &gMyContactReport;
	gScene = gPhysicsSDK->createScene(sceneDesc);
	if (gScene)
		{
		NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
		defaultMaterial->setRestitution(restitution);
		defaultMaterial->setStaticFriction(staticFriction);
		defaultMaterial->setDynamicFriction(dynamicFriction);
		}
	}

static void CreatePhysicsSDK() 
	{
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator, &gErrorStream);
	if(gPhysicsSDK) 
		{
		gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.025f);
		gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AXES, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AABBS, 1);
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SPHERES, 1);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_EDGES, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_VNORMALS, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_NORMAL, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_POINT, 1.0f);
		//gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_MASS_AXES, 1.0f);
		}
	}

static void ResetScene()
	{
	if (gPhysicsSDK && gScene) gPhysicsSDK->releaseScene(*gScene);
#ifdef RESET_SDK
	if (gPhysicsSDK) gPhysicsSDK->release();
#endif
	if (!gPhysicsSDK) CreatePhysicsSDK();
	if (gPhysicsSDK)
		{
		CreateScene(gGravity, 0, 0, 0);
		CreateHeightField(NxVec3(0,-1,0), NxVec3(30,1,30), 40, 40);
		}
	}

static void InitNx()
	{
	gAllocator = new UserAllocator;
	NxInitCooking();
	ResetScene();

#if defined(_XBOX) | defined(__CELLOS_LV2__)
	glutRemapButtonExt(0, '1', false); // A/X to spawn spheres
	glutRemapButtonExt(1, '2', false);	// B/O to spawn capsules.
	glutRemapButtonExt(2, '3', false);	// X/S to spawn boxes.
	glutRemapButtonExt(3, '4', false);	// Y/T to spawn convexes.
	glutRemapButtonExt(9, 'R', false); // Right shoulder to reset scene
#endif
	}

static void ExitCallback()
	{
	if (gPhysicsSDK)
		{
		if (gScene) gPhysicsSDK->releaseScene(*gScene);
		gPhysicsSDK->release();
		}
	if(gAllocator) delete gAllocator;
	}

static void KeyboardCallback(unsigned char key, int x, int y)
	{
	switch (key)
		{
		case 27: exit(0); break;
		case '1': CreateGrid(gSpawnPos, 5, 0, 1); break;
		case '2': CreateGrid(gSpawnPos, 5, 1, 1); break;
		case '3': CreateGrid(gSpawnPos, 5, 2, 1); break;
		case '4': CreateGrid(gSpawnPos, 5, 3, 1); break;
		case 'R':
		case 'r': ResetScene(); break;
		case 'P':
		case 'p': gPause = !gPause; break;
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
	gMouseButtons = button;
	gMouseState = state;
	}

static void MotionCallback(int x, int y)
	{
	int dx = gMouseX - x;
	int dy = gMouseY - y;

	gMouseX = x;
	gMouseY = y;

	if (gMouseState == GLUT_DOWN)
		{
		switch(gMouseButtons)
			{
			case GLUT_LEFT_BUTTON:
				{
				gRotX += (ROT_SCALE) * (float) dx;
				gRotY += (ROT_SCALE) * (float) dy;
				}
			break;
			case GLUT_RIGHT_BUTTON:
				{
				gCameraDistance += (1 + NxMath::abs(gCameraDistance)) * (ZOOM_SCALE) * (float)(dx + dy);
				if (gCameraDistance < 1) gCameraDistance = 1;
				}
			break;
			case GLUT_MIDDLE_BUTTON:
				{
				NxReal scale = ZOOM_SCALE * NxMath::abs(gCameraDistance);
				NxVec3 pan = gCameraX*scale*(NxReal)dx - gCameraY*scale*(NxReal)dy;
				gCameraLookAt += pan;
				}
			break;
			}
		}	
	}


static void RenderCallback()
	{
	if(gScene == NULL) return;

	// Physics code
	if(!gPause)	
		{
		gTouchedTris.clear();
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
	gluPerspective(80.0f, (float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT), 1.0f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -gCameraDistance);
	glRotatef(gRotY, 1, 0, 0);
	glRotatef(gRotX, 0, 1, 0);
	glTranslatef(-gCameraLookAt.x, -gCameraLookAt.y, -gCameraLookAt.z);

	glPushMatrix();
	const NxDebugRenderable *dbgRenderable=gScene->getDebugRenderable();
	gDebugRenderer.renderData(*dbgRenderable);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	GLfloat matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	gCameraX = NxVec3(matrix[0],matrix[4],matrix[8]);
	gCameraY = NxVec3(matrix[1],matrix[5],matrix[9]);
	gCameraZ = NxVec3(matrix[2],matrix[6],matrix[10]);

	NxVec3 U(matrix[0],matrix[1],matrix[2]);
	NxVec3 V(matrix[4],matrix[5],matrix[6]);
	NxVec3 N(matrix[8],matrix[9],matrix[10]);
	NxVec3 O(matrix[12],matrix[13],matrix[14]);
	gCameraOrigin.x = -O.dot(U);
	gCameraOrigin.y = -O.dot(V);
	gCameraOrigin.z = -O.dot(N);

	if (gScene)
		{
		for(unsigned int i=0;i<gScene->getNbActors();i++)
			{
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
			DrawActor(gScene->getActors()[i]);
			}

		if (gHeightField)
			{

			const NxHeightFieldShape * hfs = (const NxHeightFieldShape*)gHeightField->getShapes()[0];

			int numVerticesTouched = gTouchedTris.size()*3;
			float* pVertListTouched = new float[numVerticesTouched*3];
			float* pNormListTouched = new float[numVerticesTouched*3];

			int vertIndexTouched = 0;
			int normIndexTouched = 0;

			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			for(NxU32 i = 0; i < gTouchedTris.size(); i++)
				{
				NxU32 triangleIndex = gTouchedTris[i];
				NxU32 flags;
				NxTriangle tri;
				NxTriangle edgeTri;
				hfs->getTriangle(tri, &edgeTri, &flags, triangleIndex, true);
				NxVec3 n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
				n.normalize();
				for(int i=0;i<3;i++)
					{
					pVertListTouched[vertIndexTouched++] = tri.verts[i].x;
					pVertListTouched[vertIndexTouched++] = tri.verts[i].y;
					pVertListTouched[vertIndexTouched++] = tri.verts[i].z;
					pNormListTouched[normIndexTouched++] = n.x;
					pNormListTouched[normIndexTouched++] = n.y;
					pNormListTouched[normIndexTouched++] = n.z;
					}
				}
			
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3,GL_FLOAT, 0, pVertListTouched);
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, pNormListTouched);
			glDrawArrays(GL_TRIANGLES, 0, numVerticesTouched);

			delete[] pVertListTouched;
			delete[] pNormListTouched;

			int numVertices = (hfs->getHeightField().getNbRows()-1)*(hfs->getHeightField().getNbColumns()-1)*3*2;
			float* pVertList = new float[numVertices*3];
			float* pNormList = new float[numVertices*3];

			int vertIndex = 0;
			int normIndex = 0;

			glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
			for(NxU32 row = 0; row < hfs->getHeightField().getNbRows() - 1; row++)
				{
				for(NxU32 column = 0; column < hfs->getHeightField().getNbColumns() - 1; column++)
					{
					NxTriangle tri;
					NxVec3 n;
					NxU32 triangleIndex;

					triangleIndex = 2 * (row * hfs->getHeightField().getNbColumns() + column);

					if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
						{
						n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
						n.normalize();
						for(int i=0;i<3;i++)
							{
							pVertList[vertIndex++] = tri.verts[i].x;
							pVertList[vertIndex++] = tri.verts[i].y;
							pVertList[vertIndex++] = tri.verts[i].z;
							pNormList[normIndex++] = n.x;
							pNormList[normIndex++] = n.y;
							pNormList[normIndex++] = n.z;
							}
						}

					triangleIndex++;
					
					if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
						{
						n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
						n.normalize();
						for(int i=0;i<3;i++)
							{
							pVertList[vertIndex++] = tri.verts[i].x;
							pVertList[vertIndex++] = tri.verts[i].y;
							pVertList[vertIndex++] = tri.verts[i].z;
							pNormList[normIndex++] = n.x;
							pNormList[normIndex++] = n.y;
							pNormList[normIndex++] = n.z;
							}

						}
					}
				}

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3,GL_FLOAT, 0, pVertList);
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, pNormList);
			glDrawArrays(GL_TRIANGLES, 0, numVertices);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);

			delete[] pVertList;
			delete[] pNormList;

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

int main(int argc, char** argv)
	{
	printf("Use left mouse button to orbit.\n");
	printf("Use right mouse button to zoom.\n");
	printf("Use middle mouse button to pan.\n");
	printf("Press p to pause the simulation.\n");
	printf("Press r to reset the scene.\n");
	printf("Press 1 to spawn spheres.\n");
	printf("Press 2 to spawn capsules.\n");
	printf("Press 3 to spawn boxes.\n");
	printf("Press 4 to spawn convexes.\n");
	
	// Initialize Glut
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	int mainHandle = glutCreateWindow("SampleHeightField");
	glutSetWindow(mainHandle);
	glutDisplayFunc(RenderCallback);
	glutReshapeFunc(ReshapeCallback);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(KeyboardCallback);
	glutSpecialFunc(ArrowKeyCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
	MotionCallback(0,0);
	atexit(ExitCallback);

	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(1, -1);

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
	float SpecularColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[] = { 100.0f, 100.0f, 400.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// Physics code
	InitNx();
	// ~Physics code

	// Run
	glutMainLoop();

	return 0;
	}
