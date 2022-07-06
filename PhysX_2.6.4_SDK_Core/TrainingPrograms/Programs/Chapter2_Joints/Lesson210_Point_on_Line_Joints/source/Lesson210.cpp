// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						  LESSON 210: POINT-ON-LINE JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson210.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

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

// Actor globals
extern NxActor* groundPlane;
NxActor* wedge = NULL;
extern NxActor* pyramid;

// Joint globals
NxPointOnLineJoint* polJoint = NULL;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
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
}

NxActor* CreateRegularPyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Pyramid
	NxVec3 verts[5] = {	NxVec3(boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,boxDim.z), NxVec3(boxDim.x,-boxDim.y,boxDim.z),
						NxVec3(0,boxDim.y,0) };

    // Create descriptor for convex mesh
    NxConvexMeshDesc convexDesc;
    convexDesc.numVertices			= 5;
    convexDesc.pointStrideBytes		= sizeof(NxVec3);
    convexDesc.points				= verts;
    convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

    NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);

    NxInitCooking();
    if (0)
    {
        // Cooking from file
#ifdef WIN32
        bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
        convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookConvexMesh(convexDesc, buf);
        convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
    }

    if (convexShapeDesc.meshData)
    {
        NxActorDesc actorDesc;
        actorDesc.shapes.pushBack(&convexShapeDesc);
        actorDesc.body			= &bodyDesc;
        actorDesc.density		= density;
        actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
//      gPhysicsSDK->releaseTriangleMesh(*convexShapeDesc.meshData);
    }

    return NULL;
}

NxActor* CreateDownWedge(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// Down wedge
	NxVec3 verts[6] = {	NxVec3(boxDim.x,boxDim.y,-boxDim.z), NxVec3(boxDim.x,boxDim.y,boxDim.z), NxVec3(boxDim.x,-boxDim.y,0),
						NxVec3(-boxDim.x,boxDim.y,-boxDim.z), NxVec3(-boxDim.x,boxDim.y,boxDim.z), NxVec3(-boxDim.x,-boxDim.y,0) };

    // Create descriptor for convex mesh
    NxConvexMeshDesc convexDesc;
    convexDesc.numVertices			= 6;
    convexDesc.pointStrideBytes		= sizeof(NxVec3);
    convexDesc.points				= verts;
    convexDesc.flags				= NX_CF_COMPUTE_CONVEX | NX_CF_USE_LEGACY_COOKER;

    NxConvexShapeDesc convexShapeDesc;
	convexShapeDesc.localPose.t		= NxVec3(0,boxDim.y,0);

    NxInitCooking();
    if (0)
    {
        // Cooking from file
#ifdef WIN32
        bool status = NxCookConvexMesh(convexDesc, UserStream("c:\\tmp.bin", false));
        convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(UserStream("c:\\tmp.bin", true));
#endif
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookConvexMesh(convexDesc, buf);
        convexShapeDesc.meshData = gPhysicsSDK->createConvexMesh(MemoryReadBuffer(buf.data));
    }

    if (convexShapeDesc.meshData)
    {
        NxActorDesc actorDesc;
        actorDesc.shapes.pushBack(&convexShapeDesc);
        actorDesc.body			= &bodyDesc;
        actorDesc.density		= density;
        actorDesc.globalPose.t  = pos;
		NxActor* actor = gScene->createActor(actorDesc);

		return actor;
//      gPhysicsSDK->releaseTriangleMesh(*convexShapeDesc.meshData);
    }

    return NULL;
}

NxPointOnLineJoint* CreatePointOnLineJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis)
{
	NxPointOnLineJointDesc polDesc;
	polDesc.actor[0] = a0;
	polDesc.actor[1] = a1;
	polDesc.setGlobalAnchor(globalAnchor);
	polDesc.setGlobalAxis(globalAxis);
	polDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	return (NxPointOnLineJoint*)gScene->createJoint(polDesc);
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

	wedge = CreateDownWedge(NxVec3(0,6,0), NxVec3(5,1,1), 10);
	wedge->raiseBodyFlag(NX_BF_KINEMATIC);
	pyramid = CreateRegularPyramid(NxVec3(0,4,0), NxVec3(1,1,1), 10);
	pyramid->setLinearDamping(0.5);
	pyramid->setAngularDamping(0.5);

	NxVec3 globalAnchor = NxVec3(0,6,0);
	NxVec3 globalAxis = NxVec3(1,0,0);
	polJoint = CreatePointOnLineJoint(wedge, pyramid, globalAnchor, globalAxis);
	
	polJoint->setLimitPoint(globalAnchor);
	// Add left-right limiting planes
	polJoint->addLimitPlane(-globalAxis, globalAnchor + 5*globalAxis);
	polJoint->addLimitPlane(globalAxis, globalAnchor - 5*globalAxis);

	AddUserDataToActors(gScene);

	gSelectedActor = pyramid;
	gForceStrength = 25000;

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
	InitGlut(argc, argv, "Lesson 210: Point-on-Line Joints");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
