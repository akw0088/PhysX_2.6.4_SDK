// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Raycast Sample
// Description: This sample program shows the raycast functionality of the AGEIA
//              PhysX SDK.
//
// ===============================================================================

#define NOMINMAX
#include <GL/glut.h>
#include <stdio.h>

// Physics code
#undef random
#include "NxPhysics.h"
#include "ErrorStream.h"
#include "cooking.h"
#include "Timing.h"
#include "Stream.h"

static bool				gPause = false;
static bool				gUpdateColors = true;
static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static NxVec3			gDefaultGravity(0.0f, -9.81f, 0.0f);
static ErrorStream		gErrorStream;
static NxBounds3		gSceneBounds;
static NxF32			gElapsedTime = 0.0f;

// Render code
static NxVec3 Eye(50.0f, 50.0f, 50.0f);
static NxVec3 Dir(-0.6,-0.2,-0.7);
static NxVec3 N;
static int mx = 0;
static int my = 0;

#define TERRAIN_SIZE		65
#define TERRAIN_NB_VERTS	TERRAIN_SIZE*TERRAIN_SIZE
#define TERRAIN_NB_FACES	(TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*2
#define TERRAIN_OFFSET		-20.0f
#define TERRAIN_WIDTH		3.0f
#define TERRAIN_CHAOS		30.0f
#define ONE_OVER_RAND_MAX	(1.0f / float(RAND_MAX))
#define HALF_RAND_MAX       (RAND_MAX >> 1)
static NxVec3* gTerrainVerts = NULL;
static NxVec3* gTerrainNormals = NULL;
static NxU32* gTerrainFaces = NULL;
static NxVec3* gTerrainColors = NULL;

