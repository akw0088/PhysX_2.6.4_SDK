// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					 LESSON 602: RAYCASTING AGAINST HEIGHTFIELDS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson602.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

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
bool bRaycastClosestShape = true;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;
extern NxActor* concaveObject;
extern NxActor* heightfield;
NxActor* emitter = NULL;

// Focus actor
extern NxActor* gSelectedActor;

// Mesh globals
extern NxTriangleMeshDesc heightfieldMeshDesc;

// Collision group globals
const NxCollisionGroup groupA = 1;
const NxU32 groupFlagA = 1 << groupA;

const NxCollisionGroup groupB = 2;
const NxU32 groupFlagB = 1 << groupB;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n");
	printf("\n Special:\n --------\n v = Toggle Raycast Closest Shape/Raycast All Shapes\n");
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

void RaycastClosestShapeFromActor(NxActor* actor, NxU32 groupFlag)
{
	// Get ray origin
	NxVec3 orig = actor->getCMassGlobalPosition();

	// Get ray direction
	NxVec3 dir;
	NxMat33 m = actor->getGlobalOrientation();
    m.getColumn(0, dir);
	dir = -dir;

	NxRay ray(orig, dir);
	NxRaycastHit hit;
	NxReal dist;

	// Get the closest shape
	NxShape* closestShape = gScene->raycastClosestShape(ray, NX_ALL_SHAPES, hit, groupFlag);
	if (closestShape)
	{
		const NxVec3& worldImpact = hit.worldImpact;
		dist = hit.distance;

		RaycastPoint rp(worldImpact, NxVec3(1,0,0));
		rpArray.pushBack(rp);

		RaycastLine rl(ray.orig, worldImpact, NxVec3(0,0,1));
		rlArray.pushBack(rl);
	} 
	else 
	{
		dist = 10000;

		RaycastLine rl(ray.orig, ray.orig + dist*dir, NxVec3(0,0,1));
		rlArray.pushBack(rl);
	}
}

void RaycastAllShapesFromActor(NxActor* actor, NxU32 groupFlag)
{
	// Get ray origin
	NxVec3 orig = actor->getCMassGlobalPosition();

	// Get ray direction
	NxVec3 dir;
	NxMat33 m = actor->getGlobalOrientation();
    m.getColumn(0, dir);
	dir = -dir;

	NxRay ray(orig, dir);
    NxReal dist = 10000;

	RaycastLine rl(ray.orig, ray.orig + dist*dir, NxVec3(0,0,1));
	rlArray.pushBack(rl);	
		
	// Get all shapes
	NxU32 nbShapes = gScene->raycastAllShapes(ray, gRaycastReport, NX_ALL_SHAPES, groupFlag);
}

void ProcessInputs()
{
    ProcessMoveKeys();

	// Cast a ray out of the emitter along its negative x-axis
	if (bRaycastClosestShape)
		RaycastClosestShapeFromActor(emitter, groupFlagA | groupFlagB);
	else
		RaycastAllShapesFromActor(emitter, groupFlagA | groupFlagB);

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

	// Draw raycast points and normals
	NxU32 i;
	for (i = 0; i < rpArray.size(); i++)
	{
		RaycastPoint point = rpArray[i];
		DrawContactPoint(point.p, 1.0, point.color);
	}
	for (i = 0; i < rlArray.size(); i++)
	{
		RaycastLine line = rlArray[i];
		DrawArrow(line.p0, line.p1, line.color);
	}
	for (i = 0; i < rtArray.size(); i++)
	{
		RaycastTriangle tri = rtArray[i];
		DrawLine(tri.p0, tri.p1, tri.color);
		DrawLine(tri.p1, tri.p2, tri.color);
		DrawLine(tri.p2, tri.p0, tri.color);
	}
	rpArray.clear();
	rlArray.clear();
	rtArray.clear();

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
	switch (key)
	{
        case 'v': { bRaycastClosestShape = !bRaycastClosestShape; break; }
	}
}

NxActor* CreateEmitterBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(boxDim.x,boxDim.y,boxDim.z);
	boxDesc.localPose.t = NxVec3(0,0,0);
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

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
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

	gCameraPos = NxVec3(-25,55,15);
	gCameraSpeed = 10;

	emitter = CreateEmitterBox(gCameraPos + NxVec3(5,0,15),NxVec3(0.5,0.25,0.25), 1);
	emitter->raiseBodyFlag(NX_BF_KINEMATIC);

	box = CreateBox(gCameraPos + NxVec3(5,0,10), NxVec3(0.5,1,0.5), 20);
	sphere = CreateSphere(gCameraPos + NxVec3(0,0,15), 1, 10);
	capsule = CreateCapsule(gCameraPos + NxVec3(-5,0,10), 2, 0.5, 10);
	pyramid = CreatePyramid(gCameraPos + NxVec3(0,0,10), NxVec3(1,0.5,1.5), 10);
	concaveObject = CreateConcaveObject(gCameraPos + NxVec3(-4,0,15), NxVec3(1,1,1), 5);
	heightfield = CreateHeightfield(NxVec3(0,0,0), 0, 200);

	SetActorCollisionGroup(box, groupA);
	SetActorCollisionGroup(sphere, groupA);
	SetActorCollisionGroup(capsule, groupA);
	SetActorCollisionGroup(pyramid, groupA);
	SetActorCollisionGroup(concaveObject, groupA);
	SetActorCollisionGroup(heightfield, groupA);

	AddUserDataToActors(gScene);

	// We want to attach the raw mesh to the heightfield shape
	// user data rather than the cooked one.  Cooking the mesh
	// changes the face indices and we want the original mesh
	// attached with the original ordering intact to get the 
	// correct triangles from the raycast queries.
	NxShape** shapes = (NxShape**)(heightfield->getShapes());
    ShapeUserData* sud = (ShapeUserData*)(shapes[0]->userData);
	if (sud && sud->mesh)
	{
		delete sud->mesh;
		sud->mesh = &heightfieldMeshDesc;
	}

	// Set the heightfield to use light #1 for rendering
	((ActorUserData*)(heightfield->userData))->flags |= UD_RENDER_USING_LIGHT1;

	// Page in the hardware meshes
	PageInHardwareMeshes(gScene);

	gSelectedActor = box;
	gForceStrength = 100000;

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
	InitGlut(argc, argv, "Lesson 602: Raycasting against Heightfields");
	AddTerrainLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
