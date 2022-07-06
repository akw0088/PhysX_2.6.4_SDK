//Raycast car sample

#include "SampleRaycastCar.h"
#include "ErrorStream.h"

#include "NxAllVehicles.h"

#include <NxCooking.h>

CreationModes creationMode = MODE_CAR;


bool				gPause = false;
bool				gDebugVisualization = false;
bool				oldStyle = false;
NxPhysicsSDK*		gPhysicsSDK = 0;
NxScene*			gScene = 0;
NxVec3				gDefaultGravity(0.0f, -10.0f, 0.0f);
//ErrorStream			gErrorStream;

bool keyDown[256];



static NxMaterialIndex 
	defaultMaterialIndex, 
	somewhatBouncyMaterialIndex, 
	veryBouncyMaterialIndex, 
	frictionlessMaterialIndex,
	highFrictionMaterialIndex,
	anisoMaterialIndex;
	




NX_INLINE void setupColor(NxU32 color)
	{
		NxF32 Blue	= NxF32((color)&0xff)/255.0f;
		NxF32 Green	= NxF32((color>>8)&0xff)/255.0f;
		NxF32 Red	= NxF32((color>>16)&0xff)/255.0f;
		glColor4f(Red, Green, Blue, 1.0f);
	}



// Render code

class DebugRenderer
	{
	public:

void DebugRenderer::renderData(const NxDebugRenderable& data) const
	{
		glPushMatrix();

		// Render lines
		{
			NxU32 NbLines = data.getNbLines();
			const NxDebugLine* Lines = data.getLines();

      glDisable(GL_LIGHTING);
			glBegin(GL_LINES);
			while(NbLines--)
				{
				setupColor(Lines->color);
				glVertex3fv(&Lines->p0.x);
				glVertex3fv(&Lines->p1.x);
				Lines++;
				}
			glEnd();
		}

    glEnable(GL_LIGHTING);
		glPopMatrix();
	}


	} gDebugRenderer;



static void CreateCube(const NxVec3& pos, const NxVec3* initial_velocity=NULL)
{
	// Create body
	NxBodyDesc BodyDesc;
	if(initial_velocity)	BodyDesc.linearVelocity = *initial_velocity;

	NxBoxShapeDesc boxDesc;
	NxReal size = 0.5f;
	boxDesc.dimensions		= NxVec3(size, size * NxMath::rand(0.0f,1.0f), size);
	BodyDesc.mass = 10.f;

	NxActorDesc ActorDesc;
//	ActorDesc.userData		= (void*)size;
	ActorDesc.shapes.pushBack(&boxDesc);
	ActorDesc.body			= &BodyDesc;
	ActorDesc.globalPose.t  = pos;
	gScene->createActor(ActorDesc);
}