static void InitTerrain()
{
	// Initialize terrain vertices
	gTerrainVerts = new NxVec3[TERRAIN_NB_VERTS];
	NxU32 y;
	for(y=0;y<TERRAIN_SIZE;y++)
	{
		for(NxU32 x=0;x<TERRAIN_SIZE;x++)
		{
			gTerrainVerts[x+y*TERRAIN_SIZE] = NxVec3(NxF32(x)-(NxF32(TERRAIN_SIZE-1)*0.5f), 0.0f, NxF32(y)-(NxF32(TERRAIN_SIZE-1)*0.5f)) * TERRAIN_WIDTH;
		}
	}

	// Initialize terrain faces
	gTerrainFaces = new NxU32[TERRAIN_NB_FACES*3];
	NxU32 k = 0;
	for(NxU32 j=0;j<TERRAIN_SIZE-1;j++)
	{
		for(NxU32 i=0;i<TERRAIN_SIZE-1;i++)
		{
			// Create first triangle
			gTerrainFaces[k++] = i   + j*TERRAIN_SIZE;
			gTerrainFaces[k++] = i   + (j+1)*TERRAIN_SIZE;
			gTerrainFaces[k++] = i+1 + (j+1)*TERRAIN_SIZE;

			// Create second triangle
			gTerrainFaces[k++] = i   + j*TERRAIN_SIZE;
			gTerrainFaces[k++] = i+1 + (j+1)*TERRAIN_SIZE;
			gTerrainFaces[k++] = i+1 + j*TERRAIN_SIZE;
		}
	}

	// Initialize terrain colors
	gTerrainColors = new NxVec3[TERRAIN_NB_VERTS];
	for(y=0;y<TERRAIN_SIZE;y++)
	{
		for(NxU32 x=0;x<TERRAIN_SIZE;x++)
		{
			gTerrainColors[x+y*TERRAIN_SIZE].zero();
		}
	}

	struct Local
	{
		static void _Compute(bool* done, NxVec3* field, NxU32 x0, NxU32 y0, NxU32 size, NxF32 value)
		{
			// Compute new size
			size>>=1;
			if(!size) return;

			// Compute new heights
			NxF32 v0 = (value * NxF32(rand()-HALF_RAND_MAX) * ONE_OVER_RAND_MAX);
			NxF32 v1 = (value * NxF32(rand()-HALF_RAND_MAX) * ONE_OVER_RAND_MAX);
			NxF32 v2 = (value * NxF32(rand()-HALF_RAND_MAX) * ONE_OVER_RAND_MAX);
			NxF32 v3 = (value * NxF32(rand()-HALF_RAND_MAX) * ONE_OVER_RAND_MAX);
			NxF32 v4 = (value * NxF32(rand()-HALF_RAND_MAX) * ONE_OVER_RAND_MAX);

			NxU32 x1 = (x0+size)		% TERRAIN_SIZE;
			NxU32 x2 = (x0+size+size)	% TERRAIN_SIZE;
			NxU32 y1 = (y0+size)		% TERRAIN_SIZE;
			NxU32 y2 = (y0+size+size)	% TERRAIN_SIZE;

			if(!done[x1 + y0*TERRAIN_SIZE])	field[x1 + y0*TERRAIN_SIZE].y = v0 + 0.5f * (field[x0 + y0*TERRAIN_SIZE].y + field[x2 + y0*TERRAIN_SIZE].y);
			if(!done[x0 + y1*TERRAIN_SIZE])	field[x0 + y1*TERRAIN_SIZE].y = v1 + 0.5f * (field[x0 + y0*TERRAIN_SIZE].y + field[x0 + y2*TERRAIN_SIZE].y);
			if(!done[x2 + y1*TERRAIN_SIZE])	field[x2 + y1*TERRAIN_SIZE].y = v2 + 0.5f * (field[x2 + y0*TERRAIN_SIZE].y + field[x2 + y2*TERRAIN_SIZE].y);
			if(!done[x1 + y2*TERRAIN_SIZE])	field[x1 + y2*TERRAIN_SIZE].y = v3 + 0.5f * (field[x0 + y2*TERRAIN_SIZE].y + field[x2 + y2*TERRAIN_SIZE].y);
			if(!done[x1 + y1*TERRAIN_SIZE])	field[x1 + y1*TERRAIN_SIZE].y = v4 + 0.5f * (field[x0 + y1*TERRAIN_SIZE].y + field[x2 + y1*TERRAIN_SIZE].y);

			done[x1 + y0*TERRAIN_SIZE] = true;
			done[x0 + y1*TERRAIN_SIZE] = true;
			done[x2 + y1*TERRAIN_SIZE] = true;
			done[x1 + y2*TERRAIN_SIZE] = true;
			done[x1 + y1*TERRAIN_SIZE] = true;

			// Recurse through 4 corners
			value *= 0.5f;
			_Compute(done, field, x0,	y0,	size, value);
			_Compute(done, field, x0,	y1,	size, value);
			_Compute(done, field, x1,	y0,	size, value);
			_Compute(done, field, x1,	y1,	size, value);
		}
	};

	// Fractalize
	bool* done = new bool[TERRAIN_NB_VERTS];

	memset(done, 0, TERRAIN_NB_VERTS);
	gTerrainVerts[0].y = 10.0f;
	gTerrainVerts[TERRAIN_SIZE-1].y = 10.0f;
	gTerrainVerts[TERRAIN_SIZE*(TERRAIN_SIZE-1)].y = 10.0f;
	gTerrainVerts[TERRAIN_NB_VERTS-1].y = 10.0f;
	Local::_Compute(done, gTerrainVerts, 0, 0, TERRAIN_SIZE, TERRAIN_CHAOS);
	for(NxU32 i=0;i<TERRAIN_NB_VERTS;i++)	gTerrainVerts[i].y += TERRAIN_OFFSET;

	delete[] done;

	// Build vertex normals
	gTerrainNormals = new NxVec3[TERRAIN_NB_VERTS];
	NxBuildSmoothNormals(TERRAIN_NB_FACES, TERRAIN_NB_VERTS, gTerrainVerts, gTerrainFaces, NULL, gTerrainNormals, true);

	// Build physical model
	NxTriangleMeshDesc terrainDesc;
	terrainDesc.numVertices					= TERRAIN_NB_VERTS;
	terrainDesc.numTriangles				= TERRAIN_NB_FACES;
	terrainDesc.pointStrideBytes			= sizeof(NxVec3);
	terrainDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	terrainDesc.points						= gTerrainVerts;
	terrainDesc.triangles					= gTerrainFaces;							
	terrainDesc.flags						= 0;

	terrainDesc.heightFieldVerticalAxis		= NX_Y;
	terrainDesc.heightFieldVerticalExtent	= -1000.0f;

	NxTriangleMeshShapeDesc terrainShapeDesc;

	bool status = InitCooking();
	if (!status) {
		printf("Unable to initialize the cooking library. Please make sure that you have correctly installed the latest version of the AGEIA PhysX SDK.");
		exit(1);
	}

	MemoryWriteBuffer buf;
	status = CookTriangleMesh(terrainDesc, buf);
	if (!status) {
		printf("Unable to cook a triangle mesh.");
		exit(1);
	}

	MemoryReadBuffer readBuffer(buf.data);
	terrainShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(readBuffer);

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&terrainShapeDesc);
	gScene->createActor(ActorDesc)->userData = (void*)0;

	CloseCooking();
}

