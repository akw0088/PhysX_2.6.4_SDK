// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Hardware Fluid
// Description: This sample program shows how to use fluids in a hardware scene
//
// Originally written by: Simon Schirm (01.09.04)
//
// ===============================================================================

#define NOMINMAX
#ifdef WIN32 
#include <windows.h>
#elif LINUX
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>

// Physics code
#undef random
#include "NxPhysics.h"
#include "ErrorStream.h"

#include "Stream.h"
#include "NxCooking.h"

#include "DrawObjects.h"
#include "CookASE.h"
#include "DebugRenderer.h"
#include "Utilities.h"

#define REST_PARTICLES_PER_METER 10.f
#define KERNEL_RADIUS_MULTIPLIER 1.8f
//#define MOTION_LIMIT_MULTIPLIER (3*KERNEL_RADIUS_MULTIPLIER)
#define MOTION_LIMIT_MULTIPLIER 3
#define PACKET_SIZE_MULTIPLIER 8
#define COLLISION_DISTANCE_MULTIPLIER 0.18f

static bool				gPause = false;
static bool				gDebugRender = false;
static int				gPoints = 0;
static bool             gUpdateCamera = true;
static int              gDrawSurface = 0;
static int              gDrawSurfaceFW = 1;
static int              gDrawSurfaceSW = 0;
static bool             gDrawScene = true;
static bool             gSingleStep = false;

static bool				gSceneRunning = false;
static bool				gUseHW = 1;

static bool				up = false;
static bool				down = false;
static bool				left = false;
static bool				right = false;

static NxPhysicsSDK*	gPhysicsSDK = NULL;
static NxScene*			gScene = NULL;
static NxVec3			gDefaultGravity(0.0f, -9.81f, 0.0f);

static ErrorStream			gErrorStream;
static DebugRenderer		gDebugRenderer;

static NxFluid*			gFluid = NULL;
static NxImplicitScreenMesh* gFluidMesh = NULL;
static NxImplicitScreenMesh* gParticleMesh = NULL;
static NxReal           gProjMatrix[16];
const  int              NUM_VERTS = 400000;
//static float            gVertices[NUM_VERTS * 3];
//static float            gNormals[NUM_VERTS * 3];
struct SSimpleVertex
{
NxVec3 v3Position;
NxVec3 v3Normal; 
char    otherArbitraryData[5];
};
static SSimpleVertex gFullVerts[NUM_VERTS * 3];

static NxU16            gIndices[NUM_VERTS];
static int              gWidth;
static int              gHeight;
static NxU32            gNumVertices;
static NxU32            gNumIndices;

static float            gParticleVertices[NUM_VERTS * 3];
static float            gParticleNormals[NUM_VERTS * 3];
static NxU16            gParticleIndices[NUM_VERTS];
static NxU32            gParticleNumVertices;
static NxU32            gParticleNumIndices;

struct Particle
{
	NxVec3	pos;
	NxU32	id;
	NxReal	density;
};

static Particle			gParticleBuffer[32766];
static unsigned			gParticleBufferCap = 32766;
static unsigned			gParticleBufferNum = 0;

static unsigned			gCreatedIdsBuffer[32766];
static unsigned			gCreatedIdsBufferCap = 32766;
static unsigned			gCreatedIdsBufferNum = 0;

struct UserParticleData
{
	NxVec3 color;
	NxU32  particleIndex;
};

static UserParticleData		gParticleUserData[32766];
static unsigned				gParticleUserDataNum = 0;

/////////////////////////////////////////////////////////////////////////////////////////////
// Fluid Surface support NX_MDF_16_BIT_INDICES or NX_MDF_32_BIT_INDICES(default)
// but here developers have to make sure they are using the same type for these buffers
// allocated by themself.
// for example,
// NX_MDF_16_BIT_INDICES enabled, then in the above, we have to define the indices like this,
// static NxU16            gIndices[NUM_VERTS]; instead of static int gIndices[NUM_VERTS]
//////////////////////////////////////////////////////////////////////////////////////////////

// Data Directory Paths (solution, binary, install)
char fname[] = "../../data";
char fname2[] = "../../Samples/SampleHardwareFluid/data";
char fname3[] = "Samples/SampleHardwareFluid/data";


// Actor globals
NxActor* flatSurface;

// Mesh globals
NxTriangleMeshDesc flatSurfaceTriangleMeshDesc;

static int gMainHandle;
static float gRatio = 1.0f;
static NxVec3 Eye(5.0f, 2.0f, 5.0f);
static NxVec3 Dir(-0.6,-0.2,-0.7);
static NxVec3 N;
static int mx = 0;
static int my = 0;


