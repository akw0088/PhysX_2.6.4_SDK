// ===============================================================================
//						     PHYSX SDK TRAINING PROGRAMS
//			                     LESSON 901: FLUIDS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "Lesson901.h"
#include "UpdateTime.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;
NxVec3            gDefaultGravity(0,-9.8,0);
 
// User report globals
DebugRenderer     gDebugRenderer;
UserAllocator*	  gAllocator = NULL;

// HUD globals
HUD hud;
 
// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,2,-6);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
const NxReal gCameraSpeed = 10;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 10000;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

#define REST_PARTICLES_PER_METER 10
#define KERNEL_RADIUS_MULTIPLIER 1.8
//#define MOTION_LIMIT_MULTIPLIER (3*KERNEL_RADIUS_MULTIPLIER)
#define MOTION_LIMIT_MULTIPLIER 3
#define PACKET_SIZE_MULTIPLIER 8

// Simulation globals
NxReal gDeltaTime = 1.0/60.0;
bool bHardwareScene = false;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

// Actor globals
NxActor* groundPlane = NULL;
NxActor* box = NULL;
NxActor* flatSurface = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

// Fluid globals
NxFluid* fluid = NULL;

// Fluid particle globals
NxVec3 gParticleBuffer[10000];
NxU32 gParticleBufferCap = 10000;
NxU32 gParticleBufferNum = 0;

// Mesh globals
NxTriangleMeshDesc flatSurfaceTriangleMeshDesc;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n");
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

