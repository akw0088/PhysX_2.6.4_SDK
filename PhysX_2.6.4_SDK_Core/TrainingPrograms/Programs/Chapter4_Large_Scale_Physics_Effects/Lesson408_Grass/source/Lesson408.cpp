// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					              LESSON 408: GRASS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson408.h"

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

class Blade
{
public:
    Blade(const NxVec3& pos, const NxReal height, const NxU32 numJoints)
	{
		NxReal partHeight = height / (NxReal)numJoints;
		NxReal radius = 0.1;
		NxVec3 globalAnchor;

		bladeSegs = new NxActor*[numJoints];
		bladeLinks = new NxSphericalJoint*[numJoints];

		NxU32 i;
		for (i = 0; i < numJoints; i++)
		{
			bladeSegs[i] = CreateBlade(pos+NxVec3(0,partHeight/2+radius+(partHeight+radius*2)*i,0),NxVec3(radius/2,(partHeight+2*radius)/2,radius/2),numJoints-i+1);
			bladeSegs[i]->setLinearDamping(10);
			SetActorCollisionGroup(bladeSegs[i],1);
		}

		// Anchor blade to ground
		globalAnchor = bladeSegs[0]->getCMassGlobalPosition() - NxVec3(0,partHeight/2,0);
		bladeLinks[0] = CreateBladeLink(bladeSegs[0],NULL,globalAnchor, NxVec3(0,1,0));

		for (i = 1; i < numJoints; i++)
		{
			globalAnchor = bladeSegs[i]->getCMassGlobalPosition() - NxVec3(0,partHeight/2,0);
			bladeLinks[i] = CreateBladeLink(bladeSegs[i], bladeSegs[i-1], globalAnchor, NxVec3(0,1,0));
		}
	}

	NxActor** bladeSegs;
	NxSphericalJoint** bladeLinks;
};

const NxReal scale = 0.45;

// Actor globals
extern NxActor* groundPlane;

const NxI32 patchsize = 8;
Blade* grassBlade[patchsize*2][patchsize*2];

NxActor* ball = NULL;

// Focus actor
extern NxActor* gSelectedActor;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
}

void NewLight()
{
    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, -100.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;
        DrawActor(actor, gSelectedActor, false);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow2(actor, false);
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
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);

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

//	gScene->setTiming(0.01);  // timeStep, maxIter, TIMESTEP_FIXED

	// Create the default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.5);
	defaultMaterial->setStaticFriction(0.5);
	defaultMaterial->setDynamicFriction(0.5);

	// Create the objects in the scene
	groundPlane = CreateGroundPlane();

	NxU32 r;
	NxI32 x, z;
	NxReal fRX, fRZ;

	for (x = -patchsize; x < patchsize; x++)
	{
		for (z = -patchsize; z < patchsize; z++)
		{
			fRX = NxMath::rand(-45,45)*0.01;
			fRZ = NxMath::rand(-45,45)*0.01;

			r = NxMath::rand(0,4);

			if (r == 0) 
			{
				grassBlade[x+patchsize][z+patchsize] = new Blade(NxVec3(scale*(x+fRX),0,scale*(z+fRZ)),3,4);
			} 
			else if (r == 1) 
			{ 
				grassBlade[x+patchsize][z+patchsize] = new Blade(NxVec3(scale*(x+fRX),0,scale*(z+fRZ)),1.8,2);
			} 
			else if (r == 1) 
			{ 
				grassBlade[x+patchsize][z+patchsize] = new Blade(NxVec3(scale*(x+fRX),0,scale*(z+fRZ)),2,3);
			} 
			else if (r == 1) 
			{ 
				grassBlade[x+patchsize][z+patchsize] = new Blade(NxVec3(scale*(x+fRX),0,scale*(z+fRZ)),1.5,2);
			} 
			else 
			{ 
				grassBlade[x+patchsize][z+patchsize] = new Blade(NxVec3(scale*(x+fRX),0,scale*(z+fRZ)),2.1,3);
			}
		}
	}

	gScene->setGroupCollisionFlag(1,1,false);

	ball = CreateBall(NxVec3(0,10,0),1.5,350);

	SetActorCollisionGroup(ball, 3);
	
	gSelectedActor = ball;
	gCameraSpeed = 20;
	gForceStrength = 850000;

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
	InitGlut(argc, argv, "Lesson 408: Grass");
	NewLight();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