static void CreateFluid()
{
	//Create a set of particles
	NxVec3 pos(0,1.5,0);
	float distance = 0.1f;
	gParticleBufferNum = 0;
	unsigned sideNum = 16;
	float rad = sideNum*distance*0.5f;
	for (unsigned i=0; i<sideNum; i++)
		for (unsigned j=0; j<sideNum; j++)
			for (unsigned k=0; k<sideNum; k++)
			{
				NxVec3 p = NxVec3(i*distance,j*distance,k*distance);
				if (p.distance(NxVec3(rad,rad,rad)) < rad)
				{
					p += pos;
					gParticleBuffer[gParticleBufferNum++].pos = p;
				}
			}

	//Set structure to pass particles, and receive them after every simulation step
    NxParticleData particles;
	particles.maxParticles				= gParticleBufferNum;
	particles.numParticlesPtr			= &gParticleBufferNum;
	particles.bufferPos					= &gParticleBuffer[0].pos.x;
	particles.bufferDensity				= &gParticleBuffer[0].density;
	particles.bufferId					= &gParticleBuffer[0].id;
	particles.bufferPosByteStride		= sizeof(Particle);
	particles.bufferDensityByteStride	= sizeof(Particle);
	particles.bufferIdByteStride		= sizeof(Particle);

	//Set structure to receive IDs of created particles every simulation step
	NxParticleIdData createdIds;
	createdIds.maxIds				= gParticleBufferNum;
	createdIds.numIdsPtr			= &gCreatedIdsBufferNum;
	createdIds.bufferId				= gCreatedIdsBuffer;
	createdIds.bufferIdByteStride	= sizeof(unsigned);

	//Create a fluid descriptor
	NxFluidDesc fluidDesc;
    fluidDesc.maxParticles                  = particles.maxParticles;
    fluidDesc.kernelRadiusMultiplier		= KERNEL_RADIUS_MULTIPLIER;
    fluidDesc.restParticlesPerMeter			= REST_PARTICLES_PER_METER;
	fluidDesc.motionLimitMultiplier			= MOTION_LIMIT_MULTIPLIER;
	fluidDesc.packetSizeMultiplier			= PACKET_SIZE_MULTIPLIER;
    fluidDesc.collisionDistanceMultiplier   = COLLISION_DISTANCE_MULTIPLIER ;
    fluidDesc.stiffness						= 50.0f;
    fluidDesc.viscosity						= 22.0f;
	fluidDesc.restDensity					= 1000.0f;
    fluidDesc.damping						= 0.0f;
    fluidDesc.staticCollisionRestitution	= 0.4f;
	fluidDesc.staticCollisionAdhesion		= 0.03f;
	fluidDesc.simulationMethod				= NX_F_SPH; //NX_F_NO_PARTICLE_INTERACTION;

	fluidDesc.initialParticleData			= particles;
	fluidDesc.particlesWriteData			= particles;
	fluidDesc.particleCreationIdWriteData   = createdIds;
	
	if (!gUseHW)
    {
        fluidDesc.flags &= ~NX_FF_HARDWARE;
        fluidDesc.flags |= (1 << 20);
    }

	if(fluidDesc.maxParticles<gParticleBufferNum)
		gParticleBufferNum = fluidDesc.maxParticles;

    gFluid =	gScene->createFluid(fluidDesc);
	
	assert(gFluid != NULL);

    // Create the fluid surface
    NxImplicitScreenMeshDesc meshDesc, particleMeshDesc;
    meshDesc.setToDefault();
    meshDesc.width = gWidth;
    meshDesc.height = gHeight;

	glMatrixMode(GL_PROJECTION);
	gluPerspective(60.0f, gRatio, 1.0f, 10000.0f);
	gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);
	glMatrixMode(GL_MODELVIEW);
    glGetFloatv(GL_PROJECTION_MATRIX, gProjMatrix);
    meshDesc.projectionMatrix = gProjMatrix;
