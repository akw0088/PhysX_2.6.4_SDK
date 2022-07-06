// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                 LESSON 904: FLUID RENDERING
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================


#include "Lesson904.h"
#include "UpdateTime.h"

#include "ErrorStream.h"
#include "CookASE.h"

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;
NxScene*          gScene = NULL;
NxVec3            gDefaultGravity(0,-9.8,0);
 
// User report globals
DebugRenderer     gDebugRenderer;
UserAllocator*	  gAllocator = NULL;
ErrorStream  	  gErrorStream;

// HUD globals
HUD hud;
 
// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,2.5,15);
NxVec3 gCameraForward(0,0,-1);
NxVec3 gCameraRight(1,0,0);
const NxReal gCameraSpeed = 0.2;

//static NxVec3 Eye(5.0f, 2.0f, 5.0f);
//static NxVec3 Dir(-0.6,-0.2,-0.7);

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 50;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

#define REST_PARTICLES_PER_METER 10
#define KERNEL_RADIUS_MULTIPLIER 1.8
#define MOTION_LIMIT_MULTIPLIER 3
#define PACKET_SIZE_MULTIPLIER 8

// Simulation globals
NxReal gDeltaTime = 1.0/60.0;
NxReal gTotalTime = 0;
bool bHardwareScene = true;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

static int				gPoints = 0;
static int              gDrawSurface = 0;
static int              gDrawSurfaceFW = 1;
static int              gDrawSurfaceSW = 0;
static bool             gDrawScene = true;
static bool             gSingleStep = false;

static bool             bUpdateCamera = true;
static bool				bSceneRunning = false;

// Actor globals
NxActor* groundPlane = NULL;
NxActor* box = NULL;
NxActor* sphere = NULL;
NxActor* capsule = NULL;
NxActor* drain = NULL;

NxActor* flume = NULL;
NxActor* bowl = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/ase";
char fname1[] = "../../../Data/ase";
char fname2[] = "../../TrainingPrograms/Programs/Data/ase";
char fname3[] = "TrainingPrograms/Programs/Data/ase";

// Fluid globals
NxFluid* fluid = NULL;

// Fluid surface globals
NxImplicitScreenMesh*	gFluidMesh = NULL;
NxImplicitScreenMesh*	gParticleMesh = NULL;
NxReal					gProjMatrix[16];
const  int              NUM_VERTS = 400000;
//static float			gVertices[NUM_VERTS * 3];
//static float			gNormals[NUM_VERTS * 3];

struct SSimpleVertex
{
    NxVec3 v3Position;
    NxVec3 v3Normal; 
};
SSimpleVertex gFullVerts[NUM_VERTS * 3];

int              gIndices[NUM_VERTS];
int              gWidth;
int              gHeight;
NxU32            gNumVertices;
NxU32            gNumIndices;

// Fluid particle globals
float            gParticleVertices[NUM_VERTS * 3];
float            gParticleNormals[NUM_VERTS * 3];
int              gParticleIndices[NUM_VERTS];
NxU32            gParticleNumVertices;
NxU32            gParticleNumIndices;

NxVec3			gParticleBuffer[100000];
unsigned		gParticleBufferCap = 32766;
unsigned		gParticleBufferNum = 0;
NxVec3			gStaticParticleBuffer[100000];
unsigned		gStaticParticleBufferNum = 0;

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
    if (gScene && bUpdateCamera && gFluidMesh)
    {
        NxReal projMatrix[16];
        glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);

	    // Setup new camera
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	    gluLookAt(gCameraPos.x, gCameraPos.y, gCameraPos.z, gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

        // set new camera to surfaces for next frame
        glGetFloatv(GL_PROJECTION_MATRIX, gProjMatrix);
        if (gFluidMesh)
            gFluidMesh->setProjectionMatrix(gProjMatrix);
        if (gParticleMesh)
            gParticleMesh->setProjectionMatrix(gProjMatrix);

    	glMatrixMode(GL_MODELVIEW);
    }
    else
    {
		// Setup camera
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
		gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		ActorUserData* ud = (ActorUserData*)(actor->userData);

		if (ud && ud->flags & UD_IS_DRAIN)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
			glColor4f(0.1f, 0.8f, 1.0f, 0.8f);

 			DrawActor(actor, gSelectedActor, true);

			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else
		{
 			DrawActor(actor, gSelectedActor, true);

			// Handle shadows
            if (shadows)
            {
		        DrawActorShadow(actor, true);
            }
		}
    }
}

