// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Cloth Sample
// Description: This sample shows an example of how to use the cloth API.
//
// Originally written by: Matthias Müller-Fischer (03-08-05)
//
// ===============================================================================

#include "SampleCloth.h"
#include "MyCloth.h"
#include "Timing.h"
#include "GLFontRenderer.h"

DebugRenderer gDebugRenderer;

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;
NxVec3            gDefaultGravity(0,-9.8,0);
bool              gHardwareSimulation = false;
bool              gTearLines = false;

// Time globals
NxReal gTime;
NxReal gLastTime;
 
// Display globals
int  gMainHandle;
char gDisplayString[512] = "";

// Camera globals
NxReal gCameraFov = 40.0f;
NxVec3 gCameraPos(0,5,15);
NxVec3 gCameraForward(0,0,-1);
NxVec3 gCameraRight(1,0,0);
const NxReal gCameraSpeed = 0.2;
int gViewWidth = 0;
int gViewHeight = 0;

// MouseGlobals
int mx = 0;
int my = 0;
NxDistanceJoint *gMouseJoint = NULL;
NxActor* gMouseSphere = NULL;
NxReal gMouseDepth = 0.0f;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Simulation globals
bool bPause = false;
bool bShadows = true;
bool bWireframeMode = false;
bool bGravity = false;

// Scene
int      gSceneNr = 1;
NxArray<MyCloth *> gCloths;
NxArray<NxActor *> gMetalCores;

NxActor *gSelectedActor = NULL;
NxActor *gGroundPlane = NULL;
NxCloth *gSelectedCloth = NULL;
int		 gSelectedClothVertex = -1;

// fps
int			gFrameCounter = 0;
float       gPreviousTime = getCurrentTime();
//Stopwatch	gStopWatch;

// ------------------------------------------------------------------------------------
void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, e = down\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n m = Single Step\n x = Toggle Shadows\n");
}

// ------------------------------------------------------------------------------------

void DisplayText()
{
	float y = 0.95f;
	int len = strlen(gDisplayString);
	len = (len < 256)?len:255;
	int start = 0;
	char textBuffer[256];
	for(int i=0;i<len;i++)
	{
		if(gDisplayString[i] == '\n' || i == len-1)
		{
			int offset = i;
			if(i == len-1) offset= i+1;
			memcpy(textBuffer, gDisplayString+start, offset-start);
			textBuffer[offset-start]=0;
			GLFontRenderer::print(0.01, y, 0.03f, textBuffer);
			y -= 0.035f;
			start = offset+1;
		}
	}
}

// ------------------------------------------------------------------------------------
void RefreshDisplayString(char *demoName)
{
#ifdef __CELLOS_LV2__
	sprintf(gDisplayString, "%s (%s/%s)\nUse 1-8 to choose scene\nUse right mouse button to drag objects\nUse 't' for tear lines, 'b' for bending\nPress space to shoot", 
		demoName, gHardwareSimulation ? "hardware" : "software", gTearLines ? "tear lines" : "full tear");
#else
	sprintf(gDisplayString, "%s (%s/%s)\nUse 1-8 to choose scene\nUse right mouse button to drag objects\nUse 'h' for hardware on/off, 't' for tear lines, 'b' for bending\nPress space to shoot", 
		demoName, gHardwareSimulation ? "hardware" : "software", gTearLines ? "tear lines" : "full tear");
#endif
}

// ------------------------------------------------------------------------------------
void ProcessKeys()
{
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
			case 'e':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed; break; }
		}
	}
}

// ------------------------------------------------------------------------------------
void SetupCamera()
{
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(gCameraFov, ((float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT)), 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


// ------------------------------------------------------------------------------------
void ViewProject(NxVec3 &v, int &xi, int &yi, float &depth)
{
//We cannot do picking easily on the xbox/PS3 anyway
#if defined(_XBOX) || defined (__CELLOS_LV2__)
	xi=yi=0;
	depth=0.0f;
#else
	GLint viewPort[4];
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	GLdouble winX, winY, winZ;
	gluProject((GLdouble) v.x, (GLdouble) v.y, (GLdouble) v.z,
		modelMatrix, projMatrix, viewPort, &winX, &winY, &winZ);
	xi = (int)winX; yi = viewPort[3] - (int)winY - 1; depth = (float)winZ;
#endif
}


// ------------------------------------------------------------------------------------
void ViewUnProject(int xi, int yi, float depth, NxVec3 &v)
{
//We cannot do picking easily on the xbox/PS3 anyway
#if defined(_XBOX) || defined (__CELLOS_LV2__)
	v=NxVec3(0.0f,0.0f,0.0f);
#else
	GLint viewPort[4];
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	yi = viewPort[3] - yi - 1;
	GLdouble wx, wy, wz;
	gluUnProject((GLdouble) xi, (GLdouble) yi, (GLdouble) depth,
	modelMatrix, projMatrix, viewPort, &wx, &wy, &wz);
	v.set((NxReal)wx, (NxReal)wy, (NxReal)wz);
#endif
}

// ------------------------------------------------------------------------------------
void RenderScene(bool shadows)
{
    // Render all the actors in the scene
	int nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		bool isMetalCore = false;
		for (NxActor **core = gMetalCores.begin(); core != gMetalCores.end(); core++)
			if (actor == *core) isMetalCore = true;

		if (bWireframeMode || isMetalCore)
			DrawWireActor(actor);
		else
			DrawActor(actor);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor);
        }
    }
    // Render all the cloths in the scene
	for (MyCloth **cloth = gCloths.begin(); cloth != gCloths.end(); cloth++){
		glColor4f(1.0f, 0.0f, 0.0f,1.0f);
		(*cloth)->draw(shadows);
	}
}