/*
    meshDesc.meshData.verticesPosBegin = gVertices;
    meshDesc.meshData.verticesNormalBegin = gNormals;
    meshDesc.meshData.verticesPosByteStride = sizeof(float)*3;
    meshDesc.meshData.verticesNormalByteStride = sizeof(float)*3;
*/
	meshDesc.meshData.verticesPosBegin = &(gFullVerts[0].v3Position);
	meshDesc.meshData.verticesNormalBegin = &(gFullVerts[0].v3Normal);
	meshDesc.meshData.verticesPosByteStride = sizeof(SSimpleVertex);
	meshDesc.meshData.verticesNormalByteStride = sizeof(SSimpleVertex);

    meshDesc.meshData.maxVertices = NUM_VERTS;
    meshDesc.meshData.numVerticesPtr = &gNumVertices;
    meshDesc.meshData.indicesBegin = gIndices;
	meshDesc.meshData.flags |= NX_MDF_16_BIT_INDICES;
    meshDesc.meshData.indicesByteStride = sizeof(NxU16);
    meshDesc.meshData.maxIndices = NUM_VERTS;
    meshDesc.meshData.numIndicesPtr = &gNumIndices;

    meshDesc.screenSpacing = 5;
    meshDesc.triangleWindingPositive = 1;
    meshDesc.threshold = .25;
    meshDesc.particleRadius = .1;

    assert(meshDesc.meshData.isValid());
    assert(meshDesc.isValid());

    gFluidMesh = gFluid->createScreenSurfaceMesh(meshDesc);
	printf("\nFluid Partile Radius = %f",gFluidMesh->getParticleRadius());


    particleMeshDesc = meshDesc;
    particleMeshDesc.meshData.verticesPosBegin = gParticleVertices;
    particleMeshDesc.meshData.verticesNormalBegin = gParticleNormals;
    particleMeshDesc.meshData.numVerticesPtr = &gParticleNumVertices;
    particleMeshDesc.meshData.indicesBegin = gParticleIndices;
	particleMeshDesc.meshData.flags |= NX_MDF_16_BIT_INDICES; 
    particleMeshDesc.meshData.numIndicesPtr = &gParticleNumIndices;
	particleMeshDesc.meshData.verticesPosByteStride = sizeof(float)*3;
	particleMeshDesc.meshData.verticesNormalByteStride = sizeof(float)*3;

    gParticleMesh = gScene->createImplicitScreenMesh(particleMeshDesc);
    printf("\nUser Defined Particle Radius = %f",gParticleMesh->getParticleRadius());

    gParticleMesh->setParticles(particles);
}

static void CreateDynamicActors() {

	if(gScene && gSceneRunning)
	{
		NxU32 error;
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true, &error);
		gSceneRunning = false;
		assert(error==0);
	}

	// Add a dynamic sphere.
	NxActorDesc sphereActor;
	NxBodyDesc sphereBody;
	NxSphereShapeDesc sphereShape;
	sphereShape.radius = 0.4f;
	sphereActor.shapes.pushBack(&sphereShape);
	sphereActor.body = &sphereBody;
	sphereActor.density = 1.0f;
	sphereActor.globalPose.t.set(0.0f, 4.0f, 1.0f);
	gScene->createActor(sphereActor);

	// Add a dynamic capsule.
	NxActorDesc capsActor;
	NxCapsuleShapeDesc capsShape;
	NxBodyDesc capsBody;
	capsActor.shapes.pushBack(&capsShape);
	capsActor.body = &capsBody;
	capsActor.density = 1.0f;
	capsActor.globalPose.t.set(3.0f, 0.0f, 8.0f);
	gScene->createActor(capsActor);

	// Add a dynamic box.
	NxActorDesc boxActor;
	NxBoxShapeDesc boxShape;
	NxBodyDesc boxBody;
	boxShape.dimensions.set(0.75f, 0.75f, 0.75f);
	boxActor.shapes.pushBack(&boxShape);
	boxActor.body = &boxBody;
	boxActor.density = 1.0f;
	boxActor.globalPose.t.set(2.0f, 5.0f, 7.0f);
	gScene->createActor(boxActor);
}

NxActor* CreateFlatSurface(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride, NxTriangleMeshDesc& tmd)
{
	NxVec3* fsVerts = NULL;
    NxU32* fsFaces = NULL;
    NxVec3* fsNormals = NULL;

	// Initialize flat surface verts
    NxU32 nbVerts = (length+1)*(width+1);
	fsVerts = new NxVec3[nbVerts];

	// Build flat surface
	for (NxU32 i = 0; i < width + 1; i++)
	{
		for (NxU32 j = 0; j < length + 1; j++)
		{
			fsVerts[i+j*(width+1)] = NxVec3(stride*NxReal(i),0,stride*NxReal(j)) - NxVec3(0.5*stride*NxReal(width),0,0.5*stride*NxReal(length)); 
		}
	}

	// Initialize flat surface faces
	NxU32 nbFaces = length*width*2;
	fsFaces = new NxU32[nbFaces*3];

	NxU32 k = 0;
	for (NxU32 i = 0; i < width; i++)
	{
		for (NxU32 j = 0; j < length; j++)
		{
			// Create first triangle
			fsFaces[k] = i + j*(width+1);
			fsFaces[k+1] = i + (j+1)*(width+1);
			fsFaces[k+2] = i+1 + (j+1)*(width+1);
			k+=3;

			// Create second triangle
			fsFaces[k] = i + j*(width+1);
			fsFaces[k+1] = i+1 + (j+1)*(width+1);
			fsFaces[k+2] = i+1 + j*(width+1);
			k+=3;
		}
	}

	// allocate flat surface materials -- one for each face
//	fsMaterials = new NxMaterialIndex[nbFaces];

	// Build vertex normals
	fsNormals = new NxVec3[nbFaces];
//	NxBuildSmoothNormals(nbFaces, nbVerts, fsVerts, fsFaces, NULL, fsNormals, true);

	NxTriangleMeshDesc fsDesc;
	fsDesc.numVertices				= nbVerts;
	fsDesc.numTriangles				= nbFaces;
	fsDesc.pointStrideBytes			= sizeof(NxVec3);
	fsDesc.triangleStrideBytes		= 3*sizeof(NxU32);
	fsDesc.points					= fsVerts;
	fsDesc.triangles				= fsFaces;							
	fsDesc.flags					= 0;

	// Add the mesh material data
//	fsDesc.materialIndexStride			= sizeof(NxMaterialIndex);
//	fsDesc.materialIndices				= fsMaterials;

	NxTriangleMeshShapeDesc fsShapeDesc;

    if (0)
    {
        // Cooking from file
        bool status = NxCookTriangleMesh(fsDesc, UserStream("c:\\tmp.bin", false));
        fsShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookTriangleMesh(fsDesc, buf);
        fsShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
    }

	if (fsShapeDesc.meshData)
    {
	    NxActorDesc actorDesc;
	    actorDesc.shapes.pushBack(&fsShapeDesc);
	    actorDesc.globalPose.t = pos;
	    NxActor* actor = gScene->createActor(actorDesc);

	    // Attach drawable mesh to shape's user data
	    NxShape*const* shapes = actor->getShapes();
        shapes[0]->isTriangleMesh()->getTriangleMesh().saveToDesc(tmd);
	    shapes[0]->userData = (void*)&tmd;

	    return actor;
//      gPhysicsSDK->releaseTriangleMesh(*fsShapeDesc.meshData);
    }

	return NULL;
}

