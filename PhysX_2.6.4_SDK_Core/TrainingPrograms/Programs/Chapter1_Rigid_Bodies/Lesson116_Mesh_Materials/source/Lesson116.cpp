// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							 LESSON 116: MESH MATERIALS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson116.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;
extern ErrorStream       gErrorStream;

// HUD globals
extern HUD hud;

// Force globals
extern NxVec3 gForceVec;
extern NxReal gForceStrength;
extern bool bForceMode;

// Simulation globals
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Camera globals
extern NxVec3 gCameraPos;
extern NxReal gCameraSpeed;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;
NxActor* terrain = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Terrain globals
#define TERRAIN_SIZE		35
#define TERRAIN_NB_VERTS	TERRAIN_SIZE*TERRAIN_SIZE
#define TERRAIN_NB_FACES	(TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*2
#define TERRAIN_OFFSET		-20.0f
#define TERRAIN_WIDTH		20.0f
#define TERRAIN_CHAOS		150.0f
#define ONE_OVER_RAND_MAX	(1.0f / float(RAND_MAX))
NxVec3* gTerrainVerts = NULL;
NxVec3* gTerrainNormals = NULL;
NxU32* gTerrainFaces = NULL;
NxMaterialIndex* gTerrainMaterials = NULL;
NxMaterialIndex materialDefault = 0;
NxMaterialIndex materialIce = 1;
NxMaterialIndex materialRock = 2;
NxMaterialIndex materialMud = 3;
NxMaterialIndex materialGrass = 4;
NxArray<NxVec3> contactBuffer;

class ContactReport : public NxUserContactReport
{
public:

	virtual NxU32 onPairCreated(NxActor& s1, NxActor& s2)
	{
		return NX_NOTIFY_ON_START_TOUCH;
	}

	virtual void  onContactNotify(NxContactPair& pair, NxU32 events)
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
#if 0
					NxU32 extData = i.getExtData();
					if (
						   (((extData & 0xffff) == materialIce) && ((extData >> 16) == materialDefault))
						|| (((extData & 0xffff) == materialDefault) && ((extData >> 16) == materialIce))
						)
					{
						// We may not call the SDK again from a callback, so we just save the information we need here:
						contactBuffer.pushBack(contactPoint);
					}
#endif  // 0
				}
			}
		}
	}

} gContactReport;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\nt = Move Focus Actor to (0,5,0)\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		// Don't render terrain mesh here
		if (((ActorUserData*)(actor->userData))->flags & UD_NO_RENDER)  continue;

	    DrawActor(actor, gSelectedActor, true);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, true);
        }
    }
}

void ProcessInputs()
{
    ProcessForceKeys();

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
		if (gScene)  gDebugRenderer.renderData(*gScene->getDebugRenderable());
	}
}

float* gTerrainColors = NULL;
float* gTerrainVs = NULL;

void setTerrainColor(int vertIndex, float r, float g, float b)
{
    gTerrainColors[vertIndex*4+0] = r;
    gTerrainColors[vertIndex*4+1] = g;
    gTerrainColors[vertIndex*4+2] = b;
    gTerrainColors[vertIndex*4+3] = 1.0f;
}
void setTerrainVertex(int vertIndex, NxVec3 v)
{
    gTerrainVs[vertIndex*3+0] = v.x;
    gTerrainVs[vertIndex*3+1] = v.y;
    gTerrainVs[vertIndex*3+2] = v.z;
}    

