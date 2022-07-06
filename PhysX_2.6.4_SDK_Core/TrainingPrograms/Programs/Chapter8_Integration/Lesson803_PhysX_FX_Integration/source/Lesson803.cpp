// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			              LESSON 803: PHYSX FX INTEGRATION
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

// Scene 0 is software scene running synchronously
// Scene 1 is hardware scene running asynchronously

#include "Lesson803.h"
#include "UpdateTime.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*		  gScenes[2] = { NULL, NULL };
NxVec3            gDefaultGravity(0,-9.8,0);
 
// User report globals
DebugRenderer     gDebugRenderer;
UserAllocator*	  gAllocator = NULL;
ErrorStream       gErrorStream;

// HUD globals
HUD hud;

// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,5,-15);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
NxReal gCameraSpeed = 0.1;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 50000;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Simulation globals
NxReal gDeltaTime = 1.0/60.0;
bool bHardwareScene = false;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

// Actor globals
NxActor* groundPlane = NULL;
NxActor* sphere = NULL;
NxActor** boxes = NULL;

PFXActor** pfxActor;

// Focus actor
NxActor* gSelectedActor = NULL;

// Focus scene
NxU32 gSelectedSceneNum = 0;  // Start with software scene in focus

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------------\n g = Switch Scenes\n");
}

bool IsSelectable(NxActor* actor)
{
   NxShape*const* shapes = gSelectedActor->getShapes();
   NxU32 nShapes = gSelectedActor->getNbShapes();
   while (nShapes--)
   {
       if (shapes[nShapes]->getFlag(NX_TRIGGER_ENABLE)) 
       {           
           return false;
       }
   }

   if (actor == groundPlane)
       return false;

   return true;
}

void SelectNextActor()
{
    if (gScenes[gSelectedSceneNum])
    {
        NxU32 nbActors = gScenes[gSelectedSceneNum]->getNbActors();
        NxActor** actors = gScenes[gSelectedSceneNum]->getActors();
        for (NxU32 i = 0; i < nbActors; i++)
        {
            if (actors[i] == gSelectedActor)
            {
                NxU32 j = 1;
                gSelectedActor = actors[(i+j)%nbActors];
                while (!IsSelectable(gSelectedActor))
                {
                    j++;
                    gSelectedActor = actors[(i+j)%nbActors];
                }
                break;
            }
        }
    }
}

void ProcessCameraKeys()
{
	// Process camera keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Camera controls
			case 'w':{ gCameraPos += gCameraForward*gCameraSpeed; break; }
			case 's':{ gCameraPos -= gCameraForward*gCameraSpeed; break; }
			case 'a':{ gCameraPos -= gCameraRight*gCameraSpeed; break; }
			case 'd':{ gCameraPos += gCameraRight*gCameraSpeed; break; }
			case 'z':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed; break; }
		}
	}
}

void SetupCamera()
{
	// Setup camera
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderActors(bool shadows)
{
	for (NxU32 i = 0; i < 2; i++)
	{
		if (gScenes[i])
		{
            // Render all the actors in the scene
            NxU32 nbActors = gScenes[i]->getNbActors();
            NxActor** actors = gScenes[i]->getActors();
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
	}
}

void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color)
{
	// Draw only if the force is large enough
	NxReal force = forceVec.magnitude();
	if (force < 0.1)  return;

	forceVec = 3*forceVec/force;

	NxVec3 pos = actor->getCMassGlobalPosition();
	DrawArrow(pos, pos + forceVec, color);
}

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode)
{
	NxVec3 forceVec = forceStrength*forceDir*gDeltaTime;

	if (forceMode)
		actor->addForce(forceVec);
	else 
		actor->addTorque(forceVec);

	return forceVec;
}

void ProcessForceKeys()
{
    if (!gSelectedActor)  return;

	// Process force keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Force controls
			case 'i': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,1),gForceStrength,bForceMode); break; }
			case 'k': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
			case 'j': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(1,0,0),gForceStrength,bForceMode); break; }
			case 'l': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'u': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,1,0),gForceStrength,bForceMode); break; }
			case 'm': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
		}
	}
}

void UpdatePFXActors()
{
	NxU32 nbActors = gScenes[0]->getNbActors();
	NxActor** actors = gScenes[0]->getActors();

	// Update the hardware kinematic actors shadowing the software actors
	for (NxU32 i = 0; i < nbActors; i++)
	{
		if (pfxActor[i]->actor[0]->isDynamic())
		{
		    pfxActor[i]->actor[1]->moveGlobalPose(pfxActor[i]->actor[0]->getGlobalPose());
		}
	}
}