static void InitNx()
{
	// Initialize PhysicsSDK
//	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, &gErrorStream);
	gPhysicsSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost");

    NxInitCooking(NULL, &gErrorStream);

	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 10.0f);
	//gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_POSITION, 1);
	//gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_VELOCITY, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_PACKETS, 1);
	//gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_MESH_PACKETS, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_DRAINS, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);

	if(!gPhysicsSDK)	return;

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity				= gDefaultGravity;
	sceneDesc.simType				= NX_SIMULATION_SW;
	//You can also use a hardware scene, in which case you need to 
	//map the triangle meshes to the PhysX card. This is done below, 
	//after creating the meshes.
	//sceneDesc.simType				= NX_SIMULATION_HW;

	assert(sceneDesc.isValid());
	gScene = gPhysicsSDK->createScene(sceneDesc);
	assert(gScene != NULL);

	//flatSurface = CreateFlatSurface(NxVec3(0,0,0), 6, 6, 2, flatSurfaceTriangleMeshDesc);

#if 0
	// Create small mesh
	NxArray<NxVec3> vertices;
	NxArray<NxU32> faces;

	vertices.pushBack().set(0,-0.7,0);
	vertices.pushBack().set(-2,0,-2);
	vertices.pushBack().set(-2,0, 2);
	vertices.pushBack().set( 2,0, 2);
	vertices.pushBack().set( 2,0,-2);

	faces.pushBack() = 0;
	faces.pushBack() = 1;
	faces.pushBack() = 2;
	
	faces.pushBack() = 0;
	faces.pushBack() = 2;
	faces.pushBack() = 3;
	
	faces.pushBack() = 0;
	faces.pushBack() = 3;
	faces.pushBack() = 4;

	faces.pushBack() = 0;
	faces.pushBack() = 4;
	faces.pushBack() = 1;

	// Build physical model
	NxTriangleMeshDesc terrainDesc;
	terrainDesc.numVertices					= vertices.size();
	terrainDesc.numTriangles				= faces.size()/3;
	terrainDesc.pointStrideBytes			= sizeof(NxVec3);
	terrainDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	terrainDesc.points						= &vertices[0].x;
	terrainDesc.triangles					= &faces[0];
	terrainDesc.flags						= NX_MF_HARDWARE_MESH;

	//terrainDesc.heightFieldVerticalAxis		= NX_Y;
	//terrainDesc.heightFieldVerticalExtent	= -1000.0f;
	MemoryWriteBuffer buf;
	bool status = NxCookTriangleMesh(terrainDesc, buf);

	NxTriangleMesh* terrainMesh				= gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));

	NxTriangleMeshShapeDesc terrainMeshDesc;
	terrainMeshDesc.meshData = terrainMesh;

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&terrainMeshDesc);
	gScene->createActor(ActorDesc);
	
#else
	// Load ASE file
    int set = 0;