static void InitNx()
	{
	// Initialize PhysicsSDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, &gErrorStream);
	if(!gPhysicsSDK)	return;
	NxInitCooking(NULL, &gErrorStream);

	gPhysicsSDK->setParameter(NX_MIN_SEPARATION_FOR_PENALTY, -0.05f);
	//enable visualisation
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);

	gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_AXES, 1.0f);
	//gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_MASS_AXES, 1.0f);
	
	
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AXES, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_POINT, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_CONTACT_NORMAL, 1.0f);

	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_WORLD_AXES, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_ERROR, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_FORCE, 1.0f);
	gPhysicsSDK->setParameter(NX_VISUALIZE_JOINT_REDUCED, 1.0f);

	// Don't slow down jointed objects
	gPhysicsSDK->setParameter(NX_ADAPTIVE_FORCE, 0.0f);
	
	//create some materials -- note that we reuse the same NxMaterial every time,
	//as it gets copied to the SDK with every setMaterial() call, not just referenced.
	
	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.gravity				= gDefaultGravity;
	sceneDesc.broadPhase			= NX_BROADPHASE_COHERENT;
	sceneDesc.collisionDetection	= true;
	sceneDesc.userContactReport		= carContactReport;
	gScene = gPhysicsSDK->createScene(sceneDesc);
	
	//default material
	defaultMaterialIndex = 0;
	NxMaterial * defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.8f);
	defaultMaterial->setDynamicFriction(0.8f);

	// Create ground plane
	NxPlaneShapeDesc planeDesc;
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);

	//embed some simple shapes in terrain to make sure we can drive on them:
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(10,1,5);
	boxDesc.localPose.t.set(10,0,30);
	boxDesc.localPose.M.fromQuat(NxQuat(30, NxVec3(1,0,0)));
	actorDesc.shapes.pushBack(&boxDesc);


	NxSphereShapeDesc sphereShape;
	sphereShape.radius = 20;
	sphereShape.localPose.t.set(40, -18, 0);
	actorDesc.shapes.pushBack(&sphereShape);

	NxCapsuleShapeDesc capsuleShape;
	capsuleShape.radius = 10.0f;
	capsuleShape.height = 10.0f;
	capsuleShape.localPose.t.set(-10, -8, 0);
	capsuleShape.localPose.M.setColumn(1, NxVec3(0,0,1));
	capsuleShape.localPose.M.setColumn(2, NxVec3(0,-1,0));
	actorDesc.shapes.pushBack(&capsuleShape);

	gScene->createActor(actorDesc);


	//terrain:
	InitTerrain();

	//turn on all contact notifications:
	gScene->setActorGroupPairFlags(0, 0, NX_NOTIFY_ON_TOUCH);
	}


static int gMainHandle;
static bool gShadows = true;
static float gRatio = 1.0f;
static NxVec3 Eye(7.0f, 5.0f, 8.0f);
static NxVec3 Dir(-0.6,-0.2,-0.7);
static NxVec3 N;
static int mx = 0;
static int my = 0;

void createCar(unsigned char key, bool alt, bool ctrl) {
	NX_ASSERT(key >= '0' && key <= '9');
	bool monsterTruck = alt;
	bool oldStyle = ctrl;
	NxReal xpos = monsterTruck?4:10;
	NxReal ypos = 2.f;
	NxReal zpos = 5.f;
	NxReal zdist = 3.5f;

	switch(key) {
		case '1':   createCarWithDesc(NxVec3(xpos,ypos,zpos - 3*zdist), true, false, false, monsterTruck, oldStyle, gPhysicsSDK); break;
		case '2':   createCarWithDesc(NxVec3(xpos,ypos,zpos - 2*zdist), false, true, false, monsterTruck, oldStyle, gPhysicsSDK); break;
		case '3':   createCarWithDesc(NxVec3(xpos,ypos,zpos - 1*zdist), true, true, false, monsterTruck, oldStyle, gPhysicsSDK); break;
		case '4':   createCarWithDesc(NxVec3(xpos,ypos,zpos + 0*zdist), true, false, true, monsterTruck, oldStyle, gPhysicsSDK); break;
		case '5':   createCarWithDesc(NxVec3(xpos,ypos,zpos + 1*zdist), false, true, true, monsterTruck, oldStyle, gPhysicsSDK); break;
		case '6':   createCarWithDesc(NxVec3(xpos,ypos,zpos + 2*zdist), true, true, true, monsterTruck, oldStyle, gPhysicsSDK); break;
		case '7':	createCart(NxVec3(15, ypos, 0), true, false, oldStyle); break;
		case '8':	createCart(NxVec3(15, ypos, 4), false, true, oldStyle); break;
		case '9':	createCart(NxVec3(15, ypos, 8), true, true, oldStyle); break;
		case '0':   createCarWithDesc(NxVec3(121.58167, 1.6793282, 48.097473), true, false, false, true, oldStyle, gPhysicsSDK); break;
		default:
			printf("default reached, use break\n");
	}
}

