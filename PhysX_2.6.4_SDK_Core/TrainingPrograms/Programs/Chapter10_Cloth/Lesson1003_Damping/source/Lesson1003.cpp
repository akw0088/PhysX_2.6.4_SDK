// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                     LESSON 1003: DAMPING
//
//                     Written by Matthias Müller-Fischer, 3-8-05
//						     Edited by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#include "Lesson1003.h"
#include "UpdateTime.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;
NxVec3            gDefaultGravity(0,-9.8,0);

// User report globals
DebugRenderer     gDebugRenderer;
UserAllocator*	  gAllocator;
ErrorStream       gErrorStream;

// HUD globals
HUD hud;
char gTitleString[512] = "";

// Display globals
int gMainHandle;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,5,-25);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
const NxReal gCameraSpeed = 10;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 80000;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// MouseGlobals
int mx = 0;
int my = 0;
NxDistanceJoint* gMouseJoint = NULL;
NxActor* gMouseSphere = NULL;
NxReal gMouseDepth = 0;

// Simulation globals
NxReal gDeltaTime = 1.0/60.0;
bool bHardwareScene = false;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

// Actor globals
NxActor* groundPlane = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

// Selected cloth and cloth vertex
NxCloth* gSelectedCloth = NULL;
int		 gSelectedClothVertex = -1;

// Array of cloth objects
NxArray<MyCloth*> gCloths;

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/wavefront";
char fname2[] = "../../TrainingPrograms/Programs/Data/wavefront";
char fname3[] = "TrainingPrograms/Programs/Data/wavefront";

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n Space = Shoot a sphere\n");
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
			case 'z':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed*deltaTime; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed*deltaTime; break; }
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

// ------------------------------------------------------------------------------------
void ViewProject(NxVec3 &v, int &xi, int &yi, float &depth)
{
//We cannot do picking easily on the xbox/PS3 anyway
#if defined(_XBOX)||defined(__CELLOS_LV2__)
	xi=yi=0;
	depth=0;
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
#if defined(_XBOX)||defined(__CELLOS_LV2__)
	v=NxVec3(0,0,0);
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
			DrawActorShadow(actor, false);
        }
    }
}

void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color)
{
	// Draw only if the force is large enough
	NxReal force = forceVec.magnitude();
	if (force < 0.1)  return;

	forceVec = 3*forceVec/force;

	NxVec3 pos;
	if(actor->isDynamic())
	{
		pos= actor->getCMassGlobalPosition();
	}
	else
	{
		pos= actor->getGlobalPosition();
	}
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
			case 'i': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,1),gForceStrength,bForceMode); break; }
			case 'k': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
			case 'j': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(1,0,0),gForceStrength,bForceMode); break; }
			case 'l': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'u': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,1,0),gForceStrength,bForceMode); break; }
			case 'm': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
		    
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
	    // Update the time step
//	    gDeltaTime = UpdateTime();
	    gDeltaTime = 1.0/60.0;

		GetPhysicsResults();
        ProcessInputs();
		StartPhysics();
	}

    // Display first scene
 	RenderActors(bShadows);

    // Render all the cloths in the scene
	for (MyCloth** cloth = gCloths.begin(); cloth != gCloths.end(); cloth++)
	{
		glColor4f(1.0f, 0.0f, 0.0f,1.0f);
		(*cloth)->draw(bShadows);
	}

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

		case 'o': 
		{     
			NxCloth** cloths = gScene->getCloths();
			for (NxU32 i = 0; i < gScene->getNbCloths(); i++) 
			{
				cloths[i]->setFlags(cloths[i]->getFlags() ^ NX_CLF_BENDING_ORTHO);
			}
			break;
		}

		case 'g': 
		{     
			NxCloth** cloths = gScene->getCloths();
			for (NxU32 i = 0; i < gScene->getNbCloths(); i++) 
			{
				cloths[i]->setFlags(cloths[i]->getFlags() ^ NX_CLF_GRAVITY);
			}
			break;
		}

