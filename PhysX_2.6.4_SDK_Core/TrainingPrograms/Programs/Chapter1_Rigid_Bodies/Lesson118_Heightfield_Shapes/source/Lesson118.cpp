// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			               LESSON 118: HEIGHTFIELD SHAPES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>

#include "NxPhysics.h"
#include "CommonCode.h"
#include "Lesson118.h"

#include "NxHeightField.h"
#include "NxHeightFieldDesc.h"
#include "NxHeightFieldShapeDesc.h"
#include "NxHeightFieldSample.h"

#include "NxTriangle.h"

// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;

// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;
extern ErrorStream       gErrorStream;

// Camera globals
extern NxReal gCameraSpeed;

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
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;

extern NxActor* gSelectedActor;

NxActor* heightfield = NULL;
const NxHeightFieldShape* heightfieldShape = NULL;

NxVec3 gSpawnPos = NxVec3(1,3,1);

bool bShowContactTriangles = false;

class ContactTriangle
{
public:
	ContactTriangle(const NxVec3& _p0, const NxVec3& _p1, const NxVec3& _p2, const NxVec3& _color)
	{
		p0 = _p0;
		p1 = _p1;
		p2 = _p2;
		color = _color;
	}

	NxVec3	p0;
	NxVec3	p1;
	NxVec3	p2;
	NxVec3	color;
};

typedef NxArray<ContactTriangle> ContactTrisArray;

// Contact globals
ContactTrisArray ctArray;

class ContactReport : public NxUserContactReport
{
public:
    virtual void onContactNotify(NxContactPair& pair, NxU32 events)
    {
        // Iterate through contact points
        NxContactStreamIterator i(pair.stream);
        //user can call getNumPairs() here
        while(i.goNextPair())
        {
            //user can also call getShape() and getNumPatches() here
            while(i.goNextPatch())
            {
                //user can also call getPatchNormal() and getNumPoints() here
                const NxVec3& contactNormal = i.getPatchNormal();
                while(i.goNextPoint())
                {
                    //user can also call getPoint() and getSeparation() here
                    const NxVec3& contactPoint = i.getPoint();

                    NxU32 faceIndex = i.getFeatureIndex0();
                    if (faceIndex==0x00000000)	
                        faceIndex = i.getFeatureIndex1();
                    if (faceIndex!=0x00000000)
                    {
                        NxU32 flags;
                        NxTriangle tri;
                        NxTriangle edgeTri;
                        heightfieldShape->getTriangle(tri, &edgeTri, &flags, faceIndex, true);
                        ContactTriangle ct(tri.verts[0], tri.verts[1], tri.verts[2], NxVec3(1,0,0));
                        ctArray.pushBack(ct);
                    }
                }
            }
        }
    }
} gContactReport;

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\nt = Move Focus Actor to (0,5,0)\n");
	printf("\n Special:\n --------------\n v = Visualize Terrain Contact Triangles\n");
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
 	RenderActors(false);



	if (heightfield)
	{

		const NxHeightFieldShape * hfs = (const NxHeightFieldShape*)heightfield->getShapes()[0];

		int numVertices = (hfs->getHeightField().getNbRows()-1)*(hfs->getHeightField().getNbColumns()-1)*3*2;
		float* pVertList = new float[numVertices*3];
		float* pNormList = new float[numVertices*3];

		int vertIndex = 0;
		int normIndex = 0;

		glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
		for(NxU32 row = 0; row < hfs->getHeightField().getNbRows() - 1; row++)
		{
			for(NxU32 column = 0; column < hfs->getHeightField().getNbColumns() - 1; column++)
			{
				NxTriangle tri;
				NxVec3 n;
				NxU32 triangleIndex;

				triangleIndex = 2 * (row * hfs->getHeightField().getNbColumns() + column);

				if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
				{
					n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
					n.normalize();
					for(int i=0;i<3;i++)
						{
						pVertList[vertIndex++] = tri.verts[i].x;
						pVertList[vertIndex++] = tri.verts[i].y;
						pVertList[vertIndex++] = tri.verts[i].z;
						pNormList[normIndex++] = n.x;
						pNormList[normIndex++] = n.y;
						pNormList[normIndex++] = n.z;
						}
				}

				triangleIndex++;
				
				if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
				{
					n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
					n.normalize();
					for(int i=0;i<3;i++)
					{
						pVertList[vertIndex++] = tri.verts[i].x;
						pVertList[vertIndex++] = tri.verts[i].y;
						pVertList[vertIndex++] = tri.verts[i].z;
						pNormList[normIndex++] = n.x;
						pNormList[normIndex++] = n.y;
						pNormList[normIndex++] = n.z;
					}

				}
			}
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT, 0, pVertList);
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, pNormList);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		delete[] pVertList;
		delete[] pNormList;

	}




	// Render the contact triangles
	if (bShowContactTriangles)
	{
	    // Draw contact triangles
	    NxU32 i;
	    for (i = 0; i < ctArray.size(); i++)
	    {
		    ContactTriangle tri = ctArray[i];
		    DrawLine(tri.p0, tri.p1, tri.color);
		    DrawLine(tri.p1, tri.p2, tri.color);
		    DrawLine(tri.p2, tri.p0, tri.color);
	    }
	    ctArray.clear();
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

void SpecialKeys(unsigned char key, int x, int y)
{
	switch (key)
    {
	    case 'v': { bShowContactTriangles = !bShowContactTriangles; break; }
	}
}

static const int gMatrixSize = 3;
static const NxMaterialIndex gMatrix[9][3] =
{
    // {tesselation, material0, material1}
    {0,1,1}, {0,1,1}, {0,1,1}, 
    {0,1,1}, {0,1,1}, {0,1,1}, 
    {0,1,1}, {0,1,1}, {0,1,1} 
};

NxActor* CreateHeightfield(const NxVec3& pos, NxVec3 size = NxVec3(10,1,10), NxU32 nbColumns=10, NxU32 nbRows=10)
{
	static NxReal sixtyFourKb = 65536.0;
	static NxReal thirtyTwoKb = 32767.5;

	NxHeightFieldDesc heightFieldDesc;
	heightFieldDesc.nbColumns			= nbColumns;
	heightFieldDesc.nbRows				= nbRows;
	heightFieldDesc.verticalExtent		= -1000;
	heightFieldDesc.convexEdgeThreshold = 0;

	// Allocate storage for heightfield samples
	heightFieldDesc.samples			= new NxU32[nbColumns*nbRows];
	heightFieldDesc.sampleStride	= sizeof(NxU32);

	char* currentByte = (char*)heightFieldDesc.samples;
	for (NxU32 row = 0; row < nbRows; row++)
	{
		for (NxU32 column = 0; column < nbColumns; column++)
		{
			NxReal s = NxReal(row) / NxReal(nbRows);
			NxReal t = NxReal(column) / NxReal(nbColumns);
			NxI16 height = (NxI32)(0.5 * thirtyTwoKb * (NxMath::sin(5.0f*NxPiF32*s) + NxMath::cos(5.0f*NxPiF32*t)));
//			NxI16 height = - (nbRows / 2 - row) * (nbRows / 2 - row) - (nbColumns / 2 - column) * (nbColumns / 2 - column);
			NxU32 matrixOffset = (row % gMatrixSize) * gMatrixSize + (column % gMatrixSize);
			
			NxHeightFieldSample* currentSample = (NxHeightFieldSample*)currentByte;
			currentSample->height = height;
			currentSample->materialIndex0 = gMatrix[matrixOffset][1];
			currentSample->materialIndex1 = gMatrix[matrixOffset][2];
			currentSample->tessFlag = gMatrix[matrixOffset][0];

			currentByte += heightFieldDesc.sampleStride;
		}
	}

	NxHeightField* heightField = gScene->getPhysicsSDK().createHeightField(heightFieldDesc);

	// Data has been copied, we can free our buffer
	delete[] heightFieldDesc.samples;

	// The actor has one shape, a heightfield
	NxHeightFieldShapeDesc heightFieldShapeDesc;
	heightFieldShapeDesc.heightField	= heightField;
	heightFieldShapeDesc.shapeFlags		= NX_SF_FEATURE_INDICES | NX_SF_VISUALIZATION;
	heightFieldShapeDesc.heightScale	= size.y / sixtyFourKb;
	heightFieldShapeDesc.rowScale		= size.x / NxReal(nbRows-1);
	heightFieldShapeDesc.columnScale	= size.z / NxReal(nbColumns-1);
	// heightFieldShapeDesc.meshFlags	= NX_MESH_SMOOTH_SPHERE_COLLISIONS;
	heightFieldShapeDesc.materialIndexHighBits = 0;
	heightFieldShapeDesc.holeMaterial = 2;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&heightFieldShapeDesc);
	actorDesc.globalPose.t = pos;
	NxActor* newActor = gScene->createActor(actorDesc);

    return newActor;
}