void RenderSurface(float* vertices, float* normals, int* indices, int numVerts, int numIndices, int drawSurface)
{
    if (drawSurface == 0)
        glPolygonMode(GL_FRONT, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	    glPointSize(2);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numIndices; i+=3)  // +3 for 3 indices per triangle
    {
        int triId0 = indices[i+0]*3;       // *3 for 3 floats per vertex
        int triId1 = indices[i+1]*3;
        int triId2 = indices[i+2]*3;

        glNormal3f(normals[triId0+0], normals[triId0+1], normals[triId0+2]);
        glVertex3f(vertices[triId0+0], vertices[triId0+1], vertices[triId0+2]);

        glNormal3f(normals[triId1+0], normals[triId1+1], normals[triId1+2]);
        glVertex3f(vertices[triId1+0], vertices[triId1+1], vertices[triId1+2]);
		
		glNormal3f(normals[triId2+0], normals[triId2+1], normals[triId2+2]);
        glVertex3f(vertices[triId2+0], vertices[triId2+1], vertices[triId2+2]);
    }
    glEnd();

    glColor3f(1,1,1);
    glPolygonMode(GL_FRONT, GL_FILL);
}

// Modified for user vertex normal data structure
void RenderSurfaceWithInterleavedDataStructure(SSimpleVertex* fullVerts, int* indices, int numIndices, int drawSurface)
{
    if ( numIndices )
    {
        if (drawSurface == 0)
    	    glPolygonMode(GL_FRONT, GL_FILL);
        else
    	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// render it with Vertex_Array
		//glEnableClientState(GL_INDEX_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
    
		glVertexPointer(3, GL_FLOAT, sizeof(SSimpleVertex), &(fullVerts[0].v3Position));
		glNormalPointer(GL_FLOAT, sizeof(SSimpleVertex), &(fullVerts[0].v3Normal));
		//glIndexPointer(GL_INT, 0, indices);

		//glDrawArrays(GL_TRIANGLES, 0, numIndices);
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indices);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		//glDisableClientState(GL_INDEX_ARRAY);
/*    
		// the following also works
		float v0[3],n0[3];
		float v1[3],n1[3];
		float v2[3],n2[3];
		glBegin(GL_TRIANGLES);
        for (int i = 0; i < numIndices; i+=3)  // +3 for 3 indices per triangle
        {
		    int triId0 = indices[i+0];       // *3 for 3 floats per vertex
		    int triId1 = indices[i+1];
		    int triId2 = indices[i+2];
			
			v0[0] = gFullVerts[triId0].v3Position.x;
			v0[1] = gFullVerts[triId0].v3Position.y;
			v0[2] = gFullVerts[triId0].v3Position.z;

			v1[0] = gFullVerts[triId1].v3Position.x;
			v1[1] = gFullVerts[triId1].v3Position.y;
			v1[2] = gFullVerts[triId1].v3Position.z;

			v2[0] = gFullVerts[triId2].v3Position.x;
			v2[1] = gFullVerts[triId2].v3Position.y;
			v2[2] = gFullVerts[triId2].v3Position.z;

			n0[0] = gFullVerts[triId0].v3Normal.x;
			n0[1] = gFullVerts[triId0].v3Normal.y;
			n0[2] = gFullVerts[triId0].v3Normal.z;

			n1[0] = gFullVerts[triId1].v3Normal.x;
			n1[1] = gFullVerts[triId1].v3Normal.y;
			n1[2] = gFullVerts[triId1].v3Normal.z;

			n2[0] = gFullVerts[triId2].v3Normal.x;
			n2[1] = gFullVerts[triId2].v3Normal.y;
			n2[2] = gFullVerts[triId2].v3Normal.z;
			glNormal3fv(n0);
		    glVertex3fv(v0);
            glNormal3fv(n1);
		    glVertex3fv(v1);
            glNormal3fv(n2);
		    glVertex3fv(v2);
        }
    	glEnd();
*/
    	glPolygonMode(GL_FRONT, GL_FILL);
	}
}