#ifndef LINUX
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#else
	set = chdir(&fname[0]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	CookASE("fluidSample.ase", gScene, NxVec3(1,0,0));
	CookASE("coolFlow.ase", gScene, NxVec3(1,-4,-0), NxVec3(1,0.2,1));
#endif
	//If we are running a HW scene, then we need to map the mesh pages to the PhysX card
	//(otherwise there will not be interactions with dynamic objects).
	if (gScene && gScene->getSimType() == NX_SIMULATION_HW) 
	{
		for (NxU32 a = 0; a < gScene->getNbActors(); a++) 
		{
			NxActor* actor = gScene->getActors()[a];
			for (NxU32 i = 0; i < actor->getNbShapes(); ++ i) 
			{
				NxTriangleMeshShape* shape = actor->getShapes()[i]->isTriangleMesh();
				if (shape)
				{
					NxTriangleMesh& mesh = shape->getTriangleMesh();
					for (NxU32 j = 0; j < mesh.getPageCount(); j++)
						shape->mapPageInstance(j);
				}
			}
		}
	}

	// Add a box shaped drain.
	NxActorDesc boxDrainActor;
	NxBoxShapeDesc boxDrainShape;
	boxDrainActor.shapes.pushBack(&boxDrainShape);
	boxDrainShape.dimensions.set(40,1,40);
	boxDrainShape.shapeFlags |= NX_SF_FLUID_DRAIN;
	boxDrainActor.globalPose.t.set(0, -10, 0);
	gScene->createActor(boxDrainActor);

	//CreateDynamicActors();

	//Pre cook hotspots
	NxBounds3 precookAABB;
	precookAABB.set(NxVec3(-20,-20,-20), NxVec3(20,20,20));
	gScene->cookFluidMeshHotspot(precookAABB, PACKET_SIZE_MULTIPLIER, REST_PARTICLES_PER_METER, KERNEL_RADIUS_MULTIPLIER, MOTION_LIMIT_MULTIPLIER, COLLISION_DISTANCE_MULTIPLIER );

	CreateFluid();
 	if (gScene && !gSceneRunning) 
	{
 		gScene->simulate(1.0f/60.0f);
		gSceneRunning = true;
 	}
}

#define MANIPULATE_FLUID_RESTART   1
#define MANIPULATE_FLUID_FREEZE    2
#define MANIPULATE_FLUID_SIM       3
#define MANIPULATE_FLUID_COLL_DYN  5
#define MANIPULATE_FLUID_COLL_STC  6
#define MANIPULATE_FLUID_REMOVE    7

static void ManipulateFluid(unsigned mode)
{
	if (!gFluid) return;
	if (gSceneRunning) {
		NxU32 error;
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true, &error);
		gSceneRunning = false;
		assert(error == 0);
	}

	switch (mode)
	{
	case MANIPULATE_FLUID_RESTART:
		{
			if (gFluidMesh)
				gFluid->releaseScreenSurfaceMesh(*gFluidMesh);
			if (gParticleMesh)
				gScene->releaseImplicitScreenMesh(*gParticleMesh);
			gScene->releaseFluid(*gFluid);
			gFluid = 0;
			CreateFluid();
			break;
		}
	case MANIPULATE_FLUID_FREEZE:
		{
			bool enabled = gFluid->getFlag(NX_FF_ENABLED)!=0;
			gFluid->setFlag(NX_FF_ENABLED, !enabled);
			break;
		}
	case MANIPULATE_FLUID_SIM:
		{
			NxU32 method = gFluid->getSimulationMethod();
			switch (method)
			{
			case NX_F_SPH:
				method = NX_F_NO_PARTICLE_INTERACTION;
				break;
			case NX_F_NO_PARTICLE_INTERACTION:
				method = NX_F_SPH;
				break;
			}
			gFluid->setSimulationMethod(method);
			break;
		}
	case MANIPULATE_FLUID_COLL_DYN:
		{
			NxU32 method = gFluid->getCollisionMethod();
			if (method & NX_F_DYNAMIC)	method &= ~NX_F_DYNAMIC;
			else						method |= NX_F_DYNAMIC;
			gFluid->setCollisionMethod(method);
			break;
		}
	case MANIPULATE_FLUID_COLL_STC: 
		{
			NxU32 method = gFluid->getCollisionMethod();
			if (method & NX_F_STATIC)	method &= ~NX_F_STATIC;
			else						method |= NX_F_STATIC;
			gFluid->setCollisionMethod(method);
			break;
		}

	case MANIPULATE_FLUID_REMOVE:
		{
			gFluid->removeAllParticles();
		}
	}

	gScene->simulate(1.0f/60.0f);
	gSceneRunning = true;
}

// Render code

