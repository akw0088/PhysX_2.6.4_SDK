// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                LESSON 502: LOADING PML FILES
//
// This project is meant to demonstrate how you can load art assets into the 
// PhysX SDK.  The xml file format is based on the COLLADA format which is 
// supported by Max, Maya, XSI, and Equinox.  Although the format also deals 
// with graphics and animation, the physics data is in its own section and can be 
// used separately with any rendering pipeline.  The physics schema reflects how 
// physics models would be created in DCC tools.  Yet, the semantic of the data is 
// also good fit for PhysX.  Only some minor massaging of the data may be needed 
// to conform exactly to the API.
//
//						   Written by Stan Melax, 8-15-05
//                          Edited by Bob Schade, 5-1-06
// ===============================================================================

#include "Lesson502.h"
#include "UpdateTime.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;

#ifdef Z_UP
NxVec3            gDefaultGravity(0,0,-9.8);
#else
NxVec3            gDefaultGravity(0,-9.8,0);
#endif  // Z_UP

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

#ifdef Z_UP
NxVec3 gCameraPos(0,-15,5);
NxVec3 gCameraForward(0,1,0);
NxVec3 gCameraRight(1,0,0);
#else
NxVec3 gCameraPos(0,5,-15);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
#endif  // Z_UP

const NxReal gCameraSpeed = 10;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 5000;
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

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/pml";
char fname1[] = "../../../Data/pml";
char fname2[] = "../../TrainingPrograms/Programs/Data/pml";
char fname3[] = "TrainingPrograms/Programs/Data/pml";

extern Array<const char *> nxContext;

class MyOutputStream : public NxUserOutputStream
{
public:
	void context(int &k)
	{
			if(nxContext.count)
			{
				String s("context: ");
				for(int i=0;i<nxContext.count;i++){
					if(i) s+="::";
					s+= nxContext[i];
				}
				printf("%s\n",(const char *)s);
			}
	}
	void reportError (NxErrorCode code, const char *message, const char *file, int line)
	{
		if (code < NXE_DB_INFO)
		{
			static int k=5;
			context(k);
			printf("Error: %s",message);
			k%=20;
		}
	}
	NxAssertResponse reportAssertViolation (const char *message,const char *file, int line)
	{
		assert(0);
		return NX_AR_CONTINUE;
	}
	void print (const char *message)
	{
			static int k=5;
			context(k);
			printf("%s\n", message);
			k%=20;
	}
} myOutputStream;
NxUserOutputStream *gOutputStrem = &myOutputStream;

// Actor globals
NxActor* groundPlane = NULL;
NxActor* box = NULL;
NxActor* sphere = NULL;
NxActor* capsule = NULL;
NxActor* pyramid = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
#ifdef Z_UP
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,0,5)\n");
#else
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
#endif  // Z_UP
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
   NxU32 nbActors = gScene->getNbActors();
   NxActor** actors = gScene->getActors();
   for(NxU32 i = 0; i < nbActors; i++)
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

void ProcessCameraKeys()
{
	NxReal deltaTime;

    if (bPause) deltaTime = 0.0005; else deltaTime = gDeltaTime;

	// Process camera keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Camera controls
			case 'w':{ gCameraPos += gCameraForward*gCameraSpeed*deltaTime; break; }
			case 's':{ gCameraPos -= gCameraForward*gCameraSpeed*deltaTime; break; }
			case 'a':{ gCameraPos -= gCameraRight*gCameraSpeed*deltaTime; break; }
			case 'd':{ gCameraPos += gCameraRight*gCameraSpeed*deltaTime; break; }

#ifdef Z_UP
			case 'z':{ gCameraPos -= NxVec3(0,0,1)*gCameraSpeed*deltaTime; break; }
			case 'q':{ gCameraPos += NxVec3(0,0,1)*gCameraSpeed*deltaTime; break; }
#else
			case 'z':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed*deltaTime; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed*deltaTime; break; }
#endif  // Z_UP
		}
	}
}

void SetupCamera()
{
	// Setup camera
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);

#ifdef Z_UP
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 0.0f, 1.0f);
#else
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);
#endif  // Z_UP

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

#ifdef Z_UP
static int dispListPlaneZUp;

