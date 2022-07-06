// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						  LESSON 605: CHARACTER CONTROLLER
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "Lesson605.h"
#include "UpdateTime.h"

// Physics SDK globals
NxPhysicsSDK*   gPhysicsSDK = NULL;
NxScene*        gScene = NULL;

// User report globals
DebugRenderer   gDebugRenderer;
UserAllocator*	gAllocator = NULL;
NxUserOutputStream*	gErrorStream = NULL;

// HUD globals
HUD hud;

NxF32 G = -98.1;
NxVec3 gDefaultGravity(0,-98.1,0);

NxF32 gTimestepMultiplier = 1;
NxU32 gNbCharacters = 1;
 
// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,10,-30);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
const NxReal gCameraSpeed = 0.1;

bool bChaseCam = false;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 50000;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Simulation globals
bool bHardwareScene = false;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

// Terrain globals
#define TERRAIN_SIZE		33
#define TERRAIN_NB_VERTS	TERRAIN_SIZE*TERRAIN_SIZE
#define TERRAIN_NB_FACES	(TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*2
#define TERRAIN_OFFSET		-20
#define TERRAIN_WIDTH		10
#define TERRAIN_CHAOS		150

NxVec3* gTerrainVerts = NULL;
NxVec3* gTerrainNormals = NULL;
NxU32* gTerrainFaces = NULL;

TerrainData* gTerrainData = NULL;
NxActor* gTerrain = NULL;

// Character controller
NxController* controller = NULL;
NxActor* controllerActor = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

// Character controller globals
bool bFixedStep = false;
bool bPushCharacter = false;
NxVec3 gCharacterPos;
NxVec3 gCharacterVec = NxVec3(1,0,0);
NxF32 gCharacterSpeed = 10;

/*
  a(t) = cte = g
  v(t) = g*t + v0
  y(t) = g*t^2 + v0*t + y0
*/
bool gJump = false;
NxF32 gV0;
NxF32 jumpTime;

void StartJump(NxF32 v0)
{
	if (gJump)  return;
	jumpTime = 0.0f;
	gV0	= v0;
	gJump = true;
}

void StopJump()
{
	if (!gJump)  return;
	gJump = false;
}

NxF32 GetHeight(NxF32 elapsedTime)
{
	if (!gJump)  return 0.0f;
	jumpTime += elapsedTime;
	NxF32 h = G*jumpTime*jumpTime + gV0*jumpTime;
	return (h - gDefaultGravity.y)*elapsedTime;
}

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	//printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n x = Toggle Shadows\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Toggle Fixedstep\n f = Toggle Force Mode\n x = Toggle Shadows\n");
	printf("\n Special:\n --------\n 1,2,3 \t= Add New Objects to Scene\n v \t= Toggle Character Chase-Cam\n t,5 \t= Update Character extents\n 7,8,9 \t= Modify timestep multiplier\n");
}

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode)
{
	NxVec3 forceVec = forceStrength*forceDir;

	if (forceMode)
		actor->addForce(forceVec);
	else 
		actor->addTorque(forceVec);

	return forceVec;
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

   return true;
}

void SelectNextActor()
{
   NxU32 nbActors = gScene->getNbActors();
   NxActor** actors = gScene->getActors();
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
	
void ProcessKeys()
{
	bPushCharacter = false;

	// Process keys
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

			// Character controls
			case 'i': {gCharacterVec = NxVec3(0,0,1); bPushCharacter = true; break; }
			case 'k': {gCharacterVec = NxVec3(0,0,-1); bPushCharacter = true; break; }
			case 'j': {gCharacterVec = NxVec3(1,0,0); bPushCharacter = true; break; }
			case 'l': {gCharacterVec = NxVec3(-1,0,0); bPushCharacter = true; break; }
		}
	}
}