static void KeyboardCallback(unsigned char key, int x, int y, bool isDown)
{
	switch (key)
	{
	case 27:	exit(0); break;
	case '1':			
		gDrawSurfaceFW ^= (int)isDown; 
		if(gDrawSurfaceFW)
		{        
			gFluidMesh->setWindowWidth(gWidth);
			gFluidMesh->setWindowHeight(gHeight);
		}
		break;
	case '3':			
		gDrawSurfaceSW ^= (int)isDown; 
		if(gDrawSurfaceSW)
		{
			gParticleMesh->setWindowWidth(gWidth);
			gParticleMesh->setWindowHeight(gHeight);
		}
		break;
	case 'p':			gPause ^= isDown; break;
	case 'o':			gSingleStep ^= isDown; gPause = !gSingleStep; break;
	case 'g':			gDebugRender ^= isDown; break;
	case 'f':			if (isDown) ManipulateFluid(MANIPULATE_FLUID_FREEZE); break; 
	case ' ':			if (isDown) gPoints = (gPoints+1)%3; break;
	case 'z':			gDrawScene ^= isDown; break;
	case 'x':		    if (isDown) gDrawSurface = (gDrawSurface+1)%4; break;
	case 'c':			gUpdateCamera^= isDown; break;
	case 'h':			gUseHW ^= isDown; // no break, restart fluids on down.
	case 'r':			if (isDown) { gUpdateCamera = true; ManipulateFluid(MANIPULATE_FLUID_RESTART); } break;
	case 'j':			if (isDown) ManipulateFluid(MANIPULATE_FLUID_REMOVE); break;
	case 'k':			if (isDown) ManipulateFluid(MANIPULATE_FLUID_SIM); break;
	case 'n':			if (isDown) ManipulateFluid(MANIPULATE_FLUID_COLL_DYN); break;
	case 'm':			if (isDown) ManipulateFluid(MANIPULATE_FLUID_COLL_STC); break;
	case 'R':			if (isDown) CreateDynamicActors(); break;
	case 'w': case '8':	up = isDown; break;
	case 's': case '2':	down = isDown; break;
	case 'a': case '4':	left = isDown; break;
	case 'd': case '6':	right = isDown; break;
	case 'b':       	if (isDown) Dir *= -1; break;
	}

}
static void KeyboardDownCallback(unsigned char key, int x, int y)
	{
		KeyboardCallback(key, x, y, true);
	}
static void KeyboardUpCallback(unsigned char key, int x, int y)
	{
		KeyboardCallback(key, x, y, false);
	}

static void ArrowKeyCallback(int key, int x, int y, bool value)
	{
		switch(key) {
			case 101: up = value; break;
			case 103: down = value; break;
			case 100: left = value; break;
			case 102: right = value; break;
		}
	}
static void ArrowDownKeyCallback(int key, int x, int y)
	{
		ArrowKeyCallback(key, x, y, true);
	}
static void ArrowUpKeyCallback(int key, int x, int y)
	{
		ArrowKeyCallback(key, x, y, false);
	}


static void MouseCallback(int button, int state, int x, int y)
{
	mx = x;
	my = y;
}

#include <stdio.h>
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

void RenderSurface(float* vertices, float* normals, NxU16* indices, int numVerts, int numIndices)
{
    if ((gDrawSurface < 2) && numIndices)
    {
        if (gDrawSurface == 0)
    	    glPolygonMode(GL_FRONT, GL_FILL);
        else
    	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	    glPointSize(2);

    	glBegin(GL_TRIANGLES);
        for (int i = 0; i < numIndices; i+=3)  // +3 for 3 indices per triangle
        {
		    int triId0 = indices[i+0]*3;       // *3 for 3 floats per vertex
		    int triId1 = indices[i+1]*3;
		    int triId2 = indices[i+2]*3;


            glNormal3f(normals[triId0+0], 
			           normals[triId0+1], 
				       normals[triId0+2]);

		    glVertex3f(vertices[triId0+0], 
			           vertices[triId0+1], 
				       vertices[triId0+2]);

            glNormal3f(normals[triId1+0], 
			           normals[triId1+1], 
				       normals[triId1+2]);

		    glVertex3f(vertices[triId1+0], 
			           vertices[triId1+1], 
				       vertices[triId1+2]);

            glNormal3f(normals[triId2+0], 
			           normals[triId2+1], 
				       normals[triId2+2]);

            glVertex3f(vertices[triId2+0], 
			           vertices[triId2+1], 
				       vertices[triId2+2]);

        }
    	glEnd();

    	glColor3f(1,1,1);
	    glPolygonMode(GL_FRONT, GL_FILL);
    }
    else if (gDrawSurface == 2)
    {
	    glPointSize(3);

    	glBegin(GL_POINTS);
        for (int i = 0; i < numVerts; i++)
        {
		    int triId0 = i*3;

		    glVertex3f(vertices[triId0], 
			           vertices[triId0+1], 
				       vertices[triId0+2]);
        }
    	glEnd();

    	glColor3f(1,1,1);
    }

}