void createTruck(unsigned char key, bool alt, bool ctrl) {
	NX_ASSERT(key >= '0' && key <= '9');
	NxReal xpos = 40;
	NxReal ypos = 2.f;
	NxReal zdist = 5.f;
	NxReal zpos = 10.f + (key-'0') * zdist;
	bool oldStyle = ctrl;

	switch(key) {
		case '1': createTruckPuller(NxVec3(xpos, ypos, zpos), 10, oldStyle); break;
		case '2': createTruckWithTrailer1(NxVec3(xpos, ypos, zpos),15, oldStyle); break;
		case '3': createFullTruck(NxVec3(xpos, ypos, zpos), 32, false, oldStyle); break;
		case '4': createFullTruck(NxVec3(xpos, ypos, zpos), 32, true, oldStyle); break;
		case '5': createFullTruckWithTrailer2(NxVec3(xpos, ypos, zpos), 16, oldStyle); break;
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
		default:
			printf("default reached, use break\n");

	}
}

static void appKey(unsigned char key, bool down)
	{
	if (!down)
		return;
	//printf("appkey(%c (%x))\n", key, key);
	bool alt = (glutGetModifiers() & GLUT_ACTIVE_ALT) > 0;

	switch (key)
		{
		//case 'c':   createCar(NxVec3(0,5,0)); break;
		case 27:	exit(0); break;
		case 'x':	gShadows = !gShadows; break;
		case 'p':	gPause = !gPause; break;
		case 'o':	oldStyle = true; printf("will use old style wheels for next car!\n");  break;
		case 'n':	oldStyle = false; printf("will use wheel shapes for next car!\n");  break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0': {
			if (creationMode == MODE_CAR) {
				createCar(key, alt, oldStyle);
			} else if (creationMode == MODE_TRUCK) {
				createTruck(key, alt, oldStyle);
			}
			break;
		}

		case 'f':	NxAllVehicles::getActiveVehicle()->standUp(); break;
		case 't':	creationMode = MODE_TRUCK; break;
		case 'c':	creationMode = MODE_CAR; break;

		case 'v':	gDebugVisualization = !gDebugVisualization; break;

		case '+':	NxAllVehicles::getActiveVehicle()->gearUp(); break;
		case '-':	NxAllVehicles::getActiveVehicle()->gearDown(); break;

		/*
		case 'n':  wheelSpring.spring *= 1.1f; printf("suspensionSpring: %f\n", wheelSpring.spring); break;
		case 'm':  wheelSpring.spring *= 0.9f; printf("suspensionSpring: %f\n", wheelSpring.spring); break;
		case 'j':  wheelSpring.damper *= 1.1f; printf("suspensionDamping: %f\n", wheelSpring.damper); break;
		case 'k':  wheelSpring.damper *= 0.9f; printf("suspensionDamping: %f\n", wheelSpring.damper); break;
		case 'f':  wheelSpring.targetValue *= 1.1f; printf("suspensionBias: %f\n", wheelSpring.targetValue); break;
		case 'g':  wheelSpring.targetValue *= 0.9f; printf("suspensionBias: %f\n", wheelSpring.targetValue); break;
		case 'h':  wheelSpring.targetValue *= -1; printf("suspensionBias: %f\n", wheelSpring.targetValue); break;
		*/
		case 9: // TAB
			{
			NxAllVehicles::selectNext();
			} break;
		case 13: // Enter
			{
			NxVec3 t = Eye;
			NxVec3 Vel = Dir;
			Vel.normalize();
			Vel*=30.0f;
			CreateCube(t, &Vel);
			}
			break;
		}
	
	}


static void MouseCallback(int button, int state, int x, int y)
	{
	mx = x;
	my = y;
	}

static void MotionCallback(int x, int y)
	{
	int dx = mx - x;
	int dy = my - y;
	
	Dir.normalize();
	N.cross(Dir,NxVec3(0,1,0));
	
	NxQuat qx(NxPiF32 * dx * 20/ 180.0f, NxVec3(0,1,0));
	qx.rotate(Dir);
	NxQuat qy(NxPiF32 * dy * 20/ 180.0f, N);
	qy.rotate(Dir);
	
	mx = x;
	my = y;
	}

