#include "SampleRaycastCar.h"


#define TERRAIN_SIZE		33
#define TERRAIN_NB_VERTS	TERRAIN_SIZE*TERRAIN_SIZE
#define TERRAIN_NB_FACES	(TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*2
#define TERRAIN_OFFSET		-20.0f
#define TERRAIN_WIDTH		20.0f
#define TERRAIN_CHAOS		70.0f //150.0f
#define ONE_OVER_RAND_MAX	(1.0f / float(RAND_MAX))
static NxVec3* gTerrainVerts = NULL;
static NxVec3* gTerrainNormals = NULL;
static NxU32* gTerrainFaces = NULL;
static NxMaterialIndex * gTerrainMaterials = NULL;
NxMaterialIndex materialIce, materialRock, materialMud, materialGrass, materialDefault = 0;
static NxActor * gTerrain = NULL;

#include "NxCooking.h"
#include "Stream.h"

void smoothTriangle(NxU32 a, NxU32 b, NxU32 c)
	{
	NxVec3 & v0 = gTerrainVerts[a];
	NxVec3 & v1 = gTerrainVerts[b];
	NxVec3 & v2 = gTerrainVerts[c];

	NxReal avg = (v0.y + v1.y + v2.y) * 0.333f;
	avg *= 0.5f;
	v0.y = v0.y * 0.5f + avg;
	v1.y = v1.y * 0.5f + avg;
	v2.y = v2.y * 0.5f + avg;
	}

void chooseTrigMaterial(NxU32 faceIndex)
	{
	NxVec3 & v0 = gTerrainVerts[gTerrainFaces[faceIndex * 3]];
	NxVec3 & v1 = gTerrainVerts[gTerrainFaces[faceIndex * 3 + 1]];
	NxVec3 & v2 = gTerrainVerts[gTerrainFaces[faceIndex * 3 + 2]];

	NxVec3 edge0 = v1 - v0;
	NxVec3 edge1 = v2 - v0;

	NxVec3 normal = edge0.cross(edge1);
	normal.normalize();
	NxReal steepness = 1.0f - normal.y;

	if	(steepness > 0.25f)
		{
		gTerrainMaterials[faceIndex] = materialIce;
		}
	else if (steepness > 0.2f)
		{
		gTerrainMaterials[faceIndex] = materialRock;
		
		}
	else if (steepness > 0.1f)
		{
		gTerrainMaterials[faceIndex] = materialMud;
		}
	else
		gTerrainMaterials[faceIndex] = materialGrass;
	}