static void UpdateColors(bool doShadows)
{
	if(!gUpdateColors)	return;

	// Not really a recommended way to do shadows!
	NxRay ray;
	ray.dir = NxVec3(0.0f, 1.0f, -1.0f);
	ray.dir.normalize();

	// Cast ray for each terrain vertex
	for(int y=0;y<TERRAIN_SIZE;y++)
	{
		for(int x=0;x<TERRAIN_SIZE;x++)
		{
			NxVec3 color;
			ray.orig = gTerrainVerts[x+y*TERRAIN_SIZE];

			if(doShadows && gScene->raycastAnyShape(ray, NX_DYNAMIC_SHAPES))	color.zero();
			else												color = NxVec3(0.5f, 0.4f, 0.2f);

			gTerrainColors[x+y*TERRAIN_SIZE] = color;
		}
	}

	// Blur
	for(int i=0;i<4;i++)
	{
		NxVec3* smoothed = (NxVec3*)malloc(sizeof(NxVec3)*TERRAIN_NB_VERTS);
		memset(smoothed, 0, sizeof(NxVec3)*TERRAIN_NB_VERTS);

		const NxF32 coeff = 1.0f / 5.0f;
		NxU32 offset = TERRAIN_SIZE+1;
		for(NxU32 y=1;y<TERRAIN_SIZE-1;y++)
		{
			for(NxU32 x=1;x<TERRAIN_SIZE-1;x++)
			{
				smoothed[offset]=
				gTerrainColors[offset]*coeff +
				gTerrainColors[offset-TERRAIN_SIZE]*coeff +
				gTerrainColors[offset-1]*coeff +
				gTerrainColors[offset+1]*coeff +
				gTerrainColors[offset+TERRAIN_SIZE]*coeff;

				offset++;
			}
			offset+=2;
		}
		memcpy(gTerrainColors, smoothed, sizeof(NxVec3)*TERRAIN_NB_VERTS);
		free(smoothed);
	}
}

static void RenderTerrain()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float* pVertList = new float[TERRAIN_NB_FACES*3*3];
	float* pNormList = new float[TERRAIN_NB_FACES*3*3];
	float* pColorList = new float[TERRAIN_NB_FACES*4*3];

	int vertIndex = 0;
	int normIndex = 0;
	int colorIndex = 0;
	for(int i=0;i<TERRAIN_NB_FACES;i++)
	{
		for(int j=0;j<3;j++)
		{
		pVertList[vertIndex++] = gTerrainVerts[gTerrainFaces[i*3+j]].x;
        pVertList[vertIndex++] = gTerrainVerts[gTerrainFaces[i*3+j]].y;
        pVertList[vertIndex++] = gTerrainVerts[gTerrainFaces[i*3+j]].z;

        pNormList[normIndex++] = gTerrainNormals[gTerrainFaces[i*3+j]].x;
        pNormList[normIndex++] = gTerrainNormals[gTerrainFaces[i*3+j]].y;
        pNormList[normIndex++] = gTerrainNormals[gTerrainFaces[i*3+j]].z;

		pColorList[colorIndex++] = gTerrainColors[gTerrainFaces[i*3+j]].x;
		pColorList[colorIndex++] = gTerrainColors[gTerrainFaces[i*3+j]].y;
		pColorList[colorIndex++] = gTerrainColors[gTerrainFaces[i*3+j]].z;
		pColorList[colorIndex++] = 1.0f;
		}
	}

    glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT, 0, pVertList);
    glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, pNormList);
    glColorPointer(4,GL_FLOAT, 0, pColorList);
    glEnableClientState(GL_COLOR_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, TERRAIN_NB_FACES*3);
	glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

	delete[] pVertList;
	delete[] pNormList;
	delete[] pColorList;
}