void SetupCamera()
{
	if (!bChaseCam)
	{
		// Setup camera
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
		gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
	} 
	else 
	{
		// Setup camera
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
		
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
	  	
  		#define BUFFERSIZE 25
  		static NxMat34 mbuffer[BUFFERSIZE];
  		static int frameNum = 0;
  		int index = frameNum % BUFFERSIZE;
	  	
		NxMat34 mat, camera, cameraInv;
		NxVec3 cameraX, lookAt, cr, tmp;
		NxF32 cameraInvMat[16];
		  
		mat.id();

		mat.t = gCharacterPos;
		  
		mat.M.getColumn(2, cr);
		mat.M.getColumn(0, tmp);
		mat.M.setColumn(0, cr);
		mat.M.setColumn(2, -tmp);
		  
		if(frameNum == 0)
		{
			for(int i=0; i<BUFFERSIZE; i++)
			mbuffer[i] = mat;
		}
		  
		camera = mbuffer[index];
		mbuffer[index] = mat;
		  
		camera.t.y += 2.0f;
		  
		camera.M.getColumn(0, cameraX);
		cameraX = -cameraX;
		camera.t += (5.0f * cameraX);
		  
		lookAt = (camera.t - mat.t);
		lookAt.normalize();
		  
		camera.M.setColumn(2, lookAt);
		cr = NxVec3(0,1,0).cross(lookAt);
		cr.normalize();
		camera.M.setColumn(0, cr);
		cr = lookAt.cross(cr);
		cr.normalize();
		camera.M.setColumn(1, cr);
		  
		camera.getInverse(cameraInv);
		  
		cameraInv.getColumnMajor44(cameraInvMat);
	    
		glMultMatrixf(cameraInvMat);

		gCameraPos = camera.t;
		gCameraForward = -lookAt;
		gCameraRight.cross(gCameraForward,NxVec3(0,1,0));
		  
		frameNum++;
	}

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		// Skip actors with UD_NO_RENDER flag set
		ActorUserData* ud = (ActorUserData*)(actor->userData);
	    if (ud && (ud->flags & UD_NO_RENDER))  continue;

        DrawActor(actor, gSelectedActor, true);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, true);
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

void RenderCallback()
{
    if(gScene && !bPause)  RunPhysics();

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessKeys();
	SetupCamera();
 	RenderActors(false);  // bShadows

//	if (gTerrainData)  RenderTerrain(*gTerrainData);
	RenderCharacters();

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
		if (gScene)  gDebugRenderer.renderData(*gScene->getDebugRenderable());
	}

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
		case 'r': { SelectNextActor(); break; }		
	    default: { break; }
	}
}

void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

	switch (key)
	{
		case 'v': { bChaseCam = !bChaseCam; break; }
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
		case ' ': { StartJump(70.0f); break; }  // ### should pass a max height instead
		case 'r': bFixedStep = !bFixedStep; printf("Fixed timestep: %d\n", bFixedStep);	break;
		case 'g': ResetCharacterPos(); break;
		case '1': 
		{
            NxActor* box = CreateBox(NxVec3(0,20,0), NxVec3(1,1,1), 10); 
            box->setAngularDamping(0.5);
            SetActorCollisionGroup(box, GROUP_COLLIDABLE_PUSHABLE);
            break;
        }
		case '2': 
        {
            NxActor* sphere = CreateSphere(NxVec3(0,20,0), 1, 10);
            sphere->setAngularDamping(0.5);
            SetActorCollisionGroup(sphere, GROUP_COLLIDABLE_PUSHABLE);
			break;
		}
		case '3': 
		{
            NxActor* capsule = CreateCapsule(NxVec3(0,20,0), 4, 1, 10);
            capsule->setAngularDamping(0.5);
            SetActorCollisionGroup(capsule, GROUP_COLLIDABLE_PUSHABLE);
            break;
        }
		case '5': UpdateCharacterExtents(); break;
		case '7': gTimestepMultiplier = 1.0f;  printf("Artificial timestep multiplier: %f\n", gTimestepMultiplier);	break;
		case '8': gTimestepMultiplier = 0.1f;  printf("Artificial timestep multiplier: %f\n", gTimestepMultiplier);	break;
		case '9': gTimestepMultiplier = 10.0f;  printf("Artificial timestep multiplier: %f\n", gTimestepMultiplier);	break;
		case 27 : { exit(0); break; }
		case 't':
		{
            UpdateCharacterExtents();
            break;
		}
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

void InitGlut(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("Lesson 605: Character Controller");
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
//	glClearColor(0.3f, 0.4f, 0.5f, 1.0);
	glClearColor(0, 0, 0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);

	// Setup lighting
	glEnable(GL_LIGHTING);
	NxF32 AmbientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };		   glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	NxF32 DiffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };		   glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	NxF32 SpecularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };		   glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	NxF32 Position[]		= { -10.0f, 100.0f, -4.0f, 1.0f };	   glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	NxF32 AmbientColor1[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientColor1);
    NxF32 DiffuseColor1[]    = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseColor1);
    NxF32 SpecularColor1[]   = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularColor1);
    NxF32 Position1[]        = { 0.0f, 5.0f, 0.0f, 1.0f };	       glLightfv(GL_LIGHT1, GL_POSITION, Position1);
}