// ------------------------------------------------------------------------------------
void RenderCallback()
{
	if (gScene && !bPause)
        RunPhysics();

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessKeys();
	SetupCamera();

	RenderScene(bShadows);
	
	// gDebugRenderer.renderData(*(gScene->getDebugRenderable()));

	DisplayText();

    glutSwapBuffers();
}

// ------------------------------------------------------------------------------------
void ReshapeCallback(int width, int height)
{
	gViewWidth = width;
	gViewHeight = height;
    glViewport(0, 0, width, height);
}

// ------------------------------------------------------------------------------------
void IdleCallback()
{
    glutPostRedisplay();

	//gStopWatch.Stop();
	float time = getCurrentTime();
	float elapsedTime = time - gPreviousTime;
	
	if (elapsedTime > 1.0f) {
		char title[30];
		sprintf(title, "SampleCloth %3.1f fps", (float)gFrameCounter / elapsedTime);
		glutSetWindowTitle(title);
		gPreviousTime = time;
		gFrameCounter = 0;
		//gStopWatch.Reset();
	}

	getElapsedTime();
	//gStopWatch.Start();
}

// ------------------------------------------------------------------------------------
void KeyboardCallback(unsigned char key, int x, int y)
{
	gKeys[key] = true;
	if ('1' <= key && key <= '8') {
		gSceneNr = key - '0';
		ResetNx();
	}

	if (key == '0') {
		if(++gSceneNr == 9) gSceneNr = 1;
		ResetNx();
	}

	switch (key)
	{
	case 'o': {     
		NxCloth **cloths = gScene->getCloths();
		for (NxU32 i = 0; i < gScene->getNbCloths(); i++) {
			cloths[i]->setFlags(cloths[i]->getFlags() ^ NX_CLF_BENDING_ORTHO);
		}
		break;
			  }

	case 'G':
	case 'g': {     
		NxCloth **cloths = gScene->getCloths();
		for (NxU32 i = 0; i < gScene->getNbCloths(); i++) {
			cloths[i]->setFlags(cloths[i]->getFlags() ^ NX_CLF_GRAVITY);
		}
		break;
			  }

	case 'p': { bPause = !bPause; UpdateTime(); break; }
	case 'm': { RunPhysics(); glutPostRedisplay(); break; }
	case 'x': { bShadows = !bShadows; break; }
	case 'n': { bWireframeMode = !bWireframeMode; break; }		
	case 'b': {
		NxCloth **cloths = gScene->getCloths();
		for (NxU32 i = 0; i < gScene->getNbCloths(); i++) {
			cloths[i]->setFlags(cloths[i]->getFlags() ^ NX_CLF_BENDING);
		}
		break;
			  }
	case ' ': {
		NxActor *sphere = CreateSphere(gCameraPos, 1.0f, 1.0f);
		sphere->setLinearVelocity(gCameraForward * 20.0f);
		break; 
			  }
#ifndef __CELLOS_LV2__
	case 'h' : {
		gHardwareSimulation = !gHardwareSimulation;
		ResetNx();
		break;
			   }
#endif
	case 't' : {
		gTearLines = !gTearLines;
		ResetNx();
		break;
			   }
	case 27 : { exit(0); break; }
	default : { break; }
	}
}

// ------------------------------------------------------------------------------------
void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

}


// ------------------------------------------------------------------------------------
void LetGoActor()
{
	if (gMouseJoint) 
		gScene->releaseJoint(*gMouseJoint);
	gMouseJoint = NULL;
	if (gMouseSphere)
		gScene->releaseActor(*gMouseSphere);
	gMouseSphere = NULL;
}


