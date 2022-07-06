// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			      LESSON 119: DYNAMIC CONTINUOUS COLLISION DETECTION
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson119.h"

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
char* gOnOffString[2] = {"Off","On"};

// Force globals
extern NxVec3 gForceVec;
extern NxReal gForceStrength;
extern bool bForceMode;

// Simulation globals
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;

extern NxActor* gSelectedActor;

// CCD globals
bool bCCDEnabled = true;

bool bToggleCCDEnabled = false;
bool bRefreshScene = false;
bool bFireCCDBox = false;

extern NxVec3 gCameraPos;
extern NxVec3 gCameraForward;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------\n v = Enable/disable CCD\n t = Reset scene\n <space> = Fire a small, fast box into the scene\n");
	printf(" 1 = Create Stack(10)\n 2 = Create Stack(15)\n 3 = Create Tower(10)\n");
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;
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

	if (bToggleCCDEnabled)
	{
        bToggleCCDEnabled = false;
        bCCDEnabled = !bCCDEnabled;
        gPhysicsSDK->setParameter(NX_CONTINUOUS_CD, bCCDEnabled);
		// Set dynamic ccd on/off in HUD
		char ds[512];
		sprintf(ds, "Dynamic CCD: %s", gOnOffString[bCCDEnabled]);
		hud.SetDisplayString(2, ds, 0.015f, 0.92f);	
	}

	if (bRefreshScene)
	{
        bRefreshScene = false;
        NxActor** actors = gScene->getActors();
        while (gScene->getNbActors() > 1)
		{
            gScene->releaseActor(*actors[gScene->getNbActors()-1]);
        }
	}

	if (bFireCCDBox)
	{
        bFireCCDBox = false;
        NxActor* ccdBox = CreateCCDBox(gCameraPos, NxVec3(0.1,0.1,0.1), 10, true);
        NxVec3 vel = gCameraForward;
        vel.normalize();
        vel *= 344;
        ccdBox->setLinearVelocity(vel);
	}

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
		if (gScene)  gDebugRenderer.renderData(*gScene->getDebugRenderable());
	}
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
 	RenderActors(bShadows);

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void SpecialKeys(unsigned char key, int x, int y)
{
	switch(key)
	{
	    case '1':	CreateStack(10); break;
	    case '2':	CreateStack(15); break;
	    case '3':	CreateTower(10); break;
		case 't':
		{
			bRefreshScene = true;
		}
		break;
	    case 'v':
		{
			bToggleCCDEnabled = true;
		}
		break;	    
		case ' ':
		{
			bFireCCDBox = true;
		}
		break;
	}
}

void CreateStack(int size)
{
	const float cubeSize = 0.2f;
	const float spacing = 0;
	NxVec3 pos(0,0,0);
	float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;
	while(size)
	{
		for(int i=0;i<size;i++)
		{
			pos.x = offset + (float)i * (cubeSize * 2.0f + spacing);
			CreateCCDBox(pos, NxVec3(cubeSize, cubeSize, cubeSize), 10, false);
		}

		offset += cubeSize;
		pos.y += (cubeSize * 2.0f + spacing);
		size--;
	}
}

void CreateTower(int size)
{
	const float cubeSize = 0.2f;
	const float spacing = 0.001f;
	NxVec3 pos(0,0,0);
	while(size)
	{
		CreateCCDBox(pos, NxVec3(cubeSize, cubeSize, cubeSize), 10, false);
		pos.y += (cubeSize * 2.0f + spacing);
		size--;
	}
}

NxCCDSkeleton* CreateCCDSkeleton(float size)
{
	NxU32 triangles[3 * 12] = { 
		0,1,3,
		0,3,2,
		3,7,6,
		3,6,2,
		1,5,7,
		1,7,3,
		4,6,7,
		4,7,5,
		1,0,4,
		5,1,4,
		4,0,2,
		4,2,6
	};

	NxVec3 points[8];

	// Static mesh
	points[0].set( size, -size, -size);
	points[1].set( size, -size,  size);
	points[2].set( size,  size, -size);
	points[3].set( size,  size,  size);

	points[4].set(-size, -size, -size);
	points[5].set(-size, -size,  size);
	points[6].set(-size,  size, -size);
	points[7].set(-size,  size,  size);

	NxSimpleTriangleMesh stm;
	stm.numVertices = 8;
	stm.numTriangles = 6*2;
	stm.pointStrideBytes = sizeof(NxVec3);
	stm.triangleStrideBytes = sizeof(NxU32)*3;

	stm.points = points;
	stm.triangles = triangles;
	stm.flags |= NX_MF_FLIPNORMALS;
	return gPhysicsSDK->createCCDSkeleton(stm);
}

NxActor* CreateCCDBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density, bool doDynamicCCD)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(boxDim.x,boxDim.y,boxDim.z);
	boxDesc.localPose.t = NxVec3(0,boxDim.y,0);

	// Set box up with CCD skeleton
	boxDesc.ccdSkeleton = CreateCCDSkeleton(boxDim.x*0.8f);
	if (doDynamicCCD)  boxDesc.shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD;  // Activate dynamic-dynamic CCD for this body

	actorDesc.shapes.pushBack(&boxDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

void InitializeSpecialHUD()
{
	char ds[512];

	// Add dynamic ccd on/off to HUD
	sprintf(ds, "Dynamic CCD: %s", gOnOffString[bCCDEnabled]);
	hud.AddDisplayString(ds, 0.015f, 0.92f);
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
	if (!gPhysicsSDK)  return;

	// Set the physics parameters
//	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

	// Turn on continous collision detection for dynamic objects
	gPhysicsSDK->setParameter(NX_CONTINUOUS_CD, bCCDEnabled);
	gPhysicsSDK->setParameter(NX_CCD_EPSILON, 0.01);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity				= gDefaultGravity;
    sceneDesc.simType				= NX_SIMULATION_HW;
    gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}

	// Create the default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.5);
	defaultMaterial->setStaticFriction(0.5);
	defaultMaterial->setDynamicFriction(0.5);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	// Initialize actor user data
	AddUserDataToActors(gScene);

	// Initialize HUD
	InitializeHUD();
	InitializeSpecialHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 119: Dynamic Continuous Collision Detection");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}