void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
	if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.05);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 0.2);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity;
	sceneDesc.userContactReport     = &gContactReport;
    //sceneDesc.simType				= NX_SIMULATION_SW;
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
	box = CreateBox(NxVec3(5,0,0), NxVec3(0.5,1,0.5), 20);
	sphere = CreateSphere(NxVec3(0,0,5), 1, 10);
	capsule = CreateCapsule(NxVec3(-5,0,0), 2, 0.5, 10);
	pyramid = CreatePyramid(NxVec3(0,0,-5), NxVec3(1,0.5,1.5), 10);

    heightfield = CreateHeightfield(NxVec3(-15,-2,-15), NxVec3(30,5,30), 40, 40);
	heightfieldShape = (const NxHeightFieldShape*)heightfield->getShapes()[0];

	AddUserDataToActors(gScene);

	gSelectedActor = pyramid;
    gCameraSpeed = 10;

	if (heightfield)
	{
		gScene->setActorPairFlags(*heightfield, *box, NX_NOTIFY_ON_TOUCH);
		gScene->setActorPairFlags(*heightfield, *sphere, NX_NOTIFY_ON_TOUCH);
		gScene->setActorPairFlags(*heightfield, *capsule, NX_NOTIFY_ON_TOUCH);
		gScene->setActorPairFlags(*heightfield, *pyramid, NX_NOTIFY_ON_TOUCH);
	}

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
	InitGlut(argc, argv, "Lesson 118: Heightfield Shapes");
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}



