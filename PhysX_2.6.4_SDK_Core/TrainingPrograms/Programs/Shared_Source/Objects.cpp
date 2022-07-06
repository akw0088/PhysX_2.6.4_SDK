
#include <GL/glut.h>

#include "NxPhysics.h"

#include "DrawObjects.h"

#include "Objects.h"

static bool gShadows = false;

NxObject::NxObject(NxActor* actor) : owner(actor)
{
	color.x = color.y = color.z = 1.0f;

	NX_ASSERT(actor);
	NX_ASSERT(!actor->userData);
	actor->userData	= this;
}

NxObject::~NxObject()
{
}

NxCubeObject::NxCubeObject(NxActor* actor, NxF32 s) : NxObject(actor), size(s)
{
}

NxCubeObject::~NxCubeObject()
{
}

void NxCubeObject::render() const
{
	float glmat[16];
	glPushMatrix();
	owner->getGlobalPose().getColumnMajor44(glmat);
	glMultMatrixf(glmat);
	renderCube(color, size);
	glPopMatrix();

	// Handle shadows
	if(gShadows)
	{
		glPushMatrix();

		const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

		glMultMatrixf(ShadowMat);
		glMultMatrixf(glmat);
		glDisable(GL_LIGHTING);
		glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
		glutSolidCube(size);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_LIGHTING);

		glPopMatrix();
	}
}

NxSphereObject::NxSphereObject(NxActor* actor, NxF32 s) : NxObject(actor), size(s)
{
}

NxSphereObject::~NxSphereObject()
{
}

void NxSphereObject::render() const
{
	float glmat[16];
	glPushMatrix();
	owner->getGlobalPose().getColumnMajor44(glmat);
	glMultMatrixf(glmat);
	renderSphere(color, size);
	glPopMatrix();

	// Handle shadows
	if(gShadows)
	{
		glPushMatrix();

		const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };

		glMultMatrixf(ShadowMat);
		glMultMatrixf(glmat);

		glDisable(GL_LIGHTING);
		glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
		glutSolidSphere(size, 10, 10);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_LIGHTING);

		glPopMatrix();
	}
}

NxCapsuleObject::NxCapsuleObject(NxActor* actor, NxF32 radius, NxF32 height) : NxObject(actor)
{
}

NxCapsuleObject::~NxCapsuleObject()
{
}

static void setupGLMatrix(const NxVec3& pos, const NxMat33& orient)
{
	float glmat[16];	//4x4 column major matrix for OpenGL.
	orient.getColumnMajorStride4(&(glmat[0]));
	pos.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));
}

void NxCapsuleObject::render() const
{
	NxShape* capsule = *owner->getShapes();

	NxMat34 pose = capsule->getGlobalPose();

	const NxReal r = capsule->isCapsule()->getRadius();
	const NxReal h = capsule->isCapsule()->getHeight();

	glPushMatrix();
	setupGLMatrix(pose.t, pose.M);

	DrawCapsule(color, r, h);

	glPopMatrix();
}


NxConvexObject::NxConvexObject(NxActor* actor) : NxObject(actor)
{
}

NxConvexObject::~NxConvexObject()
{
}

void NxConvexObject::render() const
{
	if(owner->getNbShapes()!=1)
		return;
	NxShape* s = owner->getShapes()[0];
	const NxConvexShape* c = s->isConvexMesh();
	if(!c)
		return;
	const NxConvexMesh& cm = c->getConvexMesh();
/*	if(cm.getFormat(0, NX_ARRAY_HULL_VERTICES)!=NX_FORMAT_FLOAT)
		return;
	if(cm.getFormat(0, NX_ARRAY_HULL_POLYGONS)!=NX_FORMAT_BYTE)
		return;
	NxU32 nbHullVerts = cm.getCount(0, NX_ARRAY_HULL_VERTICES);
	NxU32 nbHullPolygons = cm.getCount(0, NX_ARRAY_HULL_POLYGONS);
	const NxF32* verts = (const NxF32*)cm.getBase(0, NX_ARRAY_HULL_VERTICES);
	const NxU8* faces = (const NxU8*)cm.getBase(0, NX_ARRAY_HULL_POLYGONS);*/

	NxU32 nbHullVerts = cm.getCount(0, NX_ARRAY_VERTICES);
	NxU32 nbHullTris = cm.getCount(0, NX_ARRAY_TRIANGLES);
	const NxVec3* verts = (const NxVec3*)cm.getBase(0, NX_ARRAY_VERTICES);
	const NxU32* faces = (const NxU32*)cm.getBase(0, NX_ARRAY_TRIANGLES);


	NxMat34 pose = s->getGlobalPose();
	glPushMatrix();
	setupGLMatrix(pose.t, pose.M);

	static float* vertexBuffer = NULL;
	static float* normalBuffer = NULL;
	static unsigned int bufferSize = 0;

	if (bufferSize < nbHullTris*3*3) {
		delete[] vertexBuffer;
		delete[] normalBuffer;
		vertexBuffer = new float[nbHullTris*3*3];
		normalBuffer = new float[nbHullTris*3*3];
		bufferSize = nbHullTris*3*3;
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	unsigned int index = 0;
	unsigned int vref[3];

	for(unsigned int i=0;i<nbHullTris;i++)
	{
		vref[0] = *faces++;
		vref[1] = *faces++;
		vref[2] = *faces++;

		NxVec3 v0(verts[vref[0]].x, verts[vref[0]].y, verts[vref[0]].z);
		NxVec3 v1(verts[vref[1]].x, verts[vref[1]].y, verts[vref[1]].z);
		NxVec3 v2(verts[vref[2]].x, verts[vref[2]].y, verts[vref[2]].z);
		NxVec3 n = (v0-v1)^(v0-v2);
		n.normalize();

		for (unsigned int v=0;v<3;v++) {
			vertexBuffer[index] = verts[vref[v]].x;
			normalBuffer[index++] = n.x;
			vertexBuffer[index] = verts[vref[v]].y;
			normalBuffer[index++] = n.y;
			vertexBuffer[index] = verts[vref[v]].z;
			normalBuffer[index++] = n.z;
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT, 0, vertexBuffer);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, normalBuffer);
	glDrawArrays(GL_TRIANGLES, 0, nbHullTris*3);
	glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}


void renderCube(const NxVec3& color, NxF32 size)
{
	glColor4f(color.x,color.y,color.z,1.0f);
	glutSolidCube(size);
}

void renderSphere(const NxVec3& color, NxF32 size)
{
	glColor4f(color.x,color.y,color.z,1.0f);
	glutSolidSphere(size, 10, 10);
}

void renderCapsule(const NxVec3& color, NxF32 r, NxF32 h)
{
	DrawCapsule(color, r, h);
}