// ------------------------------------------------------------------------------------
bool PickActor(int x, int y)
{
	LetGoActor();

	NxRay ray; 
	ViewUnProject(x,y,0.0f, ray.orig);
	ViewUnProject(x,y,1.0f, ray.dir);
	ray.dir -= ray.orig; ray.dir.normalize();

	NxRaycastHit hit;
	NxShape* closestShape = gScene->raycastClosestShape(ray, NX_ALL_SHAPES, hit);
	if (!closestShape || &closestShape->getActor() == gGroundPlane) return false;
	if (!closestShape->getActor().isDynamic()) return false;
	int hitx, hity;
	ViewProject(hit.worldImpact, hitx, hity, gMouseDepth);
	gMouseSphere = CreateSphere(hit.worldImpact, 0.1f, 1.0f);
	gMouseSphere->raiseBodyFlag(NX_BF_KINEMATIC);
	gMouseSphere->raiseActorFlag(NX_AF_DISABLE_COLLISION);
	NxDistanceJointDesc desc;
	gSelectedActor = &closestShape->getActor();
	desc.actor[0] = gMouseSphere;
	desc.actor[1] = gSelectedActor;
	gMouseSphere->getGlobalPose().multiplyByInverseRT(hit.worldImpact, desc.localAnchor[0]);
	gSelectedActor->getGlobalPose().multiplyByInverseRT(hit.worldImpact, desc.localAnchor[1]);
	desc.maxDistance = 0.0f;
	desc.minDistance = 0.0f;
	desc.spring.damper = 1.0f;
	desc.spring.spring = 200.0f;
	desc.flags |= NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_SPRING_ENABLED;
	NxJoint* joint = gScene->createJoint(desc);
	gMouseJoint = (NxDistanceJoint*)joint->is(NX_JOINT_DISTANCE);
	return true;
}

// ------------------------------------------------------------------------------------
void MoveActor(int x, int y)
{
	if (!gMouseSphere) return;
	NxVec3 pos;
	ViewUnProject(x,y, gMouseDepth, pos);
	gMouseSphere->setGlobalPosition(pos);
}


// ------------------------------------------------------------------------------------
void LetGoCloth()
{
	if (gSelectedCloth && gSelectedClothVertex >= 0)
		gSelectedCloth->freeVertex(gSelectedClothVertex);
	gSelectedClothVertex = -1;
}

// ------------------------------------------------------------------------------------
bool PickCloth(int x, int y)
{
	NxRay ray; 
	ViewUnProject(x,y,0.0f, ray.orig);
	ViewUnProject(x,y,1.0f, ray.dir);
	ray.dir -= ray.orig; ray.dir.normalize();

	NxVec3 hit;
	NxU32 vertexId;
	gSelectedClothVertex = -1;
	NxReal distance = NX_MAX_REAL;

	NxCloth **cloths = gScene->getCloths();
	for (NxU32 i = 0; i < gScene->getNbCloths(); i++) {
		if (cloths[i]->raycast(ray, hit, vertexId)) {
			if(hit.magnitude() < distance)
			{
				gSelectedCloth = cloths[i];
				gSelectedClothVertex = vertexId;
				int hitx, hity;
				ViewProject(hit, hitx, hity, gMouseDepth);
				distance = hit.magnitude();
			}
		}
	}

	return distance < NX_MAX_REAL;
}

// ------------------------------------------------------------------------------------
void MoveCloth(int x, int y)
{
	if (gSelectedCloth && gSelectedClothVertex >= 0) {
		NxVec3 pos; 
		ViewUnProject(x,y, gMouseDepth, pos);
		gSelectedCloth->attachVertexToGlobalPosition(gSelectedClothVertex, pos);
	}
}


// ------------------------------------------------------------------------------------
void SpecialCallback(int key, int x, int y)
{
	switch (key)
    {
		// Reset PhysX
		case GLUT_KEY_F10: ResetNx(); return; 
		case GLUT_KEY_F1 : 
			gMouseDepth += 0.02f * (1.0f - gMouseDepth); 
			MoveActor(mx,my); MoveCloth(mx,my);
			break;
		case GLUT_KEY_F2 : 
			gMouseDepth -= 0.02f * (1.0f - gMouseDepth);  
			if (gMouseDepth < 0.0f) gMouseDepth = 0.0f;
			MoveActor(mx,my); MoveCloth(mx,my);
			break;
	}
}


// ------------------------------------------------------------------------------------
void MouseCallback(int button, int state, int x, int y)
{
    mx = x;
	my = y;

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (!PickActor(x,y))
			PickCloth(x,y);
	}
	if (state == GLUT_UP) {
		LetGoActor();
		LetGoCloth();
	}
}

// ------------------------------------------------------------------------------------
void MotionCallback(int x, int y)
{
    int dx = mx - x;
    int dy = my - y;
    
	if (gMouseJoint)
		MoveActor(x,y);
	else if (gSelectedClothVertex >= 0) 
		MoveCloth(x,y);
	else {
		gCameraForward.normalize();
		gCameraRight.cross(gCameraForward,NxVec3(0,1,0));

		NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));
		qx.rotate(gCameraForward);
		NxQuat qy(NxPiF32 * dy * 20 / 180.0f, gCameraRight);
		qy.rotate(gCameraForward);
	}

    mx = x;
    my = y;
}

// ------------------------------------------------------------------------------------
void ExitCallback()
{
	if (gPhysicsSDK)
	{
		if (gScene) gPhysicsSDK->releaseScene(*gScene);
		gPhysicsSDK->release();
	}
}