//	    case 'p': { bPause = !bPause; UpdateTime(); break; }
// 		case 'm': { RunPhysics(); glutPostRedisplay(); break; }
//		case 'x': { bShadows = !bShadows; break; }
//		case 'n': { bDebugWireframeMode = !bDebugWireframeMode; break; }	

		case 'y': 
		{
			NxCloth** cloths = gScene->getCloths();
			for (NxU32 i = 0; i < gScene->getNbCloths(); i++) 
			{
				cloths[i]->setFlags(cloths[i]->getFlags() ^ NX_CLF_BENDING);
			}	            
			break;
		}
		case ' ': 
		{
			// you can't add actors to the scene 
			// while the simulation loop is running.
			// wait until the results are available
			gScene->checkResults(NX_RIGID_BODY_FINISHED,true);
			NxActor *sphere = CreateSphere(gCameraPos, 1, 1);
			sphere->setLinearVelocity(gCameraForward*20);
			break; 
		}
		case 27 : { exit(0); break; }
		default : { break; }
	}
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
	ViewUnProject(x,y,0, ray.orig);
	ViewUnProject(x,y,1, ray.dir);
	ray.dir -= ray.orig; ray.dir.normalize();

	NxRaycastHit hit;
	NxShape* closestShape = gScene->raycastClosestShape(ray, NX_ALL_SHAPES, hit);
	if (!closestShape || &closestShape->getActor() == groundPlane) return false;
	if (!closestShape->getActor().isDynamic()) return false;
	int hitx, hity;
	ViewProject(hit.worldImpact, hitx, hity, gMouseDepth);
	gMouseSphere = CreateSphere(hit.worldImpact, 0.1, 1);
	if(gMouseSphere)
	{
		gMouseSphere->raiseBodyFlag(NX_BF_KINEMATIC);
		gMouseSphere->raiseActorFlag(NX_AF_DISABLE_COLLISION);
		NxDistanceJointDesc desc;
		gSelectedActor = &closestShape->getActor();
		desc.actor[0] = gMouseSphere;
		desc.actor[1] = gSelectedActor;
		gMouseSphere->getGlobalPose().multiplyByInverseRT(hit.worldImpact, desc.localAnchor[0]);
		gSelectedActor->getGlobalPose().multiplyByInverseRT(hit.worldImpact, desc.localAnchor[1]);
		desc.maxDistance = 0;
		desc.minDistance = 0;
		desc.spring.damper = 1;
		desc.spring.spring = 200;
		desc.flags |= NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_SPRING_ENABLED;
		NxJoint* joint = gScene->createJoint(desc);
		gMouseJoint = (NxDistanceJoint*)joint->is(NX_JOINT_DISTANCE);
		return true;
	}
	return false;
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
	ViewUnProject(x,y,0, ray.orig);
	ViewUnProject(x,y,1, ray.dir);
	ray.dir -= ray.orig; ray.dir.normalize();

	NxVec3 hit;
	NxU32 vertexId;
	gSelectedClothVertex = -1;
	NxReal distance = NX_MAX_REAL;

	NxCloth** cloths = gScene->getCloths();
	for (NxU32 i = 0; i < gScene->getNbCloths(); i++) 
	{
		if (cloths[i]->raycast(ray, hit, vertexId)) 
		{
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
	if (gSelectedCloth && gSelectedClothVertex >= 0) 
	{
		NxVec3 pos; 
		ViewUnProject(x,y, gMouseDepth, pos);
		gSelectedCloth->attachVertexToGlobalPosition(gSelectedClothVertex, pos);
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

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
	{
		if (!PickActor(x,y))
			PickCloth(x,y);
	}
	if (state == GLUT_UP) 
	{
		LetGoActor();
		LetGoCloth();
	}
}

void MotionCallback(int x, int y)
{
    int dx = mx - x;
    int dy = my - y;

	if (gMouseJoint)
	{
		MoveActor(x,y);
	}
	else if (gSelectedClothVertex >= 0) 
	{
		MoveCloth(x,y);
	}
	else 
	{   
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

void SetupDampingScene()
{
    sprintf(gTitleString, "Damping Demo: Undamped, normally damped, COM damped");

	// Create objects in scene
	groundPlane = CreateGroundPlane();

	NxActor* bar = CreateBox(NxVec3(0,6.5,0), NxVec3(12,0.5,0.5), 0); 
	NxActor* box1 = CreateBox(NxVec3(-8,0,1), NxVec3(1,1,1), 10);
	NxActor* box2 = CreateBox(NxVec3(0,0,1), NxVec3(1,1,1), 10);
	NxActor* box3 = CreateBox(NxVec3(8,0,1), NxVec3(1,1,1), 10);

	// First Cloth
	NxClothDesc clothDesc;
	clothDesc.globalPose.t = NxVec3(12,7,0);
	clothDesc.thickness = 0.2;
	//clothDesc.density = 1;
	//clothDesc.bendingStiffness = 1;
	//clothDesc.stretchingStiffness = 1;
	//clothDesc.dampingCoefficient = 0.5;
	clothDesc.friction = 0.5;
	//clothDesc.collisionResponseCoefficient = 1;
	//clothDesc.attachmentResponseCoefficient = 1;
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

	clothDesc.flags |= NX_CLF_HARDWARE;

	MyCloth* regularCloth1 = new MyCloth(gScene, clothDesc, 8, 7, 0.2, "ageia.bmp");
	gCloths.push_back(regularCloth1);
	regularCloth1->getNxCloth()->attachToShape(*bar->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	// Second cloth
	clothDesc.globalPose.t = NxVec3(4,7,0);
	clothDesc.flags |= NX_CLF_DAMPING;
	clothDesc.dampingCoefficient = 1;

	MyCloth* regularCloth2 = new MyCloth(gScene, clothDesc, 8, 7, 0.2, "ageia.bmp");
	gCloths.push_back(regularCloth2);
	regularCloth2->getNxCloth()->attachToShape(*bar->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);

	// Third cloth
	clothDesc.globalPose.t = NxVec3(-4,7,0);
	clothDesc.flags |= NX_CLF_DAMPING;
	clothDesc.flags |= NX_CLF_COMDAMPING;
	clothDesc.dampingCoefficient = 1;
	
	MyCloth* regularCloth3 = new MyCloth(gScene, clothDesc, 8, 7, 0.2, "ageia.bmp");
	gCloths.push_back(regularCloth3);
	regularCloth3->getNxCloth()->attachToShape(*bar->getShapes(), NX_CLOTH_ATTACHMENT_TWOWAY);
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

void InitializeSpecialHUD()
{
	char ds[512];

	// Add lesson title string to HUD
	sprintf(ds, gTitleString);
	hud.AddDisplayString(ds, 0.015f, 0.92f);
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
	if (!gPhysicsSDK)  return;

	NxInitCooking();

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity = gDefaultGravity;
    sceneDesc.simType = NX_SIMULATION_HW;
    gScene = gPhysicsSDK->createScene(sceneDesc);	
 if(!gScene){ 
		sceneDesc.simType				= NX_SIMULATION_SW; 
		gScene = gPhysicsSDK->createScene(sceneDesc);  
		if(!gScene) return;
	}

	// Create the default material
	NxMaterialDesc       m; 
	m.restitution        = 0.5;
	m.staticFriction     = 0.2;
	m.dynamicFriction    = 0.2;
	NxMaterial* mat = gScene->getMaterialFromIndex(0);
	mat->loadFromDesc(m); 

	SetupDampingScene();

	if (gCloths.size() > 0)
		gSelectedCloth = gCloths[0]->getNxCloth();
	else
		gSelectedCloth = NULL;

	if (gScene->getNbActors() > 0)
		gSelectedActor = *gScene->getActors();
	else
		gSelectedActor = NULL;

	// Initialize HUD
    InitializeHUD();
	InitializeSpecialHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	if (gScene)  StartPhysics();
}

void SetDataDirectory()
{
	int set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 1003: Damping");
	SetDataDirectory();
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

void ReleaseNx()
{
    if (gScene)
	{
		GetPhysicsResults();  // Make sure to fetchResults() before shutting down
	    for (MyCloth** cloth = gCloths.begin(); cloth != gCloths.end(); cloth++)
		    delete *cloth;
	    gCloths.clear();
		gPhysicsSDK->releaseScene(*gScene);
	}
	NxCloseCooking();
	if (gPhysicsSDK)  gPhysicsSDK->release();
    NX_DELETE_SINGLE(gAllocator);
}

void ResetNx()
{
	LetGoActor();
	LetGoCloth();
	ReleaseNx();
	InitNx();
}

void StartPhysics()
{
	// Start collision and dynamics for delta time since the last frame
    gScene->simulate(gDeltaTime);
	gScene->flushStream();
}

void GetPhysicsResults()
{
	// Get results from gScene->simulate(gDeltaTime)
	while (!gScene->fetchResults(NX_RIGID_BODY_FINISHED, false));
}