void RenderFluid()
{
	glColor3f(1,1,1);
	glLoadIdentity();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

    if (gFluidMesh)
        gFluidMesh->generateMesh();
    if (gParticleMesh)
        gParticleMesh->generateMesh();

	if (gFluidMesh)
    {
        gFluidMesh->getMesh();
        glColor3f(0,0,1);
        //RenderSurface(gVertices, gNormals, gIndices, gNumVertices, gNumIndices);
		RenderSurfaceWithInterleavedDataStructure(gFullVerts, gIndices, gNumIndices, 0);
    }
#if 0
	if (gParticleMesh)
    {
        gParticleMesh->getMesh();
        glColor3f(0,1,0);
        RenderSurface(gParticleVertices, gParticleNormals, gParticleIndices, gParticleNumVertices, gParticleNumIndices, 0);
    }
#endif  // 0

	// Keep physics & graphics in sync
	if (gPoints == 0) {
		glColor3f(1,0,0);
		glPointSize(3);
		glBegin(GL_POINTS);
		for (unsigned p=0; p<gParticleBufferNum; p++)
		{
			NxVec3 & particle = gParticleBuffer[p];
			glVertex3fv(&particle.x);
		}
		glEnd();
		glColor3f(1,1,1);
	} else if (gPoints == 1) {
		for (unsigned p=0; p<gParticleBufferNum; p++)
		{
			glColor3f(1,0,0);
			NxVec3 & particle = gParticleBuffer[p];

			glPushMatrix();
			glTranslatef(particle.x,particle.y,particle.z);
//			glutSolidSphere(0.1,3,3);//8,5);
			glutSolidCube(0.1);
			glPopMatrix();
		}
	}
// Old red lego fluid
#if 0
    // Keep physics & graphics in sync
	for (NxU32 p=0; p<gParticleBufferNum; p++)
	{
		NxVec3& particle = gParticleBuffer[p];

		glPushMatrix();
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glTranslatef(particle.x,particle.y,particle.z);
		glutSolidCube(0.1);  // glutSolidSphere(0.1,3,3);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glPopMatrix();
	}
#endif  // 0
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
	NxVec3 forceVec = forceStrength*forceDir;

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
 	RenderActors(false/*bShadows*/);

	RenderFluid();

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	if (!bPause)
	{
	    if (gDeltaTime < 10)  gTotalTime += gDeltaTime;
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
    if((gScene && bUpdateCamera && gFluidMesh) &&
      ((gWidth != width) || (gHeight != height)))
    {
        gFluidMesh->setWindowWidth(width);
        gFluidMesh->setWindowHeight(height);
    }
    gWidth = width;
    gHeight = height;
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
		case 'v': { FreezeFluid(); break; }
		case 'c': { bUpdateCamera = !bUpdateCamera; break; }
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
    glEnable(GL_NORMALIZE);

    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, 400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

void CreateFluid(const NxVec3& pos, const NxU32 sideNum, const NxReal distance)
{
    gWidth = 512;
    gHeight = 512;

    NxParticleData particles;
    NxParticleData staticParticles;

	// Create a set of particles
	gParticleBufferNum = 0;
	float rad = sideNum*distance*0.5;
	for (unsigned i=0; i<sideNum; i++)
	{
		for (unsigned j=0; j<sideNum; j++)
		{
			for (unsigned k=0; k<sideNum; k++)
			{
				NxVec3 p = NxVec3(i*distance,j*distance,k*distance);
				if (p.distance(NxVec3(rad,rad,rad)) < rad)
				{
					p += pos;
					gParticleBuffer[gParticleBufferNum++] = p;
				}
			}
		}
    }

	// Set structure to pass particles, and receive them after every simulation step
	particles.maxParticles			= gParticleBufferNum;
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
    fluidDesc.stiffness						= 40;  // 50
    fluidDesc.viscosity						= 22;
	fluidDesc.restDensity					= 1000;
    fluidDesc.damping						= 0;
    fluidDesc.staticCollisionRestitution	= 0.4;  
	fluidDesc.staticCollisionAdhesion		= 0.01;  // 0.03;  
	fluidDesc.simulationMethod				= NX_F_SPH;  //NX_F_NO_PARTICLE_INTERACTION;

	fluidDesc.initialParticleData			= particles;
	fluidDesc.particlesWriteData			= particles;

//    fluidDesc.flags &= ~NX_FF_HARDWARE;

    fluid = gScene->createFluid(fluidDesc);
    assert(fluid != NULL);

    // Create the fluid surface
    NxImplicitScreenMeshDesc meshDesc, particleMeshDesc;
    meshDesc.setToDefault();
    meshDesc.width = gWidth;
    meshDesc.height = gHeight;

	glMatrixMode(GL_PROJECTION);
	gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x, gCameraPos.y, gCameraPos.z, gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);
	glMatrixMode(GL_MODELVIEW);
    glGetFloatv(GL_PROJECTION_MATRIX, gProjMatrix);
    meshDesc.projectionMatrix = gProjMatrix;
/*
    meshDesc.meshData.verticesPosBegin = gVertices;
    meshDesc.meshData.verticesNormalBegin = gNormals;
    meshDesc.meshData.verticesPosByteStride = sizeof(float)*3;
    meshDesc.meshData.verticesNormalByteStride = sizeof(float)*3;
*/
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
    meshDesc.threshold = 0.25;
    meshDesc.particleRadius = 0.1;

    assert(meshDesc.meshData.isValid());
    assert(meshDesc.isValid());

    gFluidMesh = fluid->createScreenSurfaceMesh(meshDesc);
//	printf("\nFluid Particle Radius = %f",gFluidMesh->getParticleRadius());

    particleMeshDesc = meshDesc;
    particleMeshDesc.meshData.verticesPosBegin = gParticleVertices;
    particleMeshDesc.meshData.verticesNormalBegin = gParticleNormals;
    particleMeshDesc.meshData.numVerticesPtr = &gParticleNumVertices;
    particleMeshDesc.meshData.indicesBegin = gParticleIndices;
    particleMeshDesc.meshData.numIndicesPtr = &gParticleNumIndices;
	particleMeshDesc.meshData.verticesPosByteStride = sizeof(float)*3;
	particleMeshDesc.meshData.verticesNormalByteStride = sizeof(float)*3;

    gParticleMesh = gScene->createImplicitScreenMesh(particleMeshDesc);
//    printf("\nUser Defined Particle Radius = %f",gParticleMesh->getParticleRadius());

/*
    gStaticParticleBufferNum = gParticleBufferNum;
    memcpy(gStaticParticleBuffer, gParticleBuffer, sizeof(NxVec3)*gStaticParticleBufferNum);
	particles.numParticlesPtr		= &gStaticParticleBufferNum;
	particles.bufferPos				= &gStaticParticleBuffer[0].x;
*/
    gParticleMesh->setParticles(particles);
}

void RestartFluid()
{
	if (bSceneRunning) {
		NxU32 error;
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true, &error);
		bSceneRunning = false;
		assert(error == 0);
	}
    if (gFluidMesh)
        fluid->releaseScreenSurfaceMesh(*gFluidMesh);
    if (gParticleMesh)
        gScene->releaseImplicitScreenMesh(*gParticleMesh);
    gScene->releaseFluid(*fluid);
	fluid = 0;
	CreateFluid(NxVec3(0,1.5,0), 19, 0.1);
}