NxActor* CreateTerrain()
{
	gTerrainData = NX_NEW_MEM(TerrainData, TEMP);
	gTerrainData->init(TERRAIN_SIZE, TERRAIN_OFFSET, TERRAIN_WIDTH, TERRAIN_CHAOS, true);

	// Build physical model
	NxTriangleMeshDesc terrainDesc;
	terrainDesc.numVertices					= TERRAIN_NB_VERTS;
	terrainDesc.numTriangles				= TERRAIN_NB_FACES;
	terrainDesc.pointStrideBytes			= sizeof(NxVec3);
	terrainDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	terrainDesc.points						= gTerrainData->verts;
	terrainDesc.triangles					= gTerrainData->faces;
	terrainDesc.flags						= 0;

	terrainDesc.heightFieldVerticalAxis		= NX_Y;
	terrainDesc.heightFieldVerticalExtent	= -1000.0f;

	NxTriangleMeshShapeDesc terrainShapeDesc;
	//terrainShapeDesc.meshData				= gPhysicsSDK->createTriangleMesh(terrainDesc);

	MemoryWriteBuffer writeBuffer;
	bool status = NxCookTriangleMesh(terrainDesc, writeBuffer);
	terrainShapeDesc.meshData	= gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(writeBuffer.data));

	terrainShapeDesc.group					= GROUP_COLLIDABLE_NON_PUSHABLE;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&terrainShapeDesc);
	return gScene->createActor(actorDesc);
}

void InitializeHUD()
{
	bHardwareScene = (gScene->getSimType() == NX_SIMULATION_HW);

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

	NxInitCooking();

	// Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator, NULL);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity = gDefaultGravity;
    gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}

    // Create the default material
    NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
    defaultMaterial->setRestitution(0.0);
    defaultMaterial->setStaticFriction(0.5);
    defaultMaterial->setDynamicFriction(0.5);

	// Create randomized terrain
    gTerrain = CreateTerrain();

    // Create stairs
    for(NxU32 i=0;i<20;i++)
	{	
		NxActor* box = CreateBox(NxVec3(-10 - NxF32(i), -3.6 + NxF32(i)*0.5, 0), NxVec3(2,2,2), 0);
		SetActorCollisionGroup(box, GROUP_COLLIDABLE_NON_PUSHABLE);
	}

	// Create cube too high for autostep
	NxActor* box1 = CreateBox(NxVec3(-10, -1-2, -10), NxVec3(2,2,2), 0);
	SetActorCollisionGroup(box1, GROUP_COLLIDABLE_NON_PUSHABLE);

	// Create rotated cube
	NxActor* box2 = CreateBox(NxVec3(10, 4-2, -10), NxVec3(2,2,2), 0);
	SetActorCollisionGroup(box2, GROUP_COLLIDABLE_NON_PUSHABLE);
	NxMat33 M = box2->getGlobalOrientation();
	M.rotX(0.5);
	box2->setGlobalOrientation(M);

    // Create high cube to debug jump
	NxActor* box3 = CreateBox(NxVec3(10, 10-2, 10), NxVec3(2,2,2), 0);
	SetActorCollisionGroup(box3, GROUP_COLLIDABLE_NON_PUSHABLE);

    // Create high cube to debug placement API
	NxActor* box4 = CreateBox(NxVec3(-10, 5.5-2, 10), NxVec3(2,2,2), 0);
	SetActorCollisionGroup(box4, GROUP_COLLIDABLE_NON_PUSHABLE);

	NxActor* sphere1 = CreateSphere(NxVec3(0, 0-1, 10), 1, 0);
	SetActorCollisionGroup(sphere1, GROUP_COLLIDABLE_NON_PUSHABLE);

	NxActor* sphere2 = CreateSphere(NxVec3(0, 1-1, 5), 1, 0);
	SetActorCollisionGroup(sphere2, GROUP_COLLIDABLE_NON_PUSHABLE);

	NxActor* capsule1 = CreateCapsule(NxVec3(0, -1.5-2, -10), 3, 2, 0);
	SetActorCollisionGroup(capsule1, GROUP_COLLIDABLE_NON_PUSHABLE);

	// Create walls
	NxU32 numBoxes = 20;
	NxReal halfBoxSize = 2;
	NxReal wallLength = numBoxes*halfBoxSize*2;
	for(NxU32 i=0;i<numBoxes;i++)
	{
		NxActor* box = CreateBox(NxVec3((NxF32(i)/NxF32(numBoxes-1))*wallLength - wallLength*0.5f, 0, wallLength*0.5f), NxVec3(halfBoxSize,halfBoxSize,halfBoxSize), 0);
		SetActorCollisionGroup(box, GROUP_COLLIDABLE_NON_PUSHABLE);
	}
	for(i=0;i<numBoxes;i++)
	{
		NxActor* box = CreateBox(NxVec3((NxF32(i)/NxF32(numBoxes-1))*wallLength - wallLength*0.5f, 0, -wallLength*0.5f), NxVec3(halfBoxSize,halfBoxSize,halfBoxSize), 0);
		SetActorCollisionGroup(box, GROUP_COLLIDABLE_NON_PUSHABLE);
	}
	for(i=0;i<numBoxes;i++)
	{
		NxActor* box = CreateBox(NxVec3(-wallLength*0.5f, 0, (NxF32(i)/NxF32(numBoxes-1))*wallLength - wallLength*0.5f), NxVec3(halfBoxSize,halfBoxSize,halfBoxSize), 0);
		SetActorCollisionGroup(box, GROUP_COLLIDABLE_NON_PUSHABLE);
	}
	for(i=0;i<numBoxes;i++)
	{
		NxActor* box = CreateBox(NxVec3(wallLength*0.5f, 0, (NxF32(i)/NxF32(numBoxes-1))*wallLength - wallLength*0.5f), NxVec3(halfBoxSize,halfBoxSize,halfBoxSize), 0);
		SetActorCollisionGroup(box, GROUP_COLLIDABLE_NON_PUSHABLE);
	}

	// Create the character controllers
	controller = InitCharacterControllers(gNbCharacters, NxVec3(0,10,0), 1, *gScene);
	controllerActor = controller->getActor();

	AddUserDataToActors(gScene);

	// Don't render character controller actor
	((ActorUserData*)(controllerActor->userData))->flags |= UD_NO_RENDER;

	// Set the heightfield to use light #1 for rendering
	((ActorUserData*)(gTerrain->userData))->flags |= UD_RENDER_USING_LIGHT1;

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
//	if (gScene)  StartPhysics();
}