extern NxMaterialIndex materialIce, materialRock, materialMud, materialGrass, materialDefault;
static void RenderCallback()
	{
#ifdef WIN32
	static DWORD PreviousTime = 0;
	DWORD CurrentTime = timeGetTime();
	DWORD ElapsedTime = CurrentTime - PreviousTime;
	if (ElapsedTime < 10.0f)
		return;

	PreviousTime = CurrentTime;


#endif


	// Clear buffers -- do it now so we can render some debug stuff in tickCar.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Physics code
	if(gScene && !gPause)	
		{
		//tickCar();
		bool left = keyDown['a'] || keyDown[20];
		bool right = keyDown['d'] || keyDown[22];
		bool forward = keyDown['w'] || keyDown[21];
		bool backward = keyDown['s'] || keyDown[23];
		NxReal steering = 0;
		if (left && !right) steering = -1;
		else if (right && !left) steering = 1;
		NxReal acceleration = 0;
		if (forward && !backward) acceleration = 1;
		else if (backward && !forward) acceleration = -1;
		bool handbrake = keyDown[' '];
		if (NxAllVehicles::getActiveVehicle())
			NxAllVehicles::getActiveVehicle()->control(steering, false, acceleration, false, handbrake);
		NxAllVehicles::updateAllVehicles(1.0f/60.f);

		gScene->simulate(1.0f/60.0f);	//Note: a real application would compute and pass the elapsed time here.
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		}
	// ~Physics code
	
	
	// Setup camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, gRatio, 1.0f, 10000.0f);
	
	float Position[]		= { 100.0f, 100.0f, 400.0f, 1.0f };		glLightfv(GL_LIGHT0, GL_POSITION, Position);
	
	if(NxAllVehicles::getActiveVehicle() != NULL)
	{
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
  	
  	#define BUFFERSIZE 25
  	static NxMat34 mbuffer[BUFFERSIZE];
  	static int frameNum = 0;
  	int index = frameNum % BUFFERSIZE;
  	
	  NxMat34 camera, cameraInv;
	  NxVec3 cameraZ, lookAt, cr, tmp;
	  NxF32 cameraInvMat[16];
	  
	  NxMat34 car = NxAllVehicles::getActiveVehicle()->getGlobalPose();
	  
	  car.M.getColumn(2, cr);
	  car.M.getColumn(0, tmp);
	  car.M.setColumn(0, cr);
	  car.M.setColumn(2, -tmp);
	  
	  
	  if(frameNum == 0)
	  {
	    for(int i=0; i<BUFFERSIZE; i++)
	      mbuffer[i] = car;
	  }
	  
	  camera = mbuffer[index];
	  mbuffer[index] = car;
	  
	  camera.t.y += 2.f;
	  
	  camera.M.getColumn(2, cameraZ);
	  camera.t += (NxAllVehicles::getActiveVehicle()->getCameraDistance() * cameraZ);
	  
	  lookAt = (camera.t - car.t);
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
	  
	  frameNum++;
	}
	else
	{
		//camera controls:
		if (keyDown['a'] || keyDown[20]) Eye -= N;
		if (keyDown['d'] || keyDown[22]) Eye += N;
		if (keyDown['w'] || keyDown[21]) Eye += Dir;
		if (keyDown['s'] || keyDown[23]) Eye -= Dir;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);
	}
	

	//render ground plane:
	glPushMatrix();
	/*
	NX_ASSERT(gScene->getNbActors() > 0);
	NX_ASSERT(gScene->getActors()[0]->getNbShapes() > 0);
	NxMaterialIndex mat = gScene->getActors()[0]->getShapes()[0]->getMaterial();
	if		(mat == materialIce)	glColor4f(1.0f,1.0f,1.0f, 1.0f);
	else if (mat == materialRock)	glColor4f(0.3f,0.3f,0.3f, 1.0f);
	else if (mat == materialMud)	glColor4f(0.6f,0.3f,0.2f, 1.0f);
	else if (mat == materialGrass)	glColor4f(0.2f,0.8f,0.2f, 1.0f);
	else 
	*/
		{ 
		GLfloat gray = 0.4f; glColor4f(gray, gray, gray, 1.0f); 
		}

	if (!gDebugVisualization) {
		glScalef(10, 0, 10);
		glutSolidCube(600.0f);
	} else {
		glBegin(GL_LINES);
		for (NxReal x = -100; x < 100; x++)
			{
			glVertex3f(x,0,-100.0f);
			glVertex3f(x,0,100.0f);

			glVertex3f(-100.0f,0,x);
			glVertex3f(100.0f,0,x);
			}
		glEnd();
	}



	glPopMatrix();

	glPushMatrix();
	RenderTerrain();

	if (gDebugVisualization) 
		{
		gDebugRenderer.renderData(*gScene->getDebugRenderable());
		} 
	else 
		{
		RenderAllActors();
		}

	static CreationModes _oldCreationMode = MODE_NONE;
	static OrthographicDrawing orthoDraw;

	if (_oldCreationMode != creationMode) {
		_oldCreationMode = creationMode;
		switch(creationMode) {
			case MODE_CAR: {
				char info[] =
					"Car Mode\n"
					"1 - normal fw car\n"
					"2 - normal rw car\n"
					"3 - normal 4x4 car\n"
					"4 - corvette fw\n"
					"5 - corvette rw\n"
					"6 - corvette 4x4\n"
					"7 - cart fw\n"
					"8 - cart bw\n"
					"9 - cart 4x4\n"
					"\nUse [ALT] for monstertrucks\n"
					"\nUse \'T\' for Truck Mode";
				orthoDraw.drawText(100, 30, 2, 1000, info);
			}break;
			case MODE_TRUCK: {
				char info[] =
					"Truck Mode\n"
					"1 - Truck\n"
					"2 - Truck with Trailer 1\n"
					"3 - Full Truck\n"
					"4 - Full Truck with 4 axis\n"
					"5 - Full Truck with Trailer (does not yet work)\n"
					"6 - \n"
					"7 - \n"
					"8 - \n"
					"9 - \n";
					"\nUse \'C\' for Car Mode";
					orthoDraw.drawText(100, 30, 2, 1000, info);
			}break;
		}
	}

	glPopMatrix();
	NxAllVehicles::drawVehicles(gDebugVisualization);

	
	/////////////// begin orthographic drawing

	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	orthoDraw.setOrthographicProjection(width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glLoadIdentity();

	renderHUD(orthoDraw);
	orthoDraw.render(false);

	glPopMatrix();
	orthoDraw.resetPerspectiveProjection();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	/////////////// end orthographic drawing

	glFlush();
	glutSwapBuffers();
	}