void RenderFluid()
{
    // Keep physics & graphics in sync
	for (NxU32 p=0; p<gParticleBufferNum; p++)
	{
		NxVec3& particle = gParticleBuffer[p];

		glPushMatrix();
		glColor4f(1.0f,0.0f,0.0f,1.0f);
		glTranslatef(particle.x,particle.y,particle.z);
		glutSolidCube(0.1);  // glutSolidSphere(0.1,8,5);
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glPopMatrix();
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
			case 'i': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,1),gForceStrength,bForceMode); break; }
			case 'k': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
			case 'j': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(1,0,0),gForceStrength,bForceMode); break; }
			case 'l': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'u': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,1,0),gForceStrength,bForceMode); break; }
			case 'm': {gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
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

	RenderFluid();

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

NxFluid* CreateFluid(const NxVec3& pos, NxU32 sideNum, NxReal distance, NxScene* scene)
{
	// Create a set of particles
	gParticleBufferNum = 0;
	NxReal rad = sideNum*distance*0.5;
	for (NxU32 i=0; i<sideNum; i++)
	{
		for (NxU32 j=0; j<sideNum; j++)
		{
			for (NxU32 k=0; k<sideNum; k++)
			{
				NxVec3 p = NxVec3(i*distance,j*distance,k*distance);
				if (p.distance(NxVec3(rad,rad,rad)) < rad)
				{
					p += pos - NxVec3(rad,rad,rad);
					gParticleBuffer[gParticleBufferNum++] = p;
				}
			}
		}
	}

	// Set structure to pass particles, and receive them after every simulation step
	NxParticleData particles;
	particles.maxParticles			= gParticleBufferCap;
	particles.numParticlesPtr		= &gParticleBufferNum;
	particles.bufferPos				= &gParticleBuffer[0].x;
	particles.bufferPosByteStride	= sizeof(NxVec3);

	// Create a fluid descriptor
	NxFluidDesc fluidDesc;
    fluidDesc.maxParticles                  = particles.maxParticles;
    fluidDesc.kernelRadiusMultiplier		= KERNEL_RADIUS_MULTIPLIER;
    fluidDesc.restParticlesPerMeter			= REST_PARTICLES_PER_METER;
	fluidDesc.motionLimitMultiplier			= MOTION_LIMIT_MULTIPLIER;
	fluidDesc.packetSizeMultiplier			= PACKET_SIZE_MULTIPLIER;
    fluidDesc.stiffness						= 50;
    fluidDesc.viscosity						= 22;
	fluidDesc.restDensity					= 1000;
    fluidDesc.damping						= 0;
    fluidDesc.staticCollisionRestitution	= 0.4;
	fluidDesc.staticCollisionAdhesion		= 0.03;
	fluidDesc.simulationMethod				= NX_F_SPH; //NX_F_NO_PARTICLE_INTERACTION;

	fluidDesc.initialParticleData			= particles;
	fluidDesc.particlesWriteData			= particles;

//    fluidDesc.flags &= ~NX_FF_HARDWARE;

    NxFluid* fl = gScene->createFluid(fluidDesc);
	assert(fl != NULL);
	return fl;

/*
    // Create the fluid surface
    NxImplicitScreenMeshDesc meshDesc, particleMeshDesc;
    meshDesc.setToDefault();
    meshDesc.width = gWidth;
    meshDesc.height = gHeight;

	glMatrixMode(GL_PROJECTION);
	gluPerspective(60.0f, gRatio, 1.0f, 10000.0f);
	gluLookAt(Eye.x, Eye.y, Eye.z, Eye.x + Dir.x, Eye.y + Dir.y, Eye.z + Dir.z, 0.0f, 1.0f, 0.0f);
	glMatrixMode(GL_MODELVIEW);
    glGetFloatv(GL_PROJECTION_MATRIX, gProjMatrix);
    meshDesc.projectionMatrix = gProjMatrix;

//    meshDesc.meshData.verticesPosBegin = gVertices;
//    meshDesc.meshData.verticesNormalBegin = gNormals;
//    meshDesc.meshData.verticesPosByteStride = sizeof(float)*3;
//    meshDesc.meshData.verticesNormalByteStride = sizeof(float)*3;

	meshDesc.meshData.verticesPosBegin = &(gFullVerts[0].v3Position);
	meshDesc.meshData.verticesNormalBegin = &(gFullVerts[0].v3Normal);
	meshDesc.meshData.verticesPosByteStride = sizeof(SSimpleVertex);
	meshDesc.meshData.verticesNormalByteStride = sizeof(SSimpleVertex);

    meshDesc.meshData.maxVertices = NUM_VERTS;
    meshDesc.meshData.numVerticesPtr = &gNumVertices;
    meshDesc.meshData.indicesBegin = gIndices;
    meshDesc.meshData.indicesByteStride = sizeof(int);
    meshDesc.meshData.maxIndices = NUM_VERTS;
    meshDesc.meshData.numIndicesPtr = &gNumIndices;

    meshDesc.screenSpacing = 5;
    meshDesc.triangleWindingPositive = 1;
    meshDesc.threshold = .25;
    meshDesc.particleRadius = .1;

    assert(meshDesc.meshData.isValid());
    assert(meshDesc.isValid());

    gFluidMesh = gFluid->createScreenSurfaceMesh(meshDesc);
	printf("\nFluid Particle Radius = %f",gFluidMesh->getParticleRadius());


    particleMeshDesc = meshDesc;
    particleMeshDesc.meshData.verticesPosBegin = gParticleVertices;
    particleMeshDesc.meshData.verticesNormalBegin = gParticleNormals;
    particleMeshDesc.meshData.numVerticesPtr = &gParticleNumVertices;
    particleMeshDesc.meshData.indicesBegin = gParticleIndices;
    particleMeshDesc.meshData.numIndicesPtr = &gParticleNumIndices;
	particleMeshDesc.meshData.verticesPosByteStride = sizeof(float)*3;
	particleMeshDesc.meshData.verticesNormalByteStride = sizeof(float)*3;

    gParticleMesh = gScene->createImplicitScreenMesh(particleMeshDesc);
    printf("\nUser Defined Particle Radius = %f",gParticleMesh->getParticleRadius());

//  gStaticParticleBufferNum = gParticleBufferNum;
//  memcpy(gStaticParticleBuffer, gParticleBuffer, sizeof(NxVec3)*gStaticParticleBufferNum);
//	particles.numParticlesPtr		= &gStaticParticleBufferNum;
//	particles.bufferPos				= &gStaticParticleBuffer[0].x;

    gParticleMesh->setParticles(particles);
*/
}

void ReleaseFluid()
{
	if (fluid)
	{
		if (gScene)  gScene->releaseFluid(*fluid);
		fluid = NULL;
	}
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
 	sceneDesc.simType = NX_SIMULATION_HW;
    sceneDesc.gravity = gDefaultGravity;
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
	flatSurface = CreateFlatSurface(NxVec3(0,0.5,0), 6, 6, 2);
	box = CreateBox(NxVec3(1,2,0),NxVec3(0.5,0.5,0.5),10);

	fluid = CreateFluid(NxVec3(0,2,0), 15, 0.1, gScene);

	AddUserDataToActors(gScene);

	// Page in the hardware meshes
	PageInHardwareMeshes(gScene);

	gSelectedActor = box;

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
	ReleaseFluid();
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
	NxReal gDeltaTime = UpdateTime();

	// Start collision and dynamics for delta time since the last frame
    gScene->simulate(1.0/60.0);
	gScene->flushStream();
}

void GetPhysicsResults()
{
	// Get results from gScene->simulate(deltaTime)
	while (!gScene->fetchResults(NX_RIGID_BODY_FINISHED, false));
}

int main(int argc, char** argv)
{
	PrintControls();
	InitGlut(argc, argv, "Lesson 901: Fluids");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