void FreezeFluid()
{
	if (!fluid) return;

	if (bSceneRunning) {
		NxU32 error;
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true, &error);
		bSceneRunning = false;
		assert(error == 0);
	}

    bool enabled = fluid->getFlag(NX_FF_ENABLED)!=0;
	fluid->setFlag(NX_FF_ENABLED, !enabled);
	gScene->simulate(1.0f/60.0f);
	bSceneRunning = true;
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

	// Initialize physics SDK
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator, &gErrorStream);
	if (!gPhysicsSDK)  return;

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 10);
	//gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_POSITION, 1);
	//gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_VELOCITY, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_PACKETS, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_DRAINS, 1);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.simType				= NX_SIMULATION_HW;
	sceneDesc.gravity				= gDefaultGravity;
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

	// Initialize cooking
    NxInitCooking(NULL, &gErrorStream);

	// Load ASE files
    int set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname1[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	bowl = CookASE("fluidSample.ase", gScene, NxVec3(1,0,0));
	flume = CookASE("coolFlow.ase", gScene, NxVec3(1,-4,-0), NxVec3(1,0.2,1));

	box = CreateBox(NxVec3(2,5-0.75,7), NxVec3(0.75,0.75,0.75), 1);
	sphere = CreateSphere(NxVec3(0,4-0.4,1), 0.4, 1);
	capsule = CreateCapsule(NxVec3(3,0-(1+0.5),8), 1, 1, 1);

	drain = CreateBox(NxVec3(0,-10-1,0), NxVec3(40,1,40), 0);
	NxShape*const* shapes = drain->getShapes();
	NxU32 nShapes = drain->getNbShapes();
	shapes[0]->setFlag(NX_SF_FLUID_DRAIN, true);
//	drain->raiseActorFlag(NX_AF_DISABLE_COLLISION);

	CreateFluid(NxVec3(0,1.5,0), 19, 0.1);

    AddUserDataToActors(gScene);
	((ActorUserData*)(drain->userData))->flags |= UD_IS_DRAIN;

	// Page in the hardware meshes
	PageInHardwareMeshes(gScene);

	gSelectedActor = box;

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
	bSceneRunning = true;
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
	InitGlut(argc, argv, "Lesson 904: Fluid Rendering");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}