void ReshapeCallback(int width, int height)
	{
	glViewport(0, 0, width, height);
	gRatio = float(width)/float(height);
	}

void IdleCallback()
	{
	glutPostRedisplay();
	}


void callback_special(int key, int x, int y)
	{
	//map specials to ASCII too:
	unsigned char c = (unsigned char) key;
	if (c >= 100)
		c -= 80;
	keyDown[c] = true;
	appKey(c, true);
	}

void callback_specialUp(int key, int x, int y)//view control
	{
	//map specials to ASCII too:
	unsigned char c = (unsigned char) key;
	if (c >= 100)
		c -= 80;
	keyDown[c] = false;
	appKey(c, false);
	}

void callback_key(unsigned char c, int x, int y)
	{
	if (c >= 'A' && c <= 'Z')
		c = c - 'A' + 'a';
	keyDown[c] = true;
	appKey(c, true);
	}

void callback_keyUp(unsigned char c, int x, int y)
	{
	if (c >= 'A' && c <= 'Z')
		c = c - 'A' + 'a';
	keyDown[c] = false;
	appKey(c,false);
	}



int main(int argc, char** argv)
	{
	// Initiamalize Glut
	printf("Press Numkeys to create car(s).  Press [Enter] to shoot boxes.\n");
	printf("press t for Truck Mode, c for Car Mode\n");
	printf("press o to create old style wheels, n for wheel shapes\n");
	printf("w,a,s,d  & up down left right - drive car\n");
	printf("v for debug render mode\n");
	printf("p for pausing the simulatioin\n");
	printf("f to apply random upward forces to the car (use when stuck)\n");
	printf("+/- to gear up/down (only on cars with gears)\n");
	printf("tab - switch cars\n");
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("SampleRaycastCar new 2");
	glutSetWindow(gMainHandle);
	glutDisplayFunc(RenderCallback);
	glutReshapeFunc(ReshapeCallback);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(callback_key);
	glutSpecialFunc(callback_special);
	glutKeyboardUpFunc(callback_keyUp);
	glutSpecialUpFunc(callback_specialUp);

	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
	MotionCallback(0,0);
	
	// Setup default render states
	glClearColor(0.3f, 0.4f, 0.5f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);
	
	// Setup lighting
	glEnable(GL_LIGHTING);
	float AmbientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };		glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	float DiffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	float SpecularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };		glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
	float Position[]		= { 100.0f, 100.0f, 400.0f, 1.0f };		glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);
	
	// Physics code
	InitNx();
	//initCar();
	// ~Physics code
	
	// Run
	glutMainLoop();
    
    return 0;
	}


