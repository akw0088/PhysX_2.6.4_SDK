/*----------------------------------------------------------------------------*\
|
|						     AGEIA PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

//Test Cloth Implementation

#if defined WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX		//suppress windows' global min,max macros.
#include <windows.h>	//needed for gl.h
#endif

#include <GL/glut.h>

#include <stdio.h>
#include "MyCloth.h"
#include "Stream.h"
#include "wavefront.h"
#include "BmpLoader.h"
#include "NxCooking.h"

#define TEAR_MEMORY_FACTOR 2

// -----------------------------------------------------------------------
MyCloth::MyCloth(NxScene *scene, NxClothDesc &desc, char *objFileName, NxReal scale)
{
	mInitDone = false;
	mTexId = 0;
	mTexCoords = NULL;
	NxClothMeshDesc meshDesc;
	generateObjMeshDesc(meshDesc, objFileName, scale);
	// todo: handle failure
	init(scene, desc, meshDesc);
}


// -----------------------------------------------------------------------
MyCloth::MyCloth(NxScene *scene, NxClothDesc &desc, NxReal w, NxReal h, NxReal d, char *texFilename, bool tearLines)
{
	mInitDone = false;
	mTexId = 0;
	mTexCoords = NULL;
	NxClothMeshDesc meshDesc;
	generateRegularMeshDesc(meshDesc, w, h, d, texFilename != NULL, tearLines);
	init(scene, desc, meshDesc);
	if (texFilename) 
		createTexture(texFilename);
}

// -----------------------------------------------------------------------
void MyCloth::init(NxScene *scene, NxClothDesc &desc, NxClothMeshDesc &meshDesc)
{
	mScene = scene;

	// if we want tearing we must tell the cooker
	// this way it will generate some space for particles that will be generated during tearing
	if (desc.flags & NX_CLF_TEARABLE)
		meshDesc.flags |= NX_CLOTH_MESH_TEARABLE;

	cookMesh(meshDesc);
	releaseMeshDescBuffers(meshDesc);

	allocateReceiveBuffers(meshDesc.numVertices, meshDesc.numTriangles);

	desc.clothMesh = mClothMesh;
	desc.meshData = mReceiveBuffers;
	mCloth = scene->createCloth(desc);
	mInitDone = true;
}

// -----------------------------------------------------------------------
MyCloth::~MyCloth()
{
	if (mInitDone) {
		mScene->releaseCloth(*mCloth);
		mScene->getPhysicsSDK().releaseClothMesh(*mClothMesh);
		releaseReceiveBuffers();
		if (mTexCoords) free(mTexCoords);
		if (mTexId) {
			glDeleteTextures(1, &mTexId);
			mTexId = 0;
		}
	}
}

// -----------------------------------------------------------------------
bool MyCloth::generateObjMeshDesc(NxClothMeshDesc &desc, char *filename, NxReal scale)
{
	WavefrontObj wo;
	wo.loadObj(filename);
	if (wo.mVertexCount == 0) return false;

	desc.numVertices				= wo.mVertexCount;
	desc.numTriangles				= wo.mTriCount;
	desc.pointStrideBytes			= sizeof(NxVec3);
	desc.triangleStrideBytes		= 3*sizeof(NxU32);
	desc.vertexMassStrideBytes		= sizeof(NxReal);
	desc.vertexFlagStrideBytes		= sizeof(NxU32);
	desc.points						= (NxVec3*)malloc(sizeof(NxVec3)*desc.numVertices);
	desc.triangles					= (NxU32*)malloc(sizeof(NxU32)*desc.numTriangles*3);
	desc.vertexMasses				= 0;
	desc.vertexFlags				= 0;
	desc.flags						= 0;

	// resize
//	for (NxU32 i = 0; i < positions.size(); i++)
//		positions[i] *= scale;

	// copy positions and indices
	NxVec3 *vSrc = (NxVec3*)wo.mVertices;
	NxVec3 *vDest = (NxVec3*)desc.points;
	for (int i = 0; i < wo.mVertexCount; i++, vDest++, vSrc++) 
		*vDest = (*vSrc)*scale;
	memcpy((NxU32*)desc.triangles, wo.mIndices, sizeof(NxU32)*desc.numTriangles*3);

	return true;
}

// -----------------------------------------------------------------------
void MyCloth::generateRegularMeshDesc(NxClothMeshDesc &desc, NxReal w, NxReal h, NxReal d, bool texCoords, bool tearLines)
{
	int numX = (int)(w / d) + 1;
	int numY = (int)(h / d) + 1;

	desc.numVertices				= (numX+1) * (numY+1);
	desc.numTriangles				= numX*numY*2;
	desc.pointStrideBytes			= sizeof(NxVec3);
	desc.triangleStrideBytes		= 3*sizeof(NxU32);
	desc.vertexMassStrideBytes		= sizeof(NxReal);
	desc.vertexFlagStrideBytes		= sizeof(NxU32);
	desc.points						= (NxVec3*)malloc(sizeof(NxVec3)*desc.numVertices);
	desc.triangles					= (NxU32*)malloc(sizeof(NxU32)*desc.numTriangles*3);
	desc.vertexMasses				= 0;
	desc.vertexFlags				= 0;
	desc.flags						= 0;

	int i,j;
	NxVec3 *p = (NxVec3*)desc.points;
	for (i = 0; i <= numY; i++) {
		for (j = 0; j <= numX; j++) {
			p->set(d*j, 0.0f, d*i); 
			p++;
		}
	}

	if (texCoords) {
		mTexCoords = (GLfloat *)malloc(sizeof(GLfloat)*2*TEAR_MEMORY_FACTOR*desc.numVertices);
		GLfloat *f = mTexCoords;
		GLfloat dx = 1.0f; if (numX > 0) dx /= numX;
		GLfloat dy = 1.0f; if (numY > 0) dy /= numY;
		for (i = 0; i <= numY; i++) {
			for (j = 0; j <= numX; j++) {
				*f++ = j*dx;
				*f++ = i*dy;
			}
		}
		mNumTexCoords = desc.numVertices;
	}
	else mNumTexCoords = 0;

	NxU32 *id = (NxU32*)desc.triangles;
	for (i = 0; i < numY; i++) {
		for (j = 0; j < numX; j++) {
			NxU32 i0 = i * (numX+1) + j;
			NxU32 i1 = i0 + 1;
			NxU32 i2 = i0 + (numX+1);
			NxU32 i3 = i2 + 1;
			if ((j+i)%2) {
				*id++ = i0; *id++ = i2; *id++ = i1;
				*id++ = i1; *id++ = i2; *id++ = i3;
			}
			else {
				*id++ = i0; *id++ = i2; *id++ = i3;
				*id++ = i0; *id++ = i3; *id++ = i1;
			}
		}
	}

	// generate tear lines if necessary
	if(tearLines)
		generateTearLines(desc, numX + 1, numY + 1);
}

// -----------------------------------------------------------------------
void MyCloth::generateTearLines(NxClothMeshDesc& desc, NxU32 w, NxU32 h)
{
	// allocate flag buffer
	if(desc.vertexFlags == 0)
		desc.vertexFlags = malloc(sizeof(NxU32)*desc.numVertices);

	// create tear lines
	NxU32* flags = (NxU32*)desc.vertexFlags;
	NxU32 y;
	for(y = 0; y < h; y++)
	{
		NxU32 x;
		for(x = 0; x < w; x++)
		{
			if(((x + y) % 16 == 0) || ((x - y + 16) % 16 == 0))
				flags[y * w + x] = NX_CLOTH_VERTEX_TEARABLE;
			else
				flags[y * w + x] = 0;
		}
	}
}

// -----------------------------------------------------------------------
void MyCloth::releaseMeshDescBuffers(const NxClothMeshDesc& desc)
{
	NxVec3* p = (NxVec3*)desc.points;
	NxU32* t = (NxU32*)desc.triangles;
	NxReal* m = (NxReal*)desc.vertexMasses;
	NxU32* f = (NxU32*)desc.vertexFlags;
	free(p);
	free(t);
	free(m);
	free(f);
}

// -----------------------------------------------------------------------
bool MyCloth::cookMesh(NxClothMeshDesc& desc)
{
	// we cook the mesh on the fly through a memory stream
	// we could also use a file stream and pre-cook the mesh
	MemoryWriteBuffer wb;
	if (!NxCookClothMesh(desc, wb)) 
		return false;

	MemoryReadBuffer rb(wb.data);
	mClothMesh = mScene->getPhysicsSDK().createClothMesh(rb);
	return true;
}

// -----------------------------------------------------------------------
void MyCloth::allocateReceiveBuffers(int numVertices, int numTriangles)
{
	// here we setup the buffers through which the SDK returns the dynamic cloth data
	// we reserve more memory for vertices than the initial mesh takes
	// because tearing creates new vertices
	// the SDK only tears cloth as long as there is room in these buffers
	NxU32 maxVertices = TEAR_MEMORY_FACTOR * numVertices;
	mReceiveBuffers.verticesPosBegin = (NxVec3*)malloc(sizeof(NxVec3)*maxVertices);
	mReceiveBuffers.verticesNormalBegin = (NxVec3*)malloc(sizeof(NxVec3)*maxVertices);
	mReceiveBuffers.verticesPosByteStride = sizeof(NxVec3);
	mReceiveBuffers.verticesNormalByteStride = sizeof(NxVec3);
	mReceiveBuffers.maxVertices = maxVertices;
	mReceiveBuffers.numVerticesPtr = (NxU32*)malloc(sizeof(NxU32));

	// the number of triangles is constant, even if the cloth is torn
	NxU32 maxIndices = 3*numTriangles;
	mReceiveBuffers.indicesBegin = (NxU32*)malloc(sizeof(NxU32)*maxIndices);
	mReceiveBuffers.indicesByteStride = sizeof(NxU32);
	mReceiveBuffers.maxIndices = maxIndices;
	mReceiveBuffers.numIndicesPtr = (NxU32*)malloc(sizeof(NxU32));

	// the parent index information would be needed if we used textured cloth
	NxU32 maxParentIndices = maxVertices;
	mReceiveBuffers.parentIndicesBegin = (NxU32*)malloc(sizeof(NxU32)*maxParentIndices);
	mReceiveBuffers.parentIndicesByteStride = sizeof(NxU32);
	mReceiveBuffers.maxParentIndices = maxParentIndices;
	mReceiveBuffers.numParentIndicesPtr = (NxU32*)malloc(sizeof(NxU32));

	// init the buffers in case we want to draw the mesh 
	// before the SDK as filled in the correct values
	*mReceiveBuffers.numVerticesPtr = 0;
	*mReceiveBuffers.numIndicesPtr = 0;
}

// -----------------------------------------------------------------------
void MyCloth::releaseReceiveBuffers()
{
	NxVec3* vp;
	NxU32* up; 
	vp = (NxVec3*)mReceiveBuffers.verticesPosBegin; free(vp);
	vp = (NxVec3*)mReceiveBuffers.verticesNormalBegin; free(vp);
	up = (NxU32*)mReceiveBuffers.numVerticesPtr; free(up);

	up = (NxU32*)mReceiveBuffers.indicesBegin; free(up);
	up = (NxU32*)mReceiveBuffers.numIndicesPtr; free(up);

	up = (NxU32*)mReceiveBuffers.parentIndicesBegin; free(up);
	up = (NxU32*)mReceiveBuffers.numParentIndicesPtr; free(up);
}

// -----------------------------------------------------------------------
void MyCloth::draw(bool shadows)
{
	NxU32 numVertices = *mReceiveBuffers.numVerticesPtr;
	NxU32 numTriangles = *mReceiveBuffers.numIndicesPtr / 3;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, mReceiveBuffers.verticesPosBegin);
	glNormalPointer(GL_FLOAT, 0, mReceiveBuffers.verticesNormalBegin);

	if (mTexId) {
		updateTextureCoordinates();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, mTexCoords);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glColor4f(1.0f, 1.0f, 1.0f,1.0f);
	}

#ifdef __CELLOS_LV2__	
	glDrawRangeElements(GL_TRIANGLES, 0, numVertices-1, 3*numTriangles, GL_UNSIGNED_INT, mReceiveBuffers.indicesBegin);
#else
	glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT, mReceiveBuffers.indicesBegin);
#endif

	if (mTexId) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

	if (shadows) {
		const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
		glPushMatrix();
		glMultMatrixf(ShadowMat);
		glDisable(GL_LIGHTING);
		glColor4f(0.05f, 0.1f, 0.15f,1.0f);

#ifdef __CELLOS_LV2__	
		glDrawRangeElements(GL_TRIANGLES, 0, numVertices-1, 3*numTriangles, GL_UNSIGNED_INT, mReceiveBuffers.indicesBegin);
#else
		glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT, mReceiveBuffers.indicesBegin);
#endif
		
		glColor4f(1.0f, 1.0f, 1.0f,1.0f);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}


// -----------------------------------------------------------------------
bool MyCloth::createTexture(char *filename)
{
	BmpLoader bl;
	if (!bl.loadBmp(filename)) return false;

	glGenTextures(1, &mTexId);
	if (!mTexId) return false;
	glBindTexture(GL_TEXTURE_2D, mTexId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		bl.mWidth, bl.mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bl.mRGB);

	return true;
}

// -----------------------------------------------------------------------
void MyCloth::updateTextureCoordinates()
{
	NxU32 numVertices = *mReceiveBuffers.numVerticesPtr;
	NxU32 *parent = (NxU32 *)mReceiveBuffers.parentIndicesBegin + mNumTexCoords;
	for (NxU32 i = mNumTexCoords; i < numVertices; i++) {
		mTexCoords[2*i] = mTexCoords[2*(*parent)];
		mTexCoords[2*i+1] = mTexCoords[2*(*parent)+1];
	}

	mNumTexCoords = numVertices;
}