void ReleaseNx()
{
	ReleaseCharacterControllers(*gScene);
	if (gScene) gPhysicsSDK->releaseScene(*gScene);
	if (gPhysicsSDK)  gPhysicsSDK->release();

	NX_DELETE_SINGLE(gTerrainData);
    NX_DELETE_SINGLE(gAllocator);

}

void ResetNx()
{
	ReleaseNx();
	InitNx();
}

void UpdateCharacter(NxReal deltaTime)
{
	deltaTime *= gTimestepMultiplier;

	NxVec3 disp = gDefaultGravity;
	NxExtendedVec3 v = GetCharacterPos(0);
	gCharacterPos.x = v.x;
	gCharacterPos.y = v.y;
	gCharacterPos.z = v.z;

	if (bPushCharacter)
	{
		NxVec3 horizontalDisp = gCharacterVec;
		horizontalDisp.y = 0.0f;
		horizontalDisp.normalize();

		disp += horizontalDisp * gCharacterSpeed;
	}

	NxU32 collisionFlags = MoveCharacter(0, *gScene, disp, deltaTime, COLLIDABLE_MASK, GetHeight(deltaTime));
	if (collisionFlags & NXCC_COLLISION_DOWN)  StopJump();

	float TimeStep = 1.0f / 60.0f;
	if (bFixedStep)	
		gScene->setTiming(TimeStep, 1, NX_TIMESTEP_FIXED);
	else
		gScene->setTiming(TimeStep, 1, NX_TIMESTEP_VARIABLE);
}

void PostUpdateCharacter()
{
	NxReal maxTimestep;
	NxTimeStepMethod method;
	NxU32 maxIter;
	NxU32 numSubSteps;

	gScene->getTiming(maxTimestep, maxIter, method, &numSubSteps);
	if (numSubSteps)  UpdateControllers();
}

void RunPhysics()
{
	// Update the time step
	NxReal deltaTime = UpdateTime();

	// Update the box character's position according to by testing its
	// axis-aligned bounding box against a copy of the current scene.
	UpdateCharacter(deltaTime);

	// Run collision and dynamics for delta time since the last frame
	gScene->simulate(deltaTime);
	gScene->flushStream();
	gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);

	// Update the box character's position according to results
	// gathered from the updated scene
	PostUpdateCharacter();
}

int main(int argc, char** argv)
{
	PrintControls();
    InitGlut(argc, argv);
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}