void RenderAllActors() {
	for (NxU32 i = 0; i < gScene->getNbActors(); i++) {
		NxActor* curActor = gScene->getActors()[i];
		if (curActor->userData)
			continue;
		for (NxU32 j = 0; j < curActor->getNbShapes(); j++) {
			NxShape* curShape = curActor->getShapes()[j];
			if (curShape->userData || curShape->isPlane() || curShape->isTriangleMesh())
				continue;
			float shapeMat[16];
			glPushMatrix();

			NxMaterialIndex material = curShape->getMaterial();
			if		(material == materialIce)	glColor4f(1.0f,1.0f,1.0f,1.0f);
			else if (material == materialRock)	glColor4f(0.3f,0.3f,0.3f,1.0f);
			else if (material == materialMud)	glColor4f(0.6f,0.3f,0.2f,1.0f);
			else if (material == materialGrass)	glColor4f(0.2f,0.8f,0.2f,1.0f);
			else { GLfloat gray = 0.4f; glColor4f(gray, gray, gray, 1.0f); }
		
			curShape->getGlobalPose().getColumnMajor44(shapeMat);
			glMultMatrixf(shapeMat);
			if (NxBoxShape* box = curShape->isBox()) {
				NxVec3 dim = box->getDimensions();
				glScalef(dim.x, dim.y, dim.z);
				glutSolidCube(2);
			} else if (NxCapsuleShape* capsule = curShape->isCapsule()) {
				NxReal radius = capsule->getRadius();
				NxReal height = capsule->getHeight();
				static GLUquadricObj* quadric = NULL;
				if (quadric == NULL) {
					quadric = gluNewQuadric();
					gluQuadricDrawStyle(quadric, GLU_FILL);
				}
				glTranslatef(0,-height/2.f, 0);
				glRotatef(90, -1,0,0);
				glutSolidSphere(radius, 40, 20);
				gluCylinder(quadric, radius, radius, height, 40, 1);
				glTranslatef(0,0,height);
				glutSolidSphere(radius, 40, 20);
				//glRotatef(90, 1,0,0);
			} else if (NxSphereShape* sphere = curShape->isSphere()) {
				NxReal radius = sphere->getRadius();
				glutSolidSphere(radius, 20, 20);
			}
			glPopMatrix();
		}
	}
}