void ProcessInputs()
{
    ProcessForceKeys();

	// Update the hardware kinematic actors shadowing the software actors
	UpdatePFXActors();

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
	    for (NxU32 i = 0; i < 2; i++)
	    {
		    if (gScenes[i])
		    {
			    gDebugRenderer.renderData(*gScenes[i]->getDebugRenderable());
		    }
		}
	}
}

void RenderCallback()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessCameraKeys();
	SetupCamera();

	if (!bPause)
	{
	    NxReal gDeltaTime = UpdateTime();

        // Software and hardware scenes both run asynchronously
		if (gScenes[0] && gScenes[1])
		{
			GetPhysicsResults(0);
			GetPhysicsResults(1);
			ProcessInputs();
			StartPhysics(0, gDeltaTime);
			StartPhysics(1, gDeltaTime);
		}
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

void ReshapeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
    gCameraAspectRatio = float(width)/float(height);
}

void IdleCallback()
{
    glutPostRedisplay();
}

void KeyboardCallback(unsigned char key, int x, int y)
{
	gKeys[key] = true;

	switch (key)
	{
		case 'r': {	SelectNextActor(); break; }
		default: { break; }
	}
}

void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

	switch (key)
	{
		case 'p': { bPause = !bPause; 
					if (bPause)
						hud.SetDisplayString(1, "Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
					else
						hud.SetDisplayString(1, "", 0.0f, 0.0f);	
					UpdateTime(); 
					break; }
		case 'x': { bShadows = !bShadows; break; }
		case 'b': { bDebugWireframeMode = !bDebugWireframeMode; break; }		
		case 'f': { bForceMode = !bForceMode; break; }
	    case 'g': { gSelectedSceneNum = (++gSelectedSceneNum)%2;
				    gSelectedActor = gScenes[gSelectedSceneNum]->getActors()[0];
			        break; }
		case 27 : { exit(0); break; }
		default : { break; }
	}
}

void SpecialCallback(int key, int x, int y)
{
	switch (key)
    {
		// Reset PhysX
		case GLUT_KEY_F10: ResetNx(); return; 
	}
}

void MouseCallback(int button, int state, int x, int y)
{
    mx = x;
    my = y;
}

void MotionCallback(int x, int y)
{
    int dx = mx - x;
    int dy = my - y;
    
    gCameraForward.normalize();
    gCameraRight.cross(gCameraForward,NxVec3(0,1,0));

    NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));
    qx.rotate(gCameraForward);
    NxQuat qy(NxPiF32 * dy * 20 / 180.0f, gCameraRight);
    qy.rotate(gCameraForward);

    mx = x;
    my = y;
}

void ExitCallback()
{
	ReleaseNx();
}

void InitGlut(int argc, char** argv, char* lessonTitle)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    gMainHandle = glutCreateWindow(lessonTitle);
    glutSetWindow(gMainHandle);
    glutDisplayFunc(RenderCallback);
    glutReshapeFunc(ReshapeCallback);
    glutIdleFunc(IdleCallback);
    glutKeyboardFunc(KeyboardCallback);
    glutKeyboardUpFunc(KeyboardUpCallback);
	glutSpecialFunc(SpecialCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MotionCallback);
	MotionCallback(0,0);
	atexit(ExitCallback);

    // Setup default render states
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);

    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

NxActor* CreateGroundPlane(int sceneIndex)
{
    // Create a plane with default descriptor
    NxPlaneShapeDesc planeDesc;
    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack(&planeDesc);
    return gScenes[sceneIndex]->createActor(actorDesc);
}

