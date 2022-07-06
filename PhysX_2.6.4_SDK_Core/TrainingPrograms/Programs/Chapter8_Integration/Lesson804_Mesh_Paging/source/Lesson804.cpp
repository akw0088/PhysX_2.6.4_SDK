// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							   LESSON 804: MESH PAGING
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson804.h"

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

// Camera globals
extern NxVec3 gCameraPos;
extern NxReal gCameraSpeed;

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
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;
extern NxActor* concaveObject;
extern NxActor* heightfield;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n");
}

void AddTerrainLight()
{
	float AmbientColor1[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientColor1);
    float DiffuseColor1[]    = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseColor1);
    float SpecularColor1[]   = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularColor1);
    float Position1[]        = { -25.0f, 55.0f, 15.0f, 1.0f };	   glLightfv(GL_LIGHT1, GL_POSITION, Position1);
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
 	RenderActors(false);

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
}

void BuildTerrainComponent(const NxVec3& pos)
{
	heightfield = CreateHeightfield(pos, 2, 200);

	NxShape*const* shapes = heightfield->getShapes();
	for (unsigned int s = 0; s < heightfield->getNbShapes(); ++s )
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

	box = CreateBox(pos + gCameraPos + NxVec3(5,0,10), 4*NxVec3(0.5,1,0.5), 1);
	sphere = CreateSphere(pos + gCameraPos + NxVec3(0,0,15), 4*1, 1);
	capsule = CreateCapsule(pos + gCameraPos + NxVec3(-5,0,10), 4*2, 4*0.5, 1);
//	pyramid = CreatePyramid(pos + gCameraPos + NxVec3(0,0,10), NxVec3(1,0.5,1.5), 10);
//	concaveObject = CreateConcaveObject(pos + gCameraPos + NxVec3(-4,0,15), NxVec3(1,1,1), 5);
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator, &gErrorStream);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity;
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

	NxVec3 A = NxVec3(1,0,0);
	NxVec3 B = NxVec3(0,0,1);
	NxVec3 C;

	C = A.cross(B);

//	gCameraPos = NxVec3(-25,55,15);
	gCameraPos = NxVec3(150,100,-150);
	gCameraSpeed = 0.5*5;

	// Build the four terrain components
    for (int i = 0; i < 2; i++)
    {
		for (int j = 0; j < 2; j++)
		{
			// width*step, height*step
			BuildTerrainComponent(NxVec3(31*20*i, 0, 31*20*j));
		}
	}

	AddUserDataToActors(gScene);

	gSelectedActor = box;
	gForceStrength = 100000;
    gCameraSpeed = 100;

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 804: Mesh Paging");
	AddTerrainLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