void RenderTerrain()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
    int pos = 0;
    for(int ii=0;ii<TERRAIN_NB_FACES;ii++)
    {
        setTerrainVertex(pos+0,gTerrainVerts[gTerrainFaces[ii*3+0]]);
        setTerrainVertex(pos+1,gTerrainVerts[gTerrainFaces[ii*3+1]]);
        setTerrainVertex(pos+2,gTerrainVerts[gTerrainFaces[ii*3+2]]);

	    NxMaterialIndex mat = gTerrainMaterials[ii];
	    if (mat == materialIce) {
            setTerrainColor(pos++,1,1,1);
            setTerrainColor(pos++,1,1,1);
            setTerrainColor(pos++,1,1,1);
            
        }
	    else if (mat == materialRock)
        {
            setTerrainColor(pos++,0.3f,0.3f,0.3f);
            setTerrainColor(pos++,0.3f,0.3f,0.3f);
            setTerrainColor(pos++,0.3f,0.3f,0.3f);
        }
		else if (mat == materialMud)
        {
            setTerrainColor(pos++,0.6f,0.3f,0.2f);
            setTerrainColor(pos++,0.6f,0.3f,0.2f);
            setTerrainColor(pos++,0.6f,0.3f,0.2f);
        }
		else
        {
            setTerrainColor(pos++,0.2f,0.8f,0.2f);
            setTerrainColor(pos++,0.2f,0.8f,0.2f);
            setTerrainColor(pos++,0.2f,0.8f,0.2f);
        }

    }
    
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, gTerrainVs);
    glColorPointer(4, GL_FLOAT, 0, gTerrainColors);
    glDrawArrays(GL_TRIANGLES,0,TERRAIN_NB_FACES*3);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glColor4f(1.0f,1.0f,1.0f,1.0f);

	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);
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

	// Draw the terrain
    RenderTerrain();

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	// Render the HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
}

void SmoothTriangle(NxU32 a, NxU32 b, NxU32 c)
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

void ChooseTrigMaterial(NxU32 faceIndex)
{
	NxVec3& v0 = gTerrainVerts[gTerrainFaces[faceIndex * 3]];
	NxVec3& v1 = gTerrainVerts[gTerrainFaces[faceIndex * 3 + 1]];
	NxVec3& v2 = gTerrainVerts[gTerrainFaces[faceIndex * 3 + 2]];

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
	{
		gTerrainMaterials[faceIndex] = materialGrass;
	}
}