/* Modified for user vertex normal data structure */
void RenderSurfaceWithInterleavedDataStructure(SSimpleVertex * fullVerts, NxU16* indices, int numIndices)
{
    if ((gDrawSurface < 2) && numIndices )
    {
        if (gDrawSurface == 0)
    	    glPolygonMode(GL_FRONT, GL_FILL);
        else
    	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// render it with Vertex_Array
		//glEnableClientState(GL_INDEX_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
    
		glVertexPointer(3, GL_FLOAT, sizeof(SSimpleVertex), &(fullVerts[0].v3Position));
		glNormalPointer(GL_FLOAT, sizeof(SSimpleVertex), &(fullVerts[0].v3Normal));
		//glIndexPointer(GL_INT, 0, indices);

		//glDrawArrays(GL_TRIANGLES, 0, numIndices);

		// default
		//glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indices);
		
		// if here we use flag |= NX_MDF_16_BIT_INDICES
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		//glDisableClientState(GL_INDEX_ARRAY);
/*    
		// the following also works
		float v0[3],n0[3];
		float v1[3],n1[3];
		float v2[3],n2[3];
		glBegin(GL_TRIANGLES);
        for (int i = 0; i < numIndices; i+=3)  // +3 for 3 indices per triangle
        {
		    int triId0 = indices[i+0];       // *3 for 3 floats per vertex
		    int triId1 = indices[i+1];
		    int triId2 = indices[i+2];
			
			v0[0] = gFullVerts[triId0].v3Position.x;
			v0[1] = gFullVerts[triId0].v3Position.y;
			v0[2] = gFullVerts[triId0].v3Position.z;

			v1[0] = gFullVerts[triId1].v3Position.x;
			v1[1] = gFullVerts[triId1].v3Position.y;
			v1[2] = gFullVerts[triId1].v3Position.z;

			v2[0] = gFullVerts[triId2].v3Position.x;
			v2[1] = gFullVerts[triId2].v3Position.y;
			v2[2] = gFullVerts[triId2].v3Position.z;

			n0[0] = gFullVerts[triId0].v3Normal.x;
			n0[1] = gFullVerts[triId0].v3Normal.y;
			n0[2] = gFullVerts[triId0].v3Normal.z;

			n1[0] = gFullVerts[triId1].v3Normal.x;
			n1[1] = gFullVerts[triId1].v3Normal.y;
			n1[2] = gFullVerts[triId1].v3Normal.z;

			n2[0] = gFullVerts[triId2].v3Normal.x;
			n2[1] = gFullVerts[triId2].v3Normal.y;
			n2[2] = gFullVerts[triId2].v3Normal.z;
			glNormal3fv(n0);
		    glVertex3fv(v0);
            glNormal3fv(n1);
		    glVertex3fv(v1);
            glNormal3fv(n2);
		    glVertex3fv(v2);
        }
    	glEnd();
*/
	}
}
static void RenderCallback()
{
    if(gScene && gUpdateCamera && gFluidMesh)
    {
        NxReal projMatrix[16];
        glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);

	    // Setup new camera
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    gluPerspective(60.0f, gRatio, 1.0f, 10000.0f);
	    gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);

        // set new camera to surfaces for next frame
        glGetFloatv(GL_PROJECTION_MATRIX, gProjMatrix);
        if (gFluidMesh)
            gFluidMesh->setProjectionMatrix(gProjMatrix);
        if (gParticleMesh)
            gParticleMesh->setProjectionMatrix(gProjMatrix);

    	glMatrixMode(GL_MODELVIEW);
    }
    else
    {
	    // Setup new camera
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    gluPerspective(60.0f, gRatio, 1.0f, 10000.0f);
	    gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);

        glGetFloatv(GL_PROJECTION_MATRIX, gProjMatrix);
    	glMatrixMode(GL_MODELVIEW);
    }

    if (gFluidMesh)
        gFluidMesh->generateMesh();
    if (gParticleMesh)
        gParticleMesh->generateMesh();

    
    // Physics code
    if(gScene && gSceneRunning)
    {
		NxU32 error;
        gScene->flushStream();
        gScene->fetchResults(NX_RIGID_BODY_FINISHED, true, &error);
		gSceneRunning = false;
		assert(error==0);
    }


	// ~Physics code
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(1,1,1);
    if (gDrawScene)
    {
	    for (NxU32 i = 0; i < gScene->getNbActors(); i++) {
		    NxActor* a = gScene->getActors()[i];
		    DrawActor(a);
		    //printf("d %d\n", i);
	    }

	    if (gDebugRender) {
		    const NxDebugRenderable* r = gScene->getDebugRenderable();
		    if (r != NULL)
			    gDebugRenderer.renderData(*r);
	    }
    }
    
    // Start next physics step.
	if(gScene && !gPause && !gSceneRunning) {
		gScene->simulate(1.0f/60.0f);
		gSceneRunning = true;
        if (gSingleStep)
        {
            gSingleStep = false;
            gPause = true;
        }
 	}

	glColor3f(1,1,1);
	glLoadIdentity();


	glEnable(GL_LIGHTING);