NxActor* CreateBox(int sceneIndex, const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(boxDim.x,boxDim.y,boxDim.z);
	boxDesc.localPose.t = NxVec3(0,boxDim.y,0);
	actorDesc.shapes.pushBack(&boxDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = density;
	actorDesc.globalPose.t = pos;

	return gScenes[sceneIndex]->createActor(actorDesc);
}

NxActor* CreateSphere(int sceneIndex, const NxVec3& pos, const NxReal radius, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a sphere
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.localPose.t = NxVec3(0,radius,0);
	actorDesc.shapes.pushBack(&sphereDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = density;
	actorDesc.globalPose.t = pos;
	return gScenes[sceneIndex]->createActor(actorDesc);	
}

NxActor* CreateCapsule(int sceneIndex, const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a capsule
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.height = height;
	capsuleDesc.radius = radius;
	capsuleDesc.localPose.t = NxVec3(0,radius+0.5*height,0);
	actorDesc.shapes.pushBack(&capsuleDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = density;
	actorDesc.globalPose.t = pos;
	return gScenes[sceneIndex]->createActor(actorDesc);	
}

NxActor* CreatePyramid(int sceneIndex, const NxVec3& pos, const NxVec3& boxDim, const NxReal density, NxConvexMeshDesc& cmd)
{
    NxBodyDesc bodyDesc;
    NxActorDesc actorDesc;

    // Pyramid
    NxVec3 verts[8] = { NxVec3(boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,-boxDim.z), NxVec3(-boxDim.x,-boxDim.y,boxDim.z), NxVec3(boxDim.x,-boxDim.y,boxDim.z), 
                        NxVec3(boxDim.x*0.5,boxDim.y,-boxDim.z*0.5), NxVec3(-boxDim.x*0.5,boxDim.y,-boxDim.z*0.5), NxVec3(-boxDim.x*0.5,boxDim.y,boxDim.z*0.5), NxVec3(boxDim.x*0.5,boxDim.y,boxDim.z*0.5) };

    // Create descriptor for triangle mesh
    NxConvexMeshDesc convexDesc;
    convexDesc.numVertices			= 8;
    convexDesc.pointStrideBytes		= sizeof(NxVec3);
    convexDesc.points				= verts;
    convexDesc.flags				= NX_CF_COMPUTE_CONVEX;

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
	    if (density)
   	    {
		    actorDesc.body = &bodyDesc;
		    actorDesc.density = density;
	    }
	    else
	    {
		    actorDesc.body = NULL;
	    }
        actorDesc.globalPose.t  = pos;
		NxActor* actor = gScenes[sceneIndex]->createActor(actorDesc);

		// Attach drawable mesh to mesh shape's user data
		NxShape*const* shapes = actor->getShapes();
        shapes[0]->isConvexMesh()->getConvexMesh().saveToDesc(cmd);
	    shapes[0]->userData = (void*)&cmd;

		return actor;
//      gPhysicsSDK->releaseConvexMesh(*convexShapeDesc.meshData);
    }
    return NULL;
}

NxActor** CreateStack(int sceneIndex, const NxVec3& pos, const NxVec3& stackDim, const NxVec3& boxDim, NxReal density)
{
	NxActor** stack = new NxActor*[(int)(8*stackDim.x*stackDim.y*stackDim.z)]; 

	NxVec3 offset = NxVec3(boxDim.x,0,boxDim.z) + pos;

	int count = 0;
	for (int i = -stackDim.x; i < stackDim.x; i++)
	{
		for (int j = 0; j < stackDim.y; j++)
		{
			for (int k = -stackDim.z; k < stackDim.z; k++)
			{
				NxVec3 boxPos = NxVec3(i*boxDim.x*2,j*boxDim.y*2,k*boxDim.z*2);
				stack[count++] = CreateBox(sceneIndex, boxPos + offset, boxDim, density);
			}
		}
	}

	return stack;
}

void AddPFXActor(int i, NxActor* actor)
{
    NxActorDesc actorDesc;
    NxBodyDesc bodyDesc;

	pfxActor[i] = new PFXActor;
	pfxActor[i]->actor[0] = actor;
	actor->saveToDesc(actorDesc);

	NxU32 nbShapes = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	while (nbShapes--)
	{
		NxShape* shape = shapes[nbShapes];
        switch (shape->getType())
        {
			case NX_SHAPE_PLANE:
			{
   	      	    NxPlaneShapeDesc planeDesc;
        	    shape->isPlane()->saveToDesc(planeDesc);
        	    actorDesc.shapes.pushBack(&planeDesc);	
			}
			break;
			case NX_SHAPE_BOX:
			{
         	    NxBoxShapeDesc boxDesc;
        	    shape->isBox()->saveToDesc(boxDesc);
        	    actorDesc.shapes.pushBack(&boxDesc);	
			}
			break;
			case NX_SHAPE_SPHERE:
			{
         	    NxSphereShapeDesc sphereDesc;
        	    shape->isSphere()->saveToDesc(sphereDesc);
        	    actorDesc.shapes.pushBack(&sphereDesc);	
			}
			break;
			case NX_SHAPE_CAPSULE:
			{
         	    NxCapsuleShapeDesc capsuleDesc;
        	    shape->isCapsule()->saveToDesc(capsuleDesc);
        	    actorDesc.shapes.pushBack(&capsuleDesc);	
			}
			break;
			case NX_SHAPE_CONVEX:
			{
			}
			break;
			case NX_SHAPE_MESH:
			{
			}
			break;
		}
	}

	if (actor->isDynamic())
		actorDesc.body = &bodyDesc;
	else
		actorDesc.body = NULL;

    pfxActor[i]->actor[1] = gScenes[1]->createActor(actorDesc);

	if (actor->isDynamic())
	{
		pfxActor[i]->actor[1]->raiseBodyFlag(NX_BF_KINEMATIC);
	}
}

void CreatePFXActors()
{
	NxU32 nbActors = gScenes[0]->getNbActors();
	NxActor** actors = gScenes[0]->getActors();

	pfxActor = new PFXActor*[nbActors];

	for (NxU32 i = 0; i < nbActors; i++)
	{
		AddPFXActor(i, actors[i]);
	}
}

void InitializeHUD()
{
	bHardwareScene = (gScenes[0]->getSimType() == NX_SIMULATION_HW);

	// Add hardware/software to HUD
	if (bHardwareScene)
	    hud.AddDisplayString("Hardware Scene", 0.74f, 0.92f);
	else
		hud.AddDisplayString("Software Scene", 0.74f, 0.92f);

	// Add pause to HUD
	if (bPause)  
		hud.AddDisplayString("Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
	else
		hud.AddDisplayString("", 0.0f, 0.0f);
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

    // Create two scenes
	for (NxU32 i = 0; i < 2; i++)
	{
        NxSceneDesc sceneDesc;
        sceneDesc.gravity               = gDefaultGravity;
		if (i == 0)
	       sceneDesc.simType			= NX_SIMULATION_SW;
		else
	       sceneDesc.simType			= NX_SIMULATION_HW;
        gScenes[i] = gPhysicsSDK->createScene(sceneDesc);
		if(!gScenes[i])
		{
			sceneDesc.simType	   = NX_SIMULATION_SW;
			gScenes[i]			   = gPhysicsSDK->createScene(sceneDesc);
		}

        // Create the default material
        NxMaterial* defaultMaterial = gScenes[i]->getMaterialFromIndex(0); 
        defaultMaterial->setRestitution(0.5);
        defaultMaterial->setStaticFriction(0.5);
        defaultMaterial->setDynamicFriction(0.5);
	}

	// Create the objects in the two scenes
	groundPlane = CreateGroundPlane(0);
	sphere = CreateSphere(0, NxVec3(0,6,0), 1, 50);
	boxes = CreateStack(1, NxVec3(0,0,0), NxVec3(6,3,6), NxVec3(0.25,0.25,0.25), 10);

	// Create the PhysX FX actors
	CreatePFXActors();

	AddUserDataToActors(gScenes[0]);
	AddUserDataToActors(gScenes[1]);

	gSelectedActor = sphere;

	// Initialize HUD
	InitializeHUD();

	// Update the time step
	UpdateTime();
	gDeltaTime = UpdateTime();

	// Start the first frame of each simulation
	for (NxU32 i = 0; i < 2; i++)
	{
	    StartPhysics(i, gDeltaTime);
	}
}

void ReleaseNx()
{
	for (NxU32 i = 0; i < 2; i++)
	{
	    if (gScenes[i])  gPhysicsSDK->releaseScene(*gScenes[i]);
	}
	if (gPhysicsSDK)  gPhysicsSDK->release();
    NX_DELETE_SINGLE(gAllocator);
}

void ResetNx()
{
	ReleaseNx();
	InitNx();
}

void StartPhysics(NxU32 sceneNum, NxReal deltaTime)
{
    if (gScenes[sceneNum])
    {
        // Start collision and dynamics for delta time since the last frame
        gScenes[sceneNum]->simulate(deltaTime);
        gScenes[sceneNum]->flushStream();
	}
}

void GetPhysicsResults(NxU32 sceneNum)
{
    if (gScenes[sceneNum])
    {
        // Get results from gScene->simulate(gDeltaTime)
        while (!gScenes[sceneNum]->fetchResults(NX_RIGID_BODY_FINISHED, false));
    }
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 803: PhysX FX Integration");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