static void RaycastClosestShapes()
{
	static NxF32 t = 0.0f;
	if(!gPause)	t += gElapsedTime;
	NxF32 offset = cosf(t) * 100.0f;

	glDisable(GL_LIGHTING);

	NxRay worldRay;
	worldRay.dir = NxVec3(0.0f, -1.0f, 0.0f);
//	worldRay.dir = NxVec3(0.0f, -1.0f, 0.2f);
//	worldRay.dir.normalize();

	for(NxU32 i=0;i<100;i++)
	{
		worldRay.orig = NxVec3((NxF32(i)-49.5f)*2.0f, 100.0f, 0.0f);
//		worldRay.orig = NxVec3((NxF32(i)-49.5f)*2.0f, 3000.0f, 0.0f);
		worldRay.orig.z += offset;

		NxRaycastHit hit;
		NxF32 dist;
		NxShape* closestShape = gScene->raycastClosestShape(worldRay, NX_ALL_SHAPES, hit);

		if(closestShape)
		{
			const NxVec3& worldImpact = hit.worldImpact;
			dist = hit.distance;

			if(hit.faceID != 0)
			{
				NxU32 vref0 = gTerrainFaces[hit.faceID*3+0];
				NxU32 vref1 = gTerrainFaces[hit.faceID*3+1];
				NxU32 vref2 = gTerrainFaces[hit.faceID*3+2];
				const NxVec3& v0 = gTerrainVerts[vref0];
				const NxVec3& v1 = gTerrainVerts[vref1];
				const NxVec3& v2 = gTerrainVerts[vref2];

				glEnableClientState(GL_VERTEX_ARRAY);
				float tris[3][3] = { {v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}};
				glVertexPointer(3, GL_FLOAT, 0, tris);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glDisableClientState(GL_VERTEX_ARRAY);
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			float lineX[2][3] = { {worldImpact.x, worldImpact.y, worldImpact.z}, {worldImpact.x+1.0f, worldImpact.y, worldImpact.z}};
			glVertexPointer(3, GL_FLOAT, 0, lineX);
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glDrawArrays(GL_LINES, 0, 2);
			float lineY[2][3] = { {worldImpact.x, worldImpact.y, worldImpact.z}, {worldImpact.x, worldImpact.y+1.0f, worldImpact.z}};
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glVertexPointer(3, GL_FLOAT, 0, lineY);
			glDrawArrays(GL_LINES, 0, 2);
			float lineZ[2][3] = { {worldImpact.x, worldImpact.y, worldImpact.z}, {worldImpact.x, worldImpact.y, worldImpact.z+1.0f}};
			glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
			glVertexPointer(3, GL_FLOAT, 0, lineZ);
			glDrawArrays(GL_LINES, 0, 2);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
		else dist = 1000.0f;

		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
		glEnableClientState(GL_VERTEX_ARRAY);
		float line[2][3] = {	{worldRay.orig.x, worldRay.orig.y, worldRay.orig.z}, 
								{worldRay.orig.x+worldRay.dir.x*dist, worldRay.orig.y+worldRay.dir.y*dist, worldRay.orig.z+worldRay.dir.z*dist}};
		glVertexPointer(3, GL_FLOAT, 0, line);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableClientState(GL_VERTEX_ARRAY);	
	}
	glEnable(GL_LIGHTING);
}

class myRaycastReport : public NxUserRaycastReport
{
	virtual bool onHit(const NxRaycastHit& hit)
	{
		int userData = (int)hit.shape->getActor().userData;
		userData |= 1;	// Mark as hit
		hit.shape->getActor().userData = (void*)userData;

		return true;
	}
}gMyReport;

static void RaycastAllShapes()
{
	static NxF32 t = 0.0f;
	if(!gPause)	t += gElapsedTime;
	NxF32 offset = cosf(t) * 20.0f;

	NxRay worldRay;
	worldRay.dir = NxVec3(1.0f, 0.0f, 0.0f);
	worldRay.orig = NxVec3(-49.5f, 10.0f+offset, 10.0f);

	gScene->raycastAllShapes(worldRay, gMyReport, NX_ALL_SHAPES);

	glDisable(GL_LIGHTING);

	NxF32 dist = 5000.0f;	// Actually infinite

	glEnableClientState(GL_VERTEX_ARRAY);
	float line[2][3] = {	{worldRay.orig.x, worldRay.orig.y, worldRay.orig.z}, 
							{worldRay.orig.x+worldRay.dir.x*dist, worldRay.orig.y+worldRay.dir.y*dist, worldRay.orig.z+worldRay.dir.z*dist}};
	glVertexPointer(3, GL_FLOAT, 0, line);
	glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);	

	glEnable(GL_LIGHTING);
}

static void CreateCube(const NxVec3& pos, int size=2, const NxVec3* initial_velocity=NULL)
{
	if(!gSceneBounds.contain(pos))
	{
		printf("Objects is out of world - creation aborted!\n");
		return;
	}

	// Create body
	NxBodyDesc BodyDesc;
	BodyDesc.angularDamping	= 0.5f;
	if(initial_velocity) BodyDesc.linearVelocity = *initial_velocity;

	NxBoxShapeDesc BoxDesc;
	BoxDesc.dimensions = NxVec3(float(size), float(size), float(size));

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&BoxDesc);
	ActorDesc.body			= &BodyDesc;
	ActorDesc.density		= 10.0f;
	ActorDesc.globalPose.t  = pos;
	NxActor* a = gScene->createActor(ActorDesc);
	if(a)
	{
		a->userData = (void*)(size<<1);
	}
}