void InitTerrain()
{

	NxMaterialDesc	m;

	NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);
		

	//terrain materials:

	// Ice
	m.restitution		= 1.0f;
	m.staticFriction	= 0.05f;
	m.dynamicFriction	= 0.05f;
	materialIce = gScene->createMaterial(m)->getMaterialIndex();
	// Rock
	m.restitution		= 0.3f;
	m.staticFriction	= 1.2f;
	m.dynamicFriction	= 0.9f;
	materialRock = gScene->createMaterial(m)->getMaterialIndex();
	// Mud
	m.restitution		= 0.0f;
	m.staticFriction	= 0.8f;
	m.dynamicFriction	= 0.2f;
	materialMud = gScene->createMaterial(m)->getMaterialIndex();
	// Grass
	m.restitution		= 0.1f;
	m.staticFriction	= 0.4f;
	m.dynamicFriction	= 0.4f;
	materialGrass = gScene->createMaterial(m)->getMaterialIndex();

	// Initialize terrain vertices
	gTerrainVerts = new NxVec3[TERRAIN_NB_VERTS];
	for(NxU32 y=0;y<TERRAIN_SIZE;y++)
	{
		for(NxU32 x=0;x<TERRAIN_SIZE;x++)
		{
			NxVec3 & v = gTerrainVerts[x+y*TERRAIN_SIZE];
			v.set(NxF32(x)-(NxF32(TERRAIN_SIZE-1)*0.5f), 0.0f, NxF32(y)-(NxF32(TERRAIN_SIZE-1)*0.5f));
			v*= TERRAIN_WIDTH;
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
			NxF32 v0 = NxMath::rand(-value, value);
			NxF32 v1 = NxMath::rand(-value, value);
			NxF32 v2 = NxMath::rand(-value, value);
			NxF32 v3 = NxMath::rand(-value, value);
			NxF32 v4 = NxMath::rand(-value, value);

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

	memset(done,0,TERRAIN_NB_VERTS);
	gTerrainVerts[0].y = 10.0f;
	gTerrainVerts[TERRAIN_SIZE-1].y = 10.0f;
	gTerrainVerts[TERRAIN_SIZE*(TERRAIN_SIZE-1)].y = 5.0f;
	gTerrainVerts[TERRAIN_NB_VERTS-1].y = 10.0f;
	Local::_Compute(done, gTerrainVerts, 0, 0, TERRAIN_SIZE, TERRAIN_CHAOS);
	for(NxU32 i=0;i<TERRAIN_NB_VERTS;i++)	
		gTerrainVerts[i].y += TERRAIN_OFFSET;

	delete[] done;
	

	// Initialize terrain faces
	gTerrainFaces = new NxU32[TERRAIN_NB_FACES*3];

	NxU32 k = 0;
	for(NxU32 j=0;j<TERRAIN_SIZE-1;j++)
	{
		for(NxU32 i=0;i<TERRAIN_SIZE-1;i++)
		{
			// Create first triangle
			gTerrainFaces[k] = i   + j*TERRAIN_SIZE;
			gTerrainFaces[k+1] = i   + (j+1)*TERRAIN_SIZE;
			gTerrainFaces[k+2] = i+1 + (j+1)*TERRAIN_SIZE;

			//while we're at it do some smoothing of the random terrain because its too rough to do a good demo of this effect.
			//smoothTriangle(gTerrainFaces[k],gTerrainFaces[k+1],gTerrainFaces[k+2]);
			k+=3;
			// Create second triangle
			gTerrainFaces[k] = i   + j*TERRAIN_SIZE;
			gTerrainFaces[k+1] = i+1 + (j+1)*TERRAIN_SIZE;
			gTerrainFaces[k+2] = i+1 + j*TERRAIN_SIZE;

			//smoothTriangle(gTerrainFaces[k],gTerrainFaces[k+1],gTerrainFaces[k+2]);
			k+=3;
		}
	}

	//allocate terrain materials -- one for each face.
	gTerrainMaterials = new NxMaterialIndex[TERRAIN_NB_FACES];

	for(NxU32 f=0;f<TERRAIN_NB_FACES;f++)
		{
		//new: generate material indices for all the faces
		chooseTrigMaterial(f);
		}
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
	//add the mesh material data:
	terrainDesc.materialIndexStride			= sizeof(NxMaterialIndex);
	terrainDesc.materialIndices				= gTerrainMaterials;


	terrainDesc.heightFieldVerticalAxis		= NX_Y;
	terrainDesc.heightFieldVerticalExtent	= -1000.0f;

	NxTriangleMeshShapeDesc terrainShapeDesc;
	NxInitCooking();
	bool status = NxCookTriangleMesh(terrainDesc, UserStream("c:\\tmp.bin", false));
	terrainShapeDesc.meshData				= gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&terrainShapeDesc);
	gTerrain = gScene->createActor(ActorDesc);
	gTerrain->userData = (void*)0;
}

void RenderTerrain()
{
  glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	//glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	
	for(NxU32 i=0;i<TERRAIN_NB_FACES;i++)
	{
	NxMaterialIndex mat = gTerrainMaterials[i];


	if		(mat == materialIce)	glColor4f(1,1,1,1);
	else if (mat == materialRock)	glColor4f(0.3,0.3,0.3,1.0); 
	else if (mat == materialMud)	glColor4f(0.6f,0.3f,0.2f,1.0f); 
	else							glColor4f(0.2f,0.8f,0.2f,1.0f);	
		
		glNormal3fv(&gTerrainNormals[gTerrainFaces[i*3+0]].x);
		glVertex3fv(&gTerrainVerts[gTerrainFaces[i*3+0]].x);

		glNormal3fv(&gTerrainNormals[gTerrainFaces[i*3+1]].x);
		glVertex3fv(&gTerrainVerts[gTerrainFaces[i*3+1]].x);

		glNormal3fv(&gTerrainNormals[gTerrainFaces[i*3+2]].x);
		glVertex3fv(&gTerrainVerts[gTerrainFaces[i*3+2]].x);
	}
	glEnd();
	glPopMatrix();
}