// ------------------------------------------------------------------------------------
void InitGlut(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("Cloth Sample");
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
	glClearColor(0.52f, 0.60f, 0.71f, 1.0f);  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
#ifdef WIN32
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
#endif

    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.8f, 0.8f, 0.8f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.7f, 0.7f, 0.7f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
//    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    float Position[]        = { 10.0f, 10.0f, 10.0f, 0.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

// ------------------------------------------------------------------------------------
void SetupCurtainScene()
{
	RefreshDisplayString("Cloth and Shape Interaction Sample");

	// Create the objects in the scene
	gGroundPlane = CreateGroundPlane();
	NxActor *sphere1 = CreateSphere(NxVec3(-1,0,-0.5), 1, 10);
	NxActor *box1 = CreateBox(NxVec3(1,0,-1), NxVec3(1,1,1), 10);
	NxActor *box2 = CreateBox(NxVec3(1,6.5,0), NxVec3(5,0.5,0.5), 10); 
	box2->setLinearDamping(5.0);

	NxD6JointDesc d6Desc;
	d6Desc.actor[0] = NULL;
	d6Desc.actor[1] = box2;
	NxVec3 globalAnchor(0,7,0);
	d6Desc.localAnchor[0] = globalAnchor;
	box2->getGlobalPose().multiplyByInverseRT(globalAnchor, d6Desc.localAnchor[1]);

	d6Desc.localAxis[0]   = NxVec3(1,0,0);
	d6Desc.localNormal[0] = NxVec3(0,1,0);
	d6Desc.localAxis[1]   = NxVec3(1,0,0);
	d6Desc.localNormal[1] = NxVec3(0,1,0);

	d6Desc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.xMotion = NX_D6JOINT_MOTION_FREE;
	d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
	d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;

	NxJoint* d6Joint = gScene->createJoint(d6Desc);

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(-3,7,0);
	clothDesc.thickness = 0.2f;
	//clothDesc.density = 1.0f;
	clothDesc.bendingStiffness = 0.5f;
	clothDesc.stretchingStiffness = 1.0f;
	//clothDesc.dampingCoefficient = 0.50f;
	clothDesc.friction = 0.5f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	//clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth *regularCloth = new MyCloth(gScene, clothDesc, 8.0f, 7.0f, 0.15f, "ageia.bmp");
	gCloths.push_back(regularCloth);

	regularCloth->getNxCloth()->attachToShape(*box2->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	gCameraPos.set(1.43f,10.96f,17.9f);
	gCameraForward.set(0.0f,-0.29f,-0.96f);
	gCameraForward.normalize();
}

// ------------------------------------------------------------------------------------
void SetupFlagScene()
{
	RefreshDisplayString("External acceleration demo");

	// Create the objects in the scene
	gGroundPlane = CreateGroundPlane();

	NxActor *capsule = CreateCapsule(NxVec3(0.0,-0.2,0.0),12.0f, 0.2f, 0.0f); 

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(0.0f,12.0f,0.0f);
	clothDesc.globalPose.M.rotX(NxHalfPiF32); 
	clothDesc.thickness = 0.2f;
	//clothDesc.density = 1.0f;
	//clothDesc.bendingStiffness = 1.0f;
	//clothDesc.stretchingStiffness = 1.0f;
	//clothDesc.dampingCoefficient = 0.50f;
	clothDesc.friction = 0.5f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
//	clothDesc.flags &= ~NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	//clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth *regularCloth = new MyCloth(gScene, clothDesc, 8.0f, 8.0f, 0.15f, "ageia.bmp");
	gCloths.push_back(regularCloth);

	regularCloth->getNxCloth()->attachToShape(*capsule->getShapes(), 0);

	gCameraPos.set(1.43f,10.96f,17.9f);
	gCameraForward.set(0.0f,-0.29f,-0.96f);
	gCameraForward.normalize();
}

// ------------------------------------------------------------------------------------

void SetupDampingScene()
{
	RefreshDisplayString("Damping Demo: Undamped, normally damped, COM damped");

	// Create objects in scene
	gGroundPlane = CreateGroundPlane();
	
	NxActor *box1 = CreateBox(NxVec3(6,6.5,0), NxVec3(12,0.5,0.5), 0); 

	NxActor *box2 = CreateBox(NxVec3(-2,0,1), NxVec3(1,1,1), 10);
	NxActor *box3 = CreateBox(NxVec3(6,0,1), NxVec3(1,1,1), 10);
	NxActor *box4 = CreateBox(NxVec3(14,0,1), NxVec3(1,1,1), 10);

	// First Cloth

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(-6,7,0);
	clothDesc.thickness = 0.2f;
	//clothDesc.density = 1.0f;
	//clothDesc.bendingStiffness = 1.0f;
	//clothDesc.stretchingStiffness = 1.0f;
	//clothDesc.dampingCoefficient = 0.50f;
	clothDesc.friction = 0.5f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	//clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth *regularCloth1 = new MyCloth(gScene, clothDesc, 8.0f, 7.0f, 0.2f, "ageia.bmp");
	gCloths.push_back(regularCloth1);
	regularCloth1->getNxCloth()->attachToShape(*box1->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	// Second cloth
	clothDesc.globalPose.t = NxVec3(2,7,0);
	clothDesc.flags |= NX_CLF_DAMPING;
	clothDesc.dampingCoefficient = 1.0f;

	MyCloth *regularCloth2 = new MyCloth(gScene, clothDesc, 8.0f, 7.0f, 0.2f, "ageia.bmp");
	gCloths.push_back(regularCloth2);
	regularCloth2->getNxCloth()->attachToShape(*box1->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	// Third cloth
	clothDesc.globalPose.t = NxVec3(10,7,0);
	clothDesc.flags |= NX_CLF_DAMPING;
	clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.dampingCoefficient = 1.0f;

	MyCloth *regularCloth3 = new MyCloth(gScene, clothDesc, 8.0f, 7.0f, 0.2f, "ageia.bmp");
	gCloths.push_back(regularCloth3);
	regularCloth3->getNxCloth()->attachToShape(*box1->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	gCameraPos.set(6.0f,5.8f,26.0f);
	gCameraForward.set(0.0f,0.0f,-1.0f);
	gCameraForward.normalize();

}
// ------------------------------------------------------------------------------------

void SetupPipeScene()
{
	RefreshDisplayString("Two Way Body Cloth Interaction Demo");

	// Create objects in scene
	gGroundPlane = CreateGroundPlane();
	NxActor *box1 = CreateBox(NxVec3(0,10.5,-1), NxVec3(1.0f,1.0f,0.1f), 0.0f);
	NxActor *box2 = CreateBox(NxVec3(0,13,1.5), NxVec3(2.0f,2.0f,0.5f), 0.0f);
	NxActor *box3 = CreateBox(NxVec3(0,15.4,-1), NxVec3(1.0f,0.1f,1.0f), 0.0f);
	NxActor *box4 = CreateBox(NxVec3(0,19,-11), NxVec3(1.0f,0.1f,1.0f), 0.0f);
	NxActor *box5 = CreateBox(NxVec3(0,17.5,-13), NxVec3(3.0f,0.1f,2.0f), 0.0f);

	NxActor *box6 = CreateBox(NxVec3(0,20,-12), NxVec3(1.0f,5.0f,0.1f), 0.0f);
	NxActor *box7 = CreateBox(NxVec3(0,18.5,-14), NxVec3(2.0f,5.0f,0.1f), 0.0f);
	NxActor *box8 = CreateBox(NxVec3(0,10.5,1.0), NxVec3(1.0f,1.0f,0.1f), 0.0f);

	for (NxU32 i = 0; i < 16; i++)
		NxActor *sphere = CreateSphere( NxVec3(0,22.0f+4.0f*i,-13),0.7,1.0f);

	NxQuat q;
	q.fromAngleAxis(20.0f,NxVec3(1.0f,0.0f,0.0f));
	box3->setGlobalOrientationQuat(q);

	q.fromAngleAxis(20.0f,NxVec3(1.0f,0.0f,0.0f));
	box4->setGlobalOrientationQuat(q);

	q.fromAngleAxis(20.0f,NxVec3(1.0f,0.0f,0.0f));
	box5->setGlobalOrientationQuat(q);

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(0,18.2,-11);
	clothDesc.globalPose.M.rotX(0.35f);
	clothDesc.thickness = 0.4f;	
	clothDesc.density = 0.05f;
	clothDesc.bendingStiffness = 1.0f;
	clothDesc.stretchingStiffness = 1.0f;
	clothDesc.dampingCoefficient = 0.10f;
	clothDesc.friction = 0.3f;
	clothDesc.collisionResponseCoefficient = 0.4f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth *objCloth1 = new MyCloth(gScene, clothDesc, "pipe.obj", 1.0f);
	objCloth1->getNxCloth()->attachToCollidingShapes(0);
	gCloths.push_back(objCloth1);

	clothDesc.globalPose.t = NxVec3(0,12.2,0);
	clothDesc.globalPose.M.rotX(NxHalfPiF32);

	MyCloth *objCloth2 = new MyCloth(gScene, clothDesc, "pipe.obj", 1.0f);
	objCloth2->getNxCloth()->attachToCollidingShapes(0);
	gCloths.push_back(objCloth2);

	gCameraPos.set(29.4f,12.93f,9.79f);
	gCameraForward.set(-0.88f,-0.11f,-0.43f);
	gCameraForward.normalize();

}
// ------------------------------------------------------------------------------------

void SetupAttachmentScene()
{
	RefreshDisplayString("Attachment Demo");

	// Create objects in scene
	gGroundPlane = CreateGroundPlane();
	NxActor *box1 = CreateBox(NxVec3(-7,12.25,0), NxVec3(2.5f,1.0f,1.0f), 0.0f);
	NxActor *box2 = CreateBox(NxVec3(0,12.25,0), NxVec3(2.5f,1.0f,1.0f), 0.0f);
	NxActor *box3 = CreateBox(NxVec3(7,12.25,0), NxVec3(2.5f,1.0f,1.0f), 0.0f);	

	NxActor *attachedBox = CreateBox(NxVec3(-6.25,3.5,2.0), NxVec3(1.25f,1.0f,1.0f), 0.5f);
	NxActor *attachedSphere = CreateSphere(NxVec3(0.0,3.5,2.0), 1.3f, 0.5f);
	NxActor *attachedCapsule = CreateCapsule(NxVec3(9.0,4.5,2.0),2.0f, 1.0f, 1.0f); 

	NxReal damping = 0.3f;
	attachedBox->setAngularDamping(damping);
	attachedBox->setLinearDamping(damping);
	attachedSphere->setAngularDamping(damping);
	attachedSphere->setLinearDamping(damping);
	attachedCapsule->setAngularDamping(damping);
	attachedCapsule->setLinearDamping(damping);

	NxQuat q;
	q.fromAngleAxis(90,NxVec3(0.0f,0.0f,1.0f));
	attachedCapsule->setGlobalOrientationQuat(q);

	NxClothDesc clothDesc;
	clothDesc.globalPose.M.rotX(1.3);
	clothDesc.thickness = 0.3f;	
	clothDesc.density = 1.0f;
	clothDesc.bendingStiffness = 1.0f;
	clothDesc.stretchingStiffness = 1.0f;
	clothDesc.dampingCoefficient = 0.5f;
	clothDesc.friction = 0.5f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	clothDesc.attachmentResponseCoefficient = 1.0f;
	clothDesc.attachmentTearFactor = 2.0f;
	clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COMDAMPING;
	//clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	clothDesc.globalPose.t = NxVec3(-7.2,13.0,0);
	MyCloth *regularCloth1 = new MyCloth(gScene, clothDesc, 2.0f, 8.0f, 0.4f);
	regularCloth1->getNxCloth()->attachToShape(box1->getShapes()[0], NX_CLOTH_ATTACHMENT_TWOWAY | NX_CLOTH_ATTACHMENT_TEARABLE);
	regularCloth1->getNxCloth()->attachToShape(attachedBox->getShapes()[0], NX_CLOTH_ATTACHMENT_TWOWAY);
	gCloths.push_back(regularCloth1);

	clothDesc.globalPose.t = NxVec3(-1.0,13.0,0);
	MyCloth *regularCloth2 = new MyCloth(gScene, clothDesc, 2.0f, 8.0f, 0.4f);
	regularCloth2->getNxCloth()->attachToShape(box2->getShapes()[0], NX_CLOTH_ATTACHMENT_TWOWAY | NX_CLOTH_ATTACHMENT_TEARABLE);
	regularCloth2->getNxCloth()->attachToShape(attachedSphere->getShapes()[0], NX_CLOTH_ATTACHMENT_TWOWAY);
	gCloths.push_back(regularCloth2);

	clothDesc.globalPose.t = NxVec3(6.1,13.0,0);
	MyCloth *regularCloth3 = new MyCloth(gScene, clothDesc, 2.0f, 8.0f, 0.4f);
	regularCloth3->getNxCloth()->attachToCollidingShapes(NX_CLOTH_ATTACHMENT_TWOWAY);
	gCloths.push_back(regularCloth3);

	gCameraPos.set(3.25f,13.37f,26.62f);
	gCameraForward.set(-0.07f,-0.21f,-0.96f);
	gCameraForward.normalize();

}
// ------------------------------------------------------------------------------------
void SetupTearingScene()
{
	RefreshDisplayString("Tearing Demo");

	// Create the objects in the scene
	gGroundPlane = CreateGroundPlane();
	NxActor *box1 = CreateBox(NxVec3(1,10,0), NxVec3(3,0.5,0.5), 0);
	NxActor *box2 = CreateBox(gTearLines ? NxVec3(3.3,2,0) : NxVec3(3.3,6,0), NxVec3(0.5,0.5,0.5), 10); 

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(-1.5,10,0);
	clothDesc.globalPose.M.rotX(NxHalfPiF32);
	clothDesc.thickness = 0.1f;
	//clothDesc.density = 1.0f;
	//clothDesc.bendingStiffness = 1.0f;
	//clothDesc.stretchingStiffness = 1.0f;
	//clothDesc.dampingCoefficient = 0.50f;
	clothDesc.friction = 0.5f;
	//clothDesc.pressure = 0.0f;
	clothDesc.tearFactor = 2.2f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_PRESSURE;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_SELFCOLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	//clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;
	clothDesc.flags |= NX_CLF_TEARABLE;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth *regularCloth = new MyCloth(gScene, clothDesc, 5.0f, 8.0f, 0.1f, "rug512.bmp", gTearLines);
	gCloths.push_back(regularCloth);

	regularCloth->getNxCloth()->attachToShape(*box1->getShapes(), 0);
	regularCloth->getNxCloth()->attachToShape(*box2->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	gCameraPos.set(1.43f,10.96f,17.9f);
	gCameraForward.set(0.0f,-0.29f,-0.96f);
	gCameraForward.normalize();
}

// ------------------------------------------------------------------------------------
void SetupPressureScene()
{
	RefreshDisplayString("Pressure Demo");

	// Create the objects in the scene
	gGroundPlane = CreateGroundPlane();
	NxActor *box1 = CreateBox(NxVec3(0,0,-5), NxVec3(10,5,0.5), 0);
	NxActor *box2 = CreateBox(NxVec3(-10,0,0), NxVec3(0.5,5,5), 0); 
	NxActor *box3 = CreateBox(NxVec3( 10,0,0), NxVec3(0.5,5,5), 0); 

	NxReal r = 1.0;
	NxReal d = 1.0;
	CreateSphere(NxVec3(0,20,0), r, d);
	CreateSphere(NxVec3(-3,35, 0), r, d);
	CreateSphere(NxVec3(3,50, 0), r, d);
	CreateSphere(NxVec3(0,40, 0), r, d);
	CreateSphere(NxVec3(-3,55, 0), r, d);
	CreateSphere(NxVec3(3,60, 0), r, d);
	CreateSphere(NxVec3(0,60, 0), r, d);
	CreateSphere(NxVec3(-3,75, 0), r, d);
	CreateSphere(NxVec3(3,80, 0), r, d);

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(0.3,6,0);
	clothDesc.thickness = 0.1f;
	//clothDesc.density = 1.0f;
	//clothDesc.bendingStiffness = 1.0f;
	//clothDesc.stretchingStiffness = 1.0f;
	//clothDesc.dampingCoefficient = 0.50f;
	clothDesc.friction = 0.5f;
	clothDesc.pressure = 2.0f;
	//clothDesc.tearFactor = 2.2f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	clothDesc.flags |= NX_CLF_PRESSURE;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_SELFCOLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	//clothDesc.flags |= NX_CLF_DAMPING;
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;
	//clothDesc.flags |= NX_CLF_TEARABLE;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth *objCloth = new MyCloth(gScene, clothDesc, "cow.obj", 1.0f);
	gCloths.push_back(objCloth);

	gCameraPos.set(-4.98f,17.8f,25.6f);
	gCameraForward.set(0.17f,-0.47f,-0.87f);
	gCameraForward.normalize();
}


// ------------------------------------------------------------------------------------
void CreateMetalCloth(const NxVec3 &position, int mode, NxClothDesc &clothDesc, char *meshName)
{
	// global metal data
	NxReal velocityThreshold = 2.0f;
	NxReal penetrationDepth  = 0.5f;
	NxActorDesc coreActorDesc;
	NxBodyDesc  coreBodyDesc;
	coreBodyDesc.mass = 10.0f;
	coreBodyDesc.linearDamping = 0.2f;
	coreBodyDesc.angularDamping = 0.2f;
	coreActorDesc.body = &coreBodyDesc;

	// create the shape, no need for size info
	// because it is automatically generated by the cloth
	if (mode == 0) { // sphere as core
		NxSphereShapeDesc sphereDesc;
		coreActorDesc.shapes.pushBack(&sphereDesc);
	}
	else if (mode == 1) { // capsule as core
		NxCapsuleShapeDesc capsuleDesc;
		coreActorDesc.shapes.pushBack(&capsuleDesc);
	}
	else if (mode == 2) { // box as core
		NxBoxShapeDesc boxDesc;
		coreActorDesc.shapes.pushBack(&boxDesc);
	}
	else if (mode == 3) { // compound of spheres as core
		const NxU32 numSpheres = 10;
		NxSphereShapeDesc compoundSphereDesc[numSpheres];
		for (NxU32 i = 0; i < numSpheres; i++) 
			coreActorDesc.shapes.pushBack(&compoundSphereDesc[i]);
	}
	else return;

	coreActorDesc.globalPose.t = position;
	NxActor *coreActor = gScene->createActor(coreActorDesc);
	gMetalCores.push_back(coreActor);

	MyCloth *objCloth = new MyCloth(gScene, clothDesc, meshName, 1.0f);
	gCloths.push_back(objCloth);
	objCloth->getNxCloth()->attachToCore(coreActor, velocityThreshold, penetrationDepth); 
}

// ------------------------------------------------------------------------------------
void SetupMetalScene()
{
	RefreshDisplayString("Metal Demo");

	// Create the objects in the scene
	gGroundPlane = CreateGroundPlane();
	NxActor *box1 = CreateBox(NxVec3(0,0,-10), NxVec3(20,10,0.5), 0);
	NxActor *box2 = CreateBox(NxVec3(-20,0,0), NxVec3(0.5,10,10), 0); 
	NxActor *box3 = CreateBox(NxVec3( 20,0,0), NxVec3(0.5,10,10), 0); 

	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(0.3,6,0);
	clothDesc.thickness = 0.1f;
	//clothDesc.density = 1.0f;
	//clothDesc.bendingStiffness = 1.0f;
	//clothDesc.stretchingStiffness = 1.0f;
	//clothDesc.dampingCoefficient = 0.50f;
	clothDesc.friction = 0.5f;
	//clothDesc.pressure = 2.0f;
	//clothDesc.tearFactor = 2.2f;
	//clothDesc.collisionResponseCoefficient = 1.0f;
	//clothDesc.attachmentResponseCoefficient = 1.0f;
	//clothDesc.solverIterations = 5;
	//clothDesc.flags |= NX_CLF_PRESSURE;
	//clothDesc.flags |= NX_CLF_STATIC;
	//clothDesc.flags |= NX_CLF_DISABLE_COLLISION;
	//clothDesc.flags |= NX_CLF_SELFCOLLISION;
	//clothDesc.flags |= NX_CLF_VISUALIZATION;
	//clothDesc.flags |= NX_CLF_GRAVITY;
	clothDesc.flags |= NX_CLF_BENDING;
	//clothDesc.flags |= NX_CLF_BENDING_ORTHO;
	//clothDesc.flags |= NX_CLF_DAMPING;
	//clothDesc.flags |= NX_CLF_COLLISION_TWOWAY;
	//clothDesc.flags |= NX_CLF_TEARABLE;

	if (gHardwareSimulation)
		clothDesc.flags |= NX_CLF_HARDWARE;

	CreateMetalCloth(NxVec3(  0.0f,  3.0f, 0.5f), 0, clothDesc, "barrel.obj");
	CreateMetalCloth(NxVec3(  0.5f, 20.0f, 0.0f), 1, clothDesc, "barrel.obj");
	CreateMetalCloth(NxVec3(  0.0f, 40.0f, 0.5f), 2, clothDesc, "barrel.obj");
	CreateMetalCloth(NxVec3(  0.5f, 60.0f, 0.0f), 3, clothDesc, "barrel.obj");

	gCameraPos.set(0.0f, 25.0f, 50.0f);
	gCameraForward.set(0.0f,-0.30f,-0.90f);
	gCameraForward.normalize();
}

// ------------------------------------------------------------------------------------
void InitNx()
{
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
    if (!gPhysicsSDK)  return;

	NxInitCooking();
	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.005f);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
//	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity;
    gScene = gPhysicsSDK->createScene(sceneDesc);

	// Create the default material
	NxMaterialDesc       m; 
	m.restitution        = 0.5f;
	m.staticFriction     = 0.2f;
	m.dynamicFriction    = 0.2f;
	NxMaterial * mat = gScene->getMaterialFromIndex(0);
	mat->loadFromDesc(m); 

	switch(gSceneNr) {
		case 1 : SetupCurtainScene(); break;
		case 2 : SetupFlagScene(); break;
		case 3 : SetupDampingScene(); break;
		case 4 : SetupPipeScene(); break;
		case 5 : SetupAttachmentScene(); break;
		case 6 : SetupTearingScene(); break;
		case 7 : SetupPressureScene(); break;
		case 8 : SetupMetalScene(); break;
	}

	if (gCloths.size() > 0)
		gSelectedCloth = gCloths[0]->getNxCloth();
	else
		gSelectedCloth = NULL;

	if (gScene->getNbActors() > 0)
		gSelectedActor = *gScene->getActors();
	else
		gSelectedActor = NULL;

	UpdateTime();

#if defined(_XBOX) | defined(__CELLOS_LV2__)
	glutRemapButtonExt(1, 'b', false);//Bending
	glutRemapButtonExt(1, 'o', true);//Orthogonal bending
	glutRemapButtonExt(2, 'G', false);//Gravity
	glutRemapButtonExt(2, 't', true);//Tear lines
	glutRemapButtonExt(3, 'x', false);//Shadows
	glutRemapButtonExt(3, 'n', true);//Debug wireframe
#endif
}


// ------------------------------------------------------------------------------------
void ReleaseNx()
{
	for (MyCloth **cloth = gCloths.begin(); cloth != gCloths.end(); cloth++)
		delete *cloth;
	gCloths.clear();

	gMetalCores.clear();

	if (gScene) gPhysicsSDK->releaseScene(*gScene);
	NxCloseCooking();
	if (gPhysicsSDK)  gPhysicsSDK->release();
}

// ------------------------------------------------------------------------------------
void ResetNx()
{
	LetGoActor();
	LetGoCloth();
	ReleaseNx();
	InitNx();
}

// ------------------------------------------------------------------------------------
NxReal UpdateTime()
{
    NxReal deltaTime;
    gTime = timeGetTime()*0.001f;  // Get current time in seconds
    deltaTime = gTime - gLastTime;
    gLastTime = gTime;
    return deltaTime;
}

// ------------------------------------------------------------------------------------
void RunPhysics()
{
	// Update the time step
	NxReal deltaTime = UpdateTime();

	deltaTime = 0.02f;	// todo: may go back to real time at some point

	// Run collision and dynamics for delta time since the last frame
	gScene->simulate(deltaTime);	
	gScene->flushStream();
	gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);

	if (gSceneNr == 2 && gScene->getNbCloths() == 1) {
		NxCloth **cloths = gScene->getCloths();
		NxReal range = 10.0f;
		cloths[0]->setExternalAcceleration(NxVec3(
			20.0f + NxMath::rand(-range, range),
			NxMath::rand(-range, range),
			NxMath::rand(-range, range)
		));
	}

	gFrameCounter++;
}

// ------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	PrintControls();
    InitGlut(argc, argv);
    InitNx();
    glutMainLoop();
	ReleaseNx();
}