//	    glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT0);

    if (gFluidMesh && gDrawSurfaceFW)
    {
        gFluidMesh->getMesh();
        glColor3f(0,0,1);
        //RenderSurface(gVertices, gNormals, gIndices, gNumVertices, gNumIndices);
		RenderSurfaceWithInterleavedDataStructure(gFullVerts, gIndices, gNumIndices);

    }

    if (gParticleMesh && gDrawSurfaceSW)
    {
        gParticleMesh->getMesh();
        glColor3f(0,1,0);
        RenderSurface(gParticleVertices, gParticleNormals, gParticleIndices, gParticleNumVertices, gParticleNumIndices);
    }

	// Manage user per particle data
	for (unsigned p=0; p<gParticleBufferNum; p++)
	{
		//create user data to particle data reference...
		NxU32 id = gParticleBuffer[p].id;
		gParticleUserData[id].particleIndex = p;
	}
	NxReal max = 0.0f;
	NxReal min = 10000.0f;
	for (unsigned p=0; p<gCreatedIdsBufferNum; p++)
	{
	
		NxU32 id = gCreatedIdsBuffer[p];
		NxU32 index = gParticleUserData[id].particleIndex;
		NxReal d = gParticleBuffer[index].density;
		d = NxMath::clamp((d - 625)/(990-625), 1.0f, 0.0f);

		gParticleUserData[id].color.set(d*d,0,d);

		if (d > max) max = d;
		if (d < min) min = d;
	}

	// Keep physics & graphics in sync
	if (gPoints == 0) {
		glPointSize(3);
		glBegin(GL_POINTS);
		for (unsigned p=0; p<gParticleBufferNum; p++)
		{
			Particle& particle = gParticleBuffer[p];
			NxVec3& color = gParticleUserData[particle.id].color;
			//glColor3f(1,0,0);
			glColor3fv(&color.x);
			glVertex3fv(&particle.pos.x);
		}
		glEnd();
		glColor3f(1,1,1);
	} else if (gPoints == 1) {
		for (unsigned p=0; p<gParticleBufferNum; p++)
		{
			Particle& particle = gParticleBuffer[p];
			NxVec3& color = gParticleUserData[particle.id].color;
			//glColor3f(1,0,0);
			glColor3fv(&color.x);
			glPushMatrix();
			glTranslatef(particle.pos.x,particle.pos.y,particle.pos.z);
//			glutSolidSphere(0.1,3,3);//8,5);
			glutSolidCube(0.1);
			glPopMatrix();
		}
	}


	glFlush();
	glutSwapBuffers();
}

static void ReshapeCallback(int width, int height)
{
	glViewport(0, 0, width, height);
	gRatio = float(width)/float(height);
    if((gScene && gUpdateCamera && gFluidMesh) &&
      ((gWidth != width) || (gHeight != height)))
    {
        gFluidMesh->setWindowWidth(width);
        gFluidMesh->setWindowHeight(height);
    }
    if((gParticleMesh) &&
      ((gWidth != width) || (gHeight != height)))
    {
        gParticleMesh->setWindowWidth(width);
        gParticleMesh->setWindowHeight(height);
    }

    gWidth = width;
    gHeight = height;
}

static void IdleCallback()
{
	NxReal velocity = 0.1f;
	if (up)
		Eye += Dir * velocity;

	if (down)
		Eye -= Dir * velocity;

	if (left)
		Eye -= N * velocity;
	
	if (right)
		Eye += N * velocity;
	
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
  gWidth = 512;
  gHeight = 512;
  SetCWDToEXE();
  
	// Initialize Glut
	printf("\n"
		"Space toggles fluid particle rendering\n"
		"[z] toggles object rendering\n"
		"[x] toggles fluid surface rendering style\n"
		"[1] toggles Firmware fluid surface rendering\n"
		"[3] toggles Software fluid surface rendering\n"
		"[c] toggles camera updating\n"
		"[r] recreates the fluid\n"
		"[R] adds dynamic RBs\n"
		"[g] to toggle debug rendering\n"
		"[h] to toggle hardware simulation (and restart)\n"
		"[b] to look backwards\n"
		"[p] to pause\n"
		"[o] to single step\n"
		"[f] toggles fluid simulation\n"
		);
	printf("Use the arrow keys or a, d, w and s or 2, 4, 6 and 8 to move the camera.\n"
		"Use the mouse to look around.\n");

    glutInit(&argc, argv);
	glutInitWindowSize(gWidth, gHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("SampleHardwareFluid");
	glutSetWindow(gMainHandle);
	glutDisplayFunc(RenderCallback);
	glutReshapeFunc(ReshapeCallback);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(KeyboardDownCallback);
	glutKeyboardUpFunc(KeyboardUpCallback);
	glutSpecialFunc(ArrowDownKeyCallback);
	glutSpecialUpFunc(ArrowUpKeyCallback);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
	MotionCallback(0,0);

	// Setup default render states
	glClearColor(0.3f, 0.4f, 0.5f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

	// Setup lighting
	glEnable(GL_LIGHTING);
	float AmbientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };		glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	float DiffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	float SpecularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[]		= { -100.0f, 100.0f, 400.0f, 1.0f };		glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	Eye = NxVec3(9.5, 3.8, 12.3);

	// Physics code
	InitNx();
	// ~Physics code

	// Run
	glutMainLoop();
	return 0;
}