void InitZUpPlaneDisplayList()
{
	// ZUP PLANE DISPLAY LIST
	glLoadIdentity();
	dispListPlaneZUp = glGenLists(1);
	glNewList(dispListPlaneZUp, GL_COMPILE);

	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glVertex3f(1,-1,0);
	glVertex3f(1,1,0);
	glVertex3f(-1,1,0);
	glVertex3f(-1,-1,0);

	glEnd();
	glEndList();
}

void DrawPlaneZUp()
{
    NxMat34 pose;

	pose.id();

	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
	pose.t.z -= 0.1f;
	SetupGLMatrix(pose.t, pose.M);
	glScaled(1024,1024,0);
	glCallList(dispListPlaneZUp);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}
#endif  // Z_UP

void RenderActors(bool shadows)
{
#ifdef Z_UP
	DrawPlaneZUp();
#endif  // Z_UP

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
#ifdef Z_UP
			DrawActorShadowZUp(actor, true);
#else
			DrawActorShadow(actor, true);
#endif  // Z_UP
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
	// Process force keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Force controls
#ifdef Z_UP
			case 'i': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,+1,0),gForceStrength,bForceMode); break; }
			case 'k': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
			case 'j': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'l': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(+1,0,0),gForceStrength,bForceMode); break; }
			case 'u': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,+1),gForceStrength,bForceMode); break; }
			case 'm': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
#else
			case 'i': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,+1),gForceStrength,bForceMode); break; }
			case 'k': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
			case 'j': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(+1,0,0),gForceStrength,bForceMode); break; }
			case 'l': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'u': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,+1,0),gForceStrength,bForceMode); break; }
			case 'm': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
#endif  // Z_UP
		}
	}
}

void ProcessInputs()
{
    ProcessForceKeys();

    // Show debug wireframes
    if (bDebugWireframeMode) 
	{
		if (gScene)
		{
			gDebugRenderer.renderData(*gScene->getDebugRenderable());
		}
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

#ifdef Z_UP
		case 't': { gSelectedActor->setGlobalPosition(NxVec3(0,0,5)); break; }
#else
		case 't': { gSelectedActor->setGlobalPosition(NxVec3(0,5,0)); break; }
#endif  // Z_UP

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

#ifdef Z_UP
    gCameraRight.cross(gCameraForward,NxVec3(0,0,1));
    NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,0,1));
#else
    gCameraRight.cross(gCameraForward,NxVec3(0,1,0));
    NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));
#endif  // Z_UP

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
#ifdef Z_UP
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { -400.0f, 100.0f, 100.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
#else
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
#endif  // Z_UP

#ifdef Z_UP
	InitZUpPlaneDisplayList();
#endif  // Z_UP
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

	NxU32 set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname1[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	//if (set != 0) set = chdir(&fname[1]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	// Create the default material
	NxMaterialDesc	defaultMaterial;
	defaultMaterial.restitution		= 0;
	defaultMaterial.staticFriction	= 0.5;
	defaultMaterial.dynamicFriction	= 0.5;
	NxMaterial *mat = gScene->getMaterialFromIndex(0);
	mat->loadFromDesc(defaultMaterial);

	nxmlLoadScene("My_Physics_Scene.pml",gPhysicsSDK,gScene);
// 	nxmlLoadScene("start.pml",gPhysicsSDK,gScene);
//	nxmlLoadScene("catapult.pml",gPhysicsSDK,gScene);	
//	nxmlLoadScene("helloobjects.pml",gPhysicsSDK,gScene);
//	nxmlLoadScene("ragman.pml",gPhysicsSDK,gScene);

	AddUserDataToActors(gScene);

    NxActor** actors = gScene->getActors();
	gSelectedActor = actors[0];

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

void ReleaseNx()
{
	GetPhysicsResults();  // make sure to fetchResults() before shutting down
	if (gScene) gPhysicsSDK->releaseScene(*gScene);
	if (gPhysicsSDK)  gPhysicsSDK->release();
    NX_DELETE_SINGLE(gAllocator);
}

void ResetNx()
{
	ReleaseNx();
	InitNx();
}

void StartPhysics()
{
	// Update the time step
	gDeltaTime = UpdateTime();

	// Start collision and dynamics for delta time since the last frame
    gScene->simulate(gDeltaTime);
	gScene->flushStream();
}

void GetPhysicsResults()
{
	// Get results from gScene->simulate(gDeltaTime)
	while (!gScene->fetchResults(NX_RIGID_BODY_FINISHED, false));
}

int main(int argc, char** argv)
{
	PrintControls();
    InitGlut(argc, argv, "Lesson 502: Loading PML Files");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