NxActor* CreateTerrain()
{
	// Initialize terrain vertices
	gTerrainVerts = new NxVec3[TERRAIN_NB_VERTS];
	for (NxU32 y = 0; y < TERRAIN_SIZE; y++)
	{
		for (NxU32 x = 0; x < TERRAIN_SIZE; x++)
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
	gTerrainVerts[TERRAIN_SIZE*(TERRAIN_SIZE-1)].y = 10.0f;
	gTerrainVerts[TERRAIN_NB_VERTS-1].y = 10.0f;
	Local::_Compute(done, gTerrainVerts, 0, 0, TERRAIN_SIZE, TERRAIN_CHAOS);
	for (NxU32 i=0;i<TERRAIN_NB_VERTS;i++)  gTerrainVerts[i].y += TERRAIN_OFFSET;
	NX_DELETE_ARRAY(done);

	// Initialize terrain faces
	gTerrainFaces = new NxU32[TERRAIN_NB_FACES*3];

	NxU32 k = 0;
	for(NxU32 j=0;j<TERRAIN_SIZE-1;j++)
	{
		for(NxU32 i=0;i<TERRAIN_SIZE-1;i++)
		{
			// Create first triangle
			gTerrainFaces[k] = i + j*TERRAIN_SIZE;
			gTerrainFaces[k+1] = i + (j+1)*TERRAIN_SIZE;
			gTerrainFaces[k+2] = i+1 + (j+1)*TERRAIN_SIZE;

			// While we're at it do some smoothing of the random terrain because its too rough to do a good demo of this effect.
			SmoothTriangle(gTerrainFaces[k],gTerrainFaces[k+1],gTerrainFaces[k+2]);
			k+=3;

			// Create second triangle
			gTerrainFaces[k] = i   + j*TERRAIN_SIZE;
			gTerrainFaces[k+1] = i+1 + (j+1)*TERRAIN_SIZE;
			gTerrainFaces[k+2] = i+1 + j*TERRAIN_SIZE;

			SmoothTriangle(gTerrainFaces[k],gTerrainFaces[k+1],gTerrainFaces[k+2]);
			k+=3;
		}
	}


	// Also make a color array, one for each vertex. 
	gTerrainColors = new float [TERRAIN_NB_FACES*4*3];
    gTerrainVs = new float [TERRAIN_NB_FACES*3*3];

	// Allocate terrain materials -- one for each face.
	gTerrainMaterials = new NxMaterialIndex[TERRAIN_NB_FACES];

	for(NxU32 f=0;f<TERRAIN_NB_FACES;f++)
	{
		// New: generate material indices for all the faces
		ChooseTrigMaterial(f);
	}

	// Build vertex normals
//	gTerrainNormals = new NxVec3[TERRAIN_NB_VERTS];
//	NxBuildSmoothNormals(TERRAIN_NB_FACES, TERRAIN_NB_VERTS, gTerrainVerts, gTerrainFaces, NULL, gTerrainNormals, true);

	// Build physical model
	NxTriangleMeshDesc terrainDesc;
	terrainDesc.numVertices					= TERRAIN_NB_VERTS;
	terrainDesc.numTriangles				= TERRAIN_NB_FACES;
	terrainDesc.pointStrideBytes			= sizeof(NxVec3);
	terrainDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	terrainDesc.points						= gTerrainVerts;
	terrainDesc.triangles					= gTerrainFaces;							
//	terrainDesc.flags						= NX_MF_HARDWARE_MESH;

	// Add the mesh material data
	terrainDesc.materialIndexStride			= sizeof(NxMaterialIndex);
	terrainDesc.materialIndices				= gTerrainMaterials;

	terrainDesc.heightFieldVerticalAxis		= NX_Y;
	terrainDesc.heightFieldVerticalExtent	= -1000.0f;

	NxTriangleMeshShapeDesc terrainShapeDesc;
//	terrainShapeDesc.materialIndex			= materialIce;

    NxInitCooking();
    if (0)
    {
        // Cooking from file
#ifdef WIN32
        bool status = NxCookTriangleMesh(terrainDesc, UserStream("c:\\tmp.bin", false));
        terrainShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
#endif
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookTriangleMesh(terrainDesc, buf);
        terrainShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
    }

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&terrainShapeDesc);
	NxActor* actor = gScene->createActor(actorDesc);

    return actor;
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 3);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity*3;
	sceneDesc.userContactReport		= &gContactReport;
    sceneDesc.simType				= NX_SIMULATION_HW;

    gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}

	// If hardware is unavailable, fall back on software
	if (!gScene)
	{
        sceneDesc.simType				= NX_SIMULATION_SW;
        gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}
	}

	// Create the default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0);
	defaultMaterial->setStaticFriction(0.5);
	defaultMaterial->setDynamicFriction(0.5);

    // Terrain materials
	NxMaterialDesc m;
	m.restitution = 1;
	m.staticFriction = 0;
	m.dynamicFriction = 0;
	materialIce = gScene->createMaterial(m)->getMaterialIndex();
	m.restitution = 0.3;
	m.staticFriction = 1.2;
	m.dynamicFriction = 1;
	materialRock = gScene->createMaterial(m)->getMaterialIndex();
	m.restitution = 0;
	m.staticFriction = 3;
	m.dynamicFriction = 1;
	materialMud = gScene->createMaterial(m)->getMaterialIndex();
	m.restitution = 0;
	m.staticFriction = 0;
	m.dynamicFriction = 0;
	materialGrass = gScene->createMaterial(m)->getMaterialIndex();

	// Create the terrain
	terrain = CreateTerrain();

#if 0
	// Page in the terrain mesh to the hardware
	NxShape*const* shapes = terrain->getShapes();
	for (unsigned int s = 0; s < terrain->getNbShapes(); ++s )
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
#endif  // 0
	
	// Create the objects in the scene
	box = CreateBox(NxVec3(20,0,0), NxVec3(2.5,5,2.5), 1);
	sphere = CreateSphere(NxVec3(0,0,20), 4, 1);
	capsule = CreateCapsule(NxVec3(-20,0,0), 5, 2, 2);
	pyramid = CreatePyramid(NxVec3(0,0,-20), NxVec3(2.5,2,2.5), 2);

	AddUserDataToActors(gScene);

	((ActorUserData*)(terrain->userData))->flags |= UD_NO_RENDER;

	gSelectedActor = pyramid;
    gForceStrength = 750000;

	bShadows = false;

    gCameraPos = NxVec3(0,5,-100);
    gCameraSpeed = 50;

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

void AddTerrainLight()
{
    float AmbientColor1[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientColor1);
    float DiffuseColor1[]    = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseColor1);
    float SpecularColor1[]   = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularColor1);
    float Position1[]        = { -25.0f, 55.0f, 15.0f, 1.0f };	   glLightfv(GL_LIGHT1, GL_POSITION, Position1);
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 116: Mesh Materials");
	AddTerrainLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