static void CreateStack(int size)
{
	float CubeSize = 2.0f;
	float Spacing = 0.0001f;
	NxVec3 Pos(0.0f, CubeSize, -5.0f);
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
	sceneDesc.gravity		= gDefaultGravity;

	// Setting proper scene bounds speeds up raycasts against dynamic actors
	gSceneBounds.min = NxVec3(-100.0f, -50.0f, -100.0f);
	gSceneBounds.max = NxVec3(100.0f, 150.0f, 100.0f);
	sceneDesc.maxBounds		= &gSceneBounds;
	sceneDesc.boundsPlanes	= true;

	gScene = gPhysicsSDK->createScene(sceneDesc);

	NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);

	InitTerrain();
}

static void KeyboardCallback(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:  exit(0); break;
	case ' ': CreateCube(NxVec3(0.0f, 30.0f, 0.0f), 1+(rand()&3)); break;
	case 's': CreateStack(10); break;
	case 'b': CreateStack(30); break;
	case 't': CreateTower(30); break;
	case 'p': gPause = !gPause; break;
	case 'c': if (gUpdateColors)
				  UpdateColors(false); //Redo colors without shadows
		gUpdateColors = !gUpdateColors; 
		break;
	case 101: case '8':	Eye += Dir * 2.0f; break;
	case 103: case '2':	Eye -= Dir * 2.0f; break;
	case 100: case '4':	Eye -= N * 2.0f; break;
	case 102: case '6':	Eye += N * 2.0f; break;
	case 'w':
		{
		NxVec3 t = Eye;
		NxVec3 Vel = Dir;
		Vel.normalize();
		Vel*=200.0f;
		CreateCube(t, 8, &Vel);
		}
		break;
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
	if(gScene == NULL) return;
	gElapsedTime = getElapsedTime();

	// Physics code
	if(!gPause)	
	{
		UpdateColors(true);
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
	// PT: keep the look-at code in the projection matrix to have a sane lighting setup.
	// *DO* *NOT* *MOVE* *THIS*
	gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	RaycastClosestShapes();
	RaycastAllShapes();

	// Keep physics & graphics in sync
	int nbActors = gScene->getNbActors();
	NxActor** actors = gScene->getActors();
	while(nbActors--)
	{
		NxActor* actor = *actors++;

		int userData = int(actor->userData);
		int touched = userData&1;
		userData&=~1;
		actor->userData = (void*)userData;

		float size = float(userData>>1);
		if(touched)	glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
		else		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glPushMatrix();
		float glmat[16];
		actor->getGlobalPose().getColumnMajor44(glmat);
		glMultMatrixf(glmat);
		glutSolidCube(size*2.0f);
		glPopMatrix();
	}

	RenderTerrain();

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
	printf("Press the keys w, space, s, b, and t to create various things.\n");
	printf("Use the arrow keys or 2, 4, 6 and 8 to move the camera, mouse to look around.\n");
	printf("Note: this demo is slow because it computes soft shadows for the boxes using raycasting!\n");
	printf("Press p to pause\n");
	printf("Press c to enable/disable shadows\n");

	// Initialize Glut
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	int mainHandle = glutCreateWindow("SampleRaycast");
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

	// Setup default render states
	glClearColor(0.3f, 0.4f, 0.5f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);

	// Setup lighting
	glEnable(GL_LIGHTING);
	float AmbientColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };		glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	float DiffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	float SpecularColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[] = { -10.0f, 100.0f, -4.0f, 1.0f };		glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	// Physics code
	InitNx();
	// ~Physics code

	// Run
	glutMainLoop();

	return 0;
}
