// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    HEIGHTFIELD CREATION METHODS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include "NxPhysics.h"

extern NxPhysicsSDK* gPhysicsSDK;
extern NxScene *gScene;

#include "Heightfield.h"
#include "tga.h"

#include <stdio.h>
#include "Stream.h"
#include "NxCooking.h"

// Mesh globals
NxTriangleMeshDesc heightfieldMeshDesc;

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/Terrain";
char fname1[] = "../../../Data/Terrain";
char fname2[] = "../../TrainingPrograms/Programs/Data/Terrain";
char fname3[] = "TrainingPrograms/Programs/Data/Terrain";

#define HEIGHTFIELD_SIZE		32
#define HEIGHTFIELD_NB_VERTS	HEIGHTFIELD_SIZE*HEIGHTFIELD_SIZE
#define HEIGHTFIELD_NB_FACES	(HEIGHTFIELD_SIZE-1)*(HEIGHTFIELD_SIZE-1)*2
#define HEIGHTFIELD_OFFSET		-20
#define HEIGHTFIELD_WIDTH		20

NxVec3* gHeightfieldVerts = NULL;
NxVec3* gHeightfieldNormals = NULL;
NxU32* gHeightfieldFaces = NULL;

NxActor* CreateHeightfield(const NxVec3& pos, int index, NxReal step)
{
	tgaInfo* heightfield = NULL;

	// Set terrain data directory
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
	
	if (index == 0)
	    heightfield = tgaLoad("terrain1.tga");
	else if (index == 1)
	    heightfield = tgaLoad("terrain2.tga");
	else
	    heightfield = tgaLoad("terrain3.tga");

	// Initialize heightfield vertices
	gHeightfieldVerts = new NxVec3[HEIGHTFIELD_NB_VERTS];
	for (NxU32 y=0;y<HEIGHTFIELD_SIZE;y++)
	{
		for (NxU32 x=0;x<HEIGHTFIELD_SIZE;x++)
		{
			gHeightfieldVerts[x+y*HEIGHTFIELD_SIZE] = NxVec3(NxF32(x)-(NxF32(HEIGHTFIELD_SIZE-1)*0.5f), 0.0f, NxF32(y)-(NxF32(HEIGHTFIELD_SIZE-1)*0.5f)) * HEIGHTFIELD_WIDTH;
		}
	}

	// Initialize heightfield faces
	gHeightfieldFaces = new NxU32[HEIGHTFIELD_NB_FACES*3];
	NxU32 k = 0;
	for (NxU32 j=0;j<HEIGHTFIELD_SIZE-1;j++)
	{
		for (NxU32 i=0;i<HEIGHTFIELD_SIZE-1;i++)
		{
			// Create first triangle
			gHeightfieldFaces[k++] = i   + j*HEIGHTFIELD_SIZE;
			gHeightfieldFaces[k++] = i   + (j+1)*HEIGHTFIELD_SIZE;
			gHeightfieldFaces[k++] = i+1 + (j+1)*HEIGHTFIELD_SIZE;

			// Create second triangle
			gHeightfieldFaces[k++] = i   + j*HEIGHTFIELD_SIZE;
			gHeightfieldFaces[k++] = i+1 + (j+1)*HEIGHTFIELD_SIZE;
			gHeightfieldFaces[k++] = i+1 + j*HEIGHTFIELD_SIZE;
		}
	}

	// Build heightfield from image data
	for (NxI32 i = 0; i < heightfield->width; i++)
	{
		for (NxI32 j = 0; j < heightfield->height; j++)
		{
			unsigned char h = (unsigned char)(*(heightfield->imageData + i + j*heightfield->width));
			gHeightfieldVerts[((heightfield->width - 1) - i) + j*heightfield->width].y = ((float)h/255.0)*step + 10;
		}
	}

	// Build vertex normals
//	gHeightfieldNormals = new NxVec3[HEIGHTFIELD_NB_VERTS];
//	NxBuildSmoothNormals(HEIGHTFIELD_NB_FACES, HEIGHTFIELD_NB_VERTS, gHeightfieldVerts, gHeightfieldFaces, NULL, gHeightfieldNormals, true);

	// Build physical model
	NxTriangleMeshDesc heightfieldDesc;
	heightfieldDesc.numVertices					= HEIGHTFIELD_NB_VERTS;
	heightfieldDesc.numTriangles				= HEIGHTFIELD_NB_FACES;
	heightfieldDesc.pointStrideBytes			= sizeof(NxVec3);
	heightfieldDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	heightfieldDesc.points						= gHeightfieldVerts;
	heightfieldDesc.triangles					= gHeightfieldFaces;							
	heightfieldDesc.flags						= NX_MF_HARDWARE_MESH;

	heightfieldDesc.heightFieldVerticalAxis		= NX_Y;
	heightfieldDesc.heightFieldVerticalExtent	= -1000;

	// Need to save away drawable heightfield mesh with correct indices
	heightfieldMeshDesc = heightfieldDesc;

	NxTriangleMeshShapeDesc heightfieldShapeDesc;
    NxInitCooking();
    if (0)
    {
        // Cooking from file
#ifdef WIN32
        bool status = NxCookTriangleMesh(heightfieldDesc, UserStream("c:\\tmp.bin", false));
        heightfieldShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
#endif
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookTriangleMesh(heightfieldDesc, buf);
        heightfieldShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
    }

	if (heightfieldShapeDesc.meshData)
    {
	    NxActorDesc actorDesc;
	    actorDesc.shapes.pushBack(&heightfieldShapeDesc);
	    actorDesc.globalPose.t = pos;  // NxVec3(0,0,0);
	    NxActor* actor = gScene->createActor(actorDesc);

	    return actor;
//      gPhysicsSDK->releaseTriangleMesh(*heightfieldShapeDesc.meshData);
    }

	return NULL;
}

void RenderHeightfield()
{
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();

//  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
//  glClear( GL_COLOR_BUFFER_BIT );

//	GLuint texture;
  
//  texture = LoadTextureRAW( "texture.raw", TRUE );
  
//  setup texture mapping
//  glEnable( GL_TEXTURE_2D );
//  glBindTexture( GL_TEXTURE_2D, texture );

	for(NxU32 i=0;i<HEIGHTFIELD_NB_FACES;i++)
	{
//		glNormal3fv(&gHeightfieldNormals[gHeightfieldFaces[i*3+0]].x);
		glVertex3fv(&gHeightfieldVerts[gHeightfieldFaces[i*3+0]].x);
//		glTexCoord2d(float(i*3+0)%(float)(HEIGHTFIELD_SIZE), float(i*3+0)/float(HEIGHTFIELD_SIZE));

//		glNormal3fv(&gHeightfieldNormals[gHeightfieldFaces[i*3+1]].x);
		glVertex3fv(&gHeightfieldVerts[gHeightfieldFaces[i*3+1]].x);
//		glTexCoord2d(float(i*3+1)%(float)(HEIGHTFIELD_SIZE), float(i*3+1)/float(HEIGHTFIELD_SIZE));

//		glNormal3fv(&gHeightfieldNormals[gHeightfieldFaces[i*3+2]].x);
		glVertex3fv(&gHeightfieldVerts[gHeightfieldFaces[i*3+2]].x);
//		glTexCoord2d(float(i*3+2)%(float)(HEIGHTFIELD_SIZE), float(i*3+2)/float(HEIGHTFIELD_SIZE));
	}
    // free the texture
 //   FreeTexture( texture );
//    glEnd();
}

