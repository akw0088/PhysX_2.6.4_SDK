// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							   OBJECT DRAWING ROUTINES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "NxPhysics.h"
#include "DrawObjects.h"

#include "UserData.h"

#include <GL/glut.h>

static float gPlaneData[]={
    -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f
};

static float gCylinderData[]={
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,
	0.866025f,0.500000f,1.0f,0.866025f,0.500000f,1.0f,0.866025f,0.500000f,0.0f,0.866025f,0.500000f,0.0f,
	0.500000f,0.866025f,1.0f,0.500000f,0.866025f,1.0f,0.500000f,0.866025f,0.0f,0.500000f,0.866025f,0.0f,
	-0.0f,1.0f,1.0f,-0.0f,1.0f,1.0f,-0.0f,1.0f,0.0f,-0.0f,1.0f,0.0f,
	-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,0.0f,-0.500000f,0.866025f,0.0f,
	-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,0.0f,-0.866025f,0.500000f,0.0f,
	-1.0f,-0.0f,1.0f,-1.0f,-0.0f,1.0f,-1.0f,-0.0f,0.0f,-1.0f,-0.0f,0.0f,
	-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,0.0f,-0.866025f,-0.500000f,0.0f,
	-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,0.0f,-0.500000f,-0.866025f,0.0f,
	0.0f,-1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,
	0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,0.0f,0.500000f,-0.866025f,0.0f,
	0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,0.0f,0.866026f,-0.500000f,0.0f,
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f
};

static float gCylinderDataCapsTop[]={
	0.866026f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,-1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,-1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-1.000000f,-0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-1.000000f,-0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	1.000000f,0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	1.000000f,0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.866026f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
};

static float gCylinderDataCapsBottom[]={
	1.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.000000f,1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.000000f,1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-1.000000f,-0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-1.000000f,-0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,-1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,-1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866026f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866026f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	1.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
};

static void RenderPlane()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gPlaneData);
    glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gPlaneData+3);
	glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

static void RenderBox()
{
	glutSolidCube(2);
}

static void RenderSphere()
{
	glutSolidSphere(1.0f, 12, 12);
}

static void RenderCylinder()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderData);
    glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderData+3);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 13*2);

    glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsTop);
    glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsTop+3);
	glDrawArrays(GL_TRIANGLES, 0, 36);

    glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsBottom);
    glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsBottom+3);
	glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void SetupGLMatrix(const NxVec3& pos, const NxMat33& orient)
{
	float glmat[16];	//4x4 column major matrix for OpenGL.
	orient.getColumnMajorStride4(&(glmat[0]));
	pos.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));
}

void DrawLine(const NxVec3& p0, const NxVec3& p1, const NxVec3& color, float lineWidth)
{
	glDisable(GL_LIGHTING);
	glLineWidth(lineWidth);
	glColor4f(color.x, color.y, color.z, 1.0f);
	NxVec3 av3LineEndpoints[] = {p0, p1};
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(NxVec3), &av3LineEndpoints[0].x);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}

void DrawTriangle(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2, const NxVec3& color)
{
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	NxVec3 av3LineEndpoints[] = {p0, p1, p2};
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(NxVec3), &av3LineEndpoints[0].x);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}

void DrawCircle(NxU32 nbSegments, const NxMat34& matrix, const NxVec3& color, const NxF32 radius, const bool semicircle)
{
	NxF32 step = NxTwoPiF32/NxF32(nbSegments);
	NxU32 segs = nbSegments;
	if(semicircle)
	{
		segs /= 2;
	}

	for(NxU32 i=0;i<segs;i++)
	{
		NxU32 j=i+1;
		if(j==nbSegments)	j=0;

		NxF32 angle0 = NxF32(i)*step;
		NxF32 angle1 = NxF32(j)*step;

		NxVec3 p0,p1;
		matrix.multiply(NxVec3(radius * sinf(angle0), radius * cosf(angle0), 0.0f), p0);
		matrix.multiply(NxVec3(radius * sinf(angle1), radius * cosf(angle1), 0.0f), p1);

		DrawLine(p0, p1, color);
	}
}

void DrawEllipse(NxU32 nbSegments, const NxMat34& matrix, const NxVec3& color, const NxF32 radius1, const NxF32 radius2, const bool semicircle)
{
	NxF32 step = NxTwoPiF32/NxF32(nbSegments);
	NxU32 segs = nbSegments;
	if(semicircle)
	{
		segs /= 2;
	}

	for(NxU32 i=0;i<segs;i++)
	{
		NxU32 j=i+1;
		if(j==nbSegments)	j=0;

		NxF32 angle0 = NxF32(i)*step;
		NxF32 angle1 = NxF32(j)*step;

		NxVec3 p0,p1;
		matrix.multiply(NxVec3(radius1 * sinf(angle0), radius2 * cosf(angle0), 0.0f), p0);
		matrix.multiply(NxVec3(radius1 * sinf(angle1), radius2 * cosf(angle1), 0.0f), p1);

		DrawLine(p0, p1, color);
	}
}

void DrawWirePlane(NxShape* plane, const NxVec3& color)
{
}

void DrawPlane(NxShape* plane)
{
	NxMat34 pose =	plane->getGlobalPose();

	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
	pose.t.y -= 0.1f;
	SetupGLMatrix(pose.t, pose.M);
	glScalef(1024,0,1024);
	RenderPlane();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void DrawWireBox(const NxBox& obb, const NxVec3& color, float lineWidth)
{
	// Compute obb vertices
	NxVec3 pp[8];
	NxComputeBoxPoints(obb, pp);
		
	// Draw all lines
	const NxU32* Indices = NxGetBoxEdges();
	for(NxU32 i=0;i<12;i++)
	{
		NxU32 VRef0 = *Indices++;
		NxU32 VRef1 = *Indices++;
		DrawLine(pp[VRef0], pp[VRef1], color, lineWidth);
	}
}

void DrawWireBox(NxShape* box, const NxVec3& color, float lineWidth)
{
	NxBox obb;
	box->isBox()->getWorldOBB(obb);

	DrawWireBox(obb, color, lineWidth);
}

void DrawBox(NxShape* box)
{
	NxMat34 pose = box->getGlobalPose();

	glPushMatrix();
	SetupGLMatrix(pose.t, pose.M);
	NxVec3 boxDim = box->isBox()->getDimensions();
	glScalef(boxDim.x, boxDim.y, boxDim.z);
	RenderBox();
	glPopMatrix();
}

void DrawWireSphere(NxShape* sphere, const NxVec3& color)
{
	NxMat34 pose = sphere->getGlobalPose();

	glPushMatrix();
	NxReal r = sphere->isSphere()->getRadius();

	NxVec3 c0;	pose.M.getColumn(0, c0);
	NxVec3 c1;	pose.M.getColumn(1, c1);
	NxVec3 c2;	pose.M.getColumn(2, c2);
	DrawCircle(20, pose, color, r);

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	DrawCircle(20, pose, color, r);

	pose.M.setColumn(0, c2);
	pose.M.setColumn(1, c0);
	pose.M.setColumn(2, c1);
	DrawCircle(20, pose, color, r);

	glPopMatrix();
}

// Draw Test Sphere
void DrawWireSphere(NxSphere* sphere, const NxVec3& color)
{
	NxMat34 pose;

	pose.t = sphere->center;

	glPushMatrix();
	NxReal r = sphere->radius;

	NxVec3 c0;	pose.M.getColumn(0, c0);
	NxVec3 c1;	pose.M.getColumn(1, c1);
	NxVec3 c2;	pose.M.getColumn(2, c2);
	DrawCircle(20, pose, color, r);

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	DrawCircle(20, pose, color, r);

	pose.M.setColumn(0, c2);
	pose.M.setColumn(1, c0);
	pose.M.setColumn(2, c1);
	DrawCircle(20, pose, color, r);

	glPopMatrix();
}

void DrawSphere(NxShape* sphere)
{
	NxMat34 pose = sphere->getGlobalPose();

	glPushMatrix();
	SetupGLMatrix(pose.t, pose.M);
	NxReal r = sphere->isSphere()->getRadius();
	glScalef(r,r,r);
	RenderSphere();
	glPopMatrix();
}

void DrawWireCapsule(NxShape* capsule, const NxVec3& color)
{
	NxMat34 pose = capsule->getGlobalPose();

	NxReal r = capsule->isCapsule()->getRadius();
	NxReal h = capsule->isCapsule()->getHeight();

	NxSegment SG;
	pose.M.getColumn(1, SG.p1);
	SG.p1 *= 0.5f*h;
	SG.p0 = -SG.p1;
	SG.p0 += pose.t;
	SG.p1 += pose.t;

	NxVec3 c0;	pose.M.getColumn(0, c0);
	NxVec3 c1;	pose.M.getColumn(1, c1);
	NxVec3 c2;	pose.M.getColumn(2, c2);
	DrawLine(SG.p0 + c0*r, SG.p1 + c0*r, color);
	DrawLine(SG.p0 - c0*r, SG.p1 - c0*r, color);
	DrawLine(SG.p0 + c2*r, SG.p1 + c2*r, color);
	DrawLine(SG.p0 - c2*r, SG.p1 - c2*r, color);

	pose.M.setColumn(0, -c1);
	pose.M.setColumn(1, -c0);
	pose.M.setColumn(2, c2);
	pose.t = SG.p0;
	DrawCircle(20, pose, color, r, true);	//halfcircle -- flipped

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, -c0);
	pose.M.setColumn(2, c2);
	pose.t = SG.p1;
	DrawCircle(20, pose, color, r, true);

	pose.M.setColumn(0, -c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	pose.t = SG.p0;
	DrawCircle(20, pose, color, r, true);//halfcircle -- good

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	pose.t = SG.p1;
	DrawCircle(20, pose, color, r, true);

	pose.M.setColumn(0, c2);
	pose.M.setColumn(1, c0);
	pose.M.setColumn(2, c1);
	pose.t = SG.p0;
	DrawCircle(20, pose, color, r);	//full circle
	pose.t = SG.p1;
	DrawCircle(20, pose, color, r);
}

static void computeBasis(const NxVec3& dir, NxVec3& right, NxVec3& up)
{
	// Derive two remaining vectors
	if(fabsf(dir.y)>0.9999f)	right = NxVec3(1.0f, 0.0f, 0.0f);
	else						right = (NxVec3(0.0f, 1.0f, 0.0f) ^ dir);
	right.normalize();
	up = dir ^ right;
}

void DrawWireCapsule(const NxCapsule& capsule, const NxVec3& color)
{
	NxReal r = capsule.radius;
	NxVec3 dir = capsule.p0 - capsule.p1;
	NxReal h = dir.magnitude();
	NxMat34 pose;
	pose.t = (capsule.p0 + capsule.p1)*0.5f;

	if(h!=0.0f)
	{
		dir/=h;
		NxVec3 right, up;
		computeBasis(dir, right, up);
		pose.M.setColumn(0, right);
		pose.M.setColumn(1, dir);
		pose.M.setColumn(2, up);
	}
	else
	{
		pose.M.id();
	}

//	NxMat34 pose = capsule->getGlobalPose();
//	const NxReal & r = capsule->isCapsule()->getRadius();
//	const NxReal & h = capsule->isCapsule()->getHeight();

	NxSegment SG;
	pose.M.getColumn(1, SG.p1);
	SG.p1 *= 0.5f*h;
	SG.p0 = -SG.p1;
	SG.p0 += pose.t;
	SG.p1 += pose.t;

	NxVec3 c0;	pose.M.getColumn(0, c0);
	NxVec3 c1;	pose.M.getColumn(1, c1);
	NxVec3 c2;	pose.M.getColumn(2, c2);
	DrawLine(SG.p0 + c0*r, SG.p1 + c0*r, color);
	DrawLine(SG.p0 - c0*r, SG.p1 - c0*r, color);
	DrawLine(SG.p0 + c2*r, SG.p1 + c2*r, color);
	DrawLine(SG.p0 - c2*r, SG.p1 - c2*r, color);

	pose.M.setColumn(0, -c1);
	pose.M.setColumn(1, -c0);
	pose.M.setColumn(2, c2);
	pose.t = SG.p0;
	DrawCircle(20, pose, color, r, true);	//halfcircle -- flipped

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, -c0);
	pose.M.setColumn(2, c2);
	pose.t = SG.p1;
	DrawCircle(20, pose, color, r, true);

	pose.M.setColumn(0, -c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	pose.t = SG.p0;
	DrawCircle(20, pose, color, r, true);//halfcircle -- good

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	pose.t = SG.p1;
	DrawCircle(20, pose, color, r, true);

	pose.M.setColumn(0, c2);
	pose.M.setColumn(1, c0);
	pose.M.setColumn(2, c1);
	pose.t = SG.p0;
	DrawCircle(20, pose, color, r);	//full circle
	pose.t = SG.p1;
	DrawCircle(20, pose, color, r);
}

void DrawCapsule(NxShape* capsule)
{
	NxMat34 pose = capsule->getGlobalPose();

	const NxReal & r = capsule->isCapsule()->getRadius();
	const NxReal & h = capsule->isCapsule()->getHeight();

	glPushMatrix();
	SetupGLMatrix(pose.t, pose.M);

	glPushMatrix();
	glTranslatef(0.0f, h*0.5f, 0.0f);
	glScalef(r,r,r);
	RenderSphere();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f,-h*0.5f, 0.0f);
	glScalef(r,r,r);
	RenderSphere();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f,h*0.5f, 0.0f);
	glScalef(r,h,r);
	glRotatef(90.0f,1.0f,0.0f,0.0f);
	RenderCylinder();
	glPopMatrix();

	glPopMatrix();
}

void DrawCapsule(const NxVec3& color, NxF32 r, NxF32 h)
{
	glColor4f(color.x, color.y, color.z, 1.0f);

	glPushMatrix();
	glTranslatef(0.0f, h*0.5f, 0.0f);
	glScalef(r,r,r);
	RenderSphere();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f,-h*0.5f, 0.0f);
	glScalef(r,r,r);
	RenderSphere();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f,h*0.5f, 0.0f);
	glScalef(r,h,r);
	glRotatef(90.0f,1.0f,0.0f,0.0f);
	RenderCylinder();
	glPopMatrix();
}


typedef NxVec3 Point;
typedef struct _Triangle { NxU32 p0; NxU32 p1; NxU32 p2; } Triangle;

void DrawWireConvex(NxShape* mesh, const NxVec3& color, bool useShapeUserData)
{
	if(mesh->userData == NULL) return;

	NxMat34 pose = mesh->getGlobalPose();

	void* ptr = NULL;
	if (useShapeUserData) {
		ShapeUserData* sud = (ShapeUserData*)(mesh->userData);
		ptr = sud->mesh;
		if (!(sud && sud->mesh))  return;
	} else {
		ptr = mesh->userData;
	}

	NxConvexMeshDesc meshDesc = *((NxConvexMeshDesc*)(ptr));
//	mesh->isConvexMesh()->getConvexMesh().saveToDesc(meshDesc);

	NxU32 nbVerts = meshDesc.numVertices;
	NxU32 nbTriangles = meshDesc.numTriangles;

	Point* points = (Point *)meshDesc.points;
	Triangle* triangles = (Triangle *)meshDesc.triangles;

	glPushMatrix();

	float glmat[16];	//4x4 column major matrix for OpenGL.
	pose.M.getColumnMajorStride4(&(glmat[0]));
	pose.t.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));

	while(nbTriangles--)
	{
		DrawLine(points[triangles->p0], points[triangles->p1], color);
		DrawLine(points[triangles->p1], points[triangles->p2], color);
		DrawLine(points[triangles->p2], points[triangles->p0], color);
		triangles++;
	}

	glPopMatrix();
}

void DrawTriangleList(int iTriangleCount, Triangle *pTriangles, Point *pPoints)
{
	static int iBufferSize=0;
	static float *pfVertexBuffer=NULL;
	static float *pfNormalBuffer=NULL;

	if(iBufferSize < iTriangleCount*3)
	{
		iBufferSize=3*iTriangleCount;

		delete[] pfVertexBuffer;
		pfVertexBuffer = new float[iBufferSize*3];

		delete[] pfNormalBuffer;
		pfNormalBuffer = new float[iBufferSize*3];
	}

	float *pfDestinationVertex=pfVertexBuffer;
	float *pfDestinationNormal=pfNormalBuffer;

	for(int iTriangle=0; iTriangle<iTriangleCount; iTriangle++)
	{
		*pfDestinationVertex++=pPoints[pTriangles->p0].x;
		*pfDestinationVertex++=pPoints[pTriangles->p0].y;
		*pfDestinationVertex++=pPoints[pTriangles->p0].z;
		*pfDestinationVertex++=pPoints[pTriangles->p1].x;
		*pfDestinationVertex++=pPoints[pTriangles->p1].y;
		*pfDestinationVertex++=pPoints[pTriangles->p1].z;
		*pfDestinationVertex++=pPoints[pTriangles->p2].x;
		*pfDestinationVertex++=pPoints[pTriangles->p2].y;
		*pfDestinationVertex++=pPoints[pTriangles->p2].z;

		NxVec3 edge1 = pPoints[pTriangles->p1] - pPoints[pTriangles->p0];
		NxVec3 edge2 = pPoints[pTriangles->p2] - pPoints[pTriangles->p0];
		NxVec3 normal = edge1.cross(edge2);
		normal.normalize();

		*pfDestinationNormal++=normal.x;
		*pfDestinationNormal++=normal.y;
		*pfDestinationNormal++=normal.z;
		*pfDestinationNormal++=normal.x;
		*pfDestinationNormal++=normal.y;
		*pfDestinationNormal++=normal.z;
		*pfDestinationNormal++=normal.x;
		*pfDestinationNormal++=normal.y;
		*pfDestinationNormal++=normal.z;

		pTriangles++;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, pfVertexBuffer);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, pfNormalBuffer);

	glDrawArrays(GL_TRIANGLES, 0, 3*iTriangleCount);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void DrawConvex(NxShape* mesh, bool useShapeUserData)
{
	NxConvexMeshDesc meshDesc;

	if(mesh->userData == NULL) return;

	void* ptr = NULL;
	if (useShapeUserData) {
		ShapeUserData* sud = (ShapeUserData*)(mesh->userData);
		ptr = sud->mesh;
		if (!(sud && sud->mesh))  return;

		meshDesc = *((NxConvexMeshDesc*)(ptr));
	} else {
		mesh->isConvexMesh()->getConvexMesh().saveToDesc(meshDesc);
	}	

	// SRM : Please note that I looked into a crash issue posed by
	// one of our customers, and this code (i.e. reinterpreting the
	// NxShape's userData as an NxConvexMeshDesc * was crashing because
	// in the SampleRaycastCar demo, it sets that pointer equal to the
	// NxWheel because that is used in NxVehicle::handleContactPair().  Thus
	// in order to allow this code not to crash on the PS3, we should
	// simply force the shape to save its description here.
	//mesh->isConvexMesh()->getConvexMesh().saveToDesc(meshDesc);
	/**
	if(mesh->userData != NULL)
	{
		meshDesc = *((NxConvexMeshDesc*)(mesh->userData));
	} else {
		mesh->isConvexMesh()->getConvexMesh().saveToDesc(meshDesc);
	}
	**/

	NxMat34 pose = mesh->getGlobalPose();

	NxU32 nbVerts = meshDesc.numVertices;
	NxU32 nbTriangles = meshDesc.numTriangles;

	Point* points = (Point *)meshDesc.points;
	Triangle* triangles = (Triangle *)meshDesc.triangles;

	glPushMatrix();

	float glmat[16];	//4x4 column major matrix for OpenGL.
	pose.M.getColumnMajorStride4(&(glmat[0]));
	pose.t.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));

	DrawTriangleList(nbTriangles, triangles, points);
	
	glPopMatrix();
}

void DrawWireMesh(NxShape* mesh, const NxVec3& color, bool useShapeUserData)
{
	if(mesh->userData == NULL) return;

	NxMat34 pose = mesh->getGlobalPose();

	void* ptr = NULL;
	if (useShapeUserData) {
		ShapeUserData* sud = (ShapeUserData*)(mesh->userData);
		ptr = sud->mesh;
		if (!(sud && sud->mesh))  return;
	} else {
		ptr = mesh->userData;
	}

	NxTriangleMeshDesc meshDesc = *((NxTriangleMeshDesc*)(ptr));
//	mesh->isTriangleMesh()->getTriangleMesh().saveToDesc(meshDesc);	
	
	NxU32 nbVerts = meshDesc.numVertices;
	NxU32 nbTriangles = meshDesc.numTriangles;

	Point* points = (Point *)meshDesc.points;
	Triangle* triangles = (Triangle *)meshDesc.triangles;

	glPushMatrix();

	float glmat[16];	//4x4 column major matrix for OpenGL.
	pose.M.getColumnMajorStride4(&(glmat[0]));
	pose.t.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));

	while(nbTriangles--)
	{
		DrawLine(points[triangles->p0], points[triangles->p1], color);
		DrawLine(points[triangles->p1], points[triangles->p2], color);
		DrawLine(points[triangles->p2], points[triangles->p0], color);
		triangles++;
	}

	glPopMatrix();
}

NxArray<NxU32>	gTouchedTris;

void DrawHeightfield(NxShape* hf)
{
	//  ShapeUserData* sud = (ShapeUserData*)(hf->userData);
	//	if (!(sud && sud->model && sud->modelType == MT_NX_MESH))  return;
	//
	//	NxMat34 pose = hf->getGlobalPose();

	//	const NxHeightFieldShape* hfs = (const NxHeightFieldShape*)gHeightField->getShapes()[0];
	const NxHeightFieldShape* hfs = (const NxHeightFieldShape*)hf;

	glBegin(GL_TRIANGLES);
	glColor4f(0.1,0.1,0.7,1);
	for (NxU32 row = 0; row < hfs->getHeightField().getNbRows() - 1; row++)
	{
		for (NxU32 column = 0; column < hfs->getHeightField().getNbColumns() - 1; column++)
		{
			NxTriangle tri;
			NxVec3 n;
			NxU32 triangleIndex;

			triangleIndex = 2 * (row * hfs->getHeightField().getNbColumns() + column);

			if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
			{
				n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
				n.normalize();
				glNormal3fv(&n.x);
				glVertex3fv(&tri.verts[0].x);
				glVertex3fv(&tri.verts[1].x);
				glVertex3fv(&tri.verts[2].x);
			}

			triangleIndex++;

			if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
			{
				n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
				n.normalize();
				glNormal3fv(&n.x);
				glVertex3fv(&tri.verts[0].x);
				glVertex3fv(&tri.verts[1].x);
				glVertex3fv(&tri.verts[2].x);
			}
		}
	}
	glEnd();

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	glColor4f(1,0,0,1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1,-2);
	glBegin(GL_TRIANGLES);
	for(NxU32 i = 0; i < gTouchedTris.size(); i++)
	{
		NxU32 triangleIndex = gTouchedTris[i];
		NxU32 flags;
		NxTriangle tri;
		NxTriangle edgeTri;
		hfs->getTriangle(tri, &edgeTri, &flags, triangleIndex, true);
		NxVec3 n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
		n.normalize();
		glNormal3fv(&n.x);
		glVertex3fv(&tri.verts[0].x);
		glVertex3fv(&tri.verts[1].x);
		glVertex3fv(&tri.verts[2].x);
	}
	glEnd();
	glPopAttrib();
	glColor4f(1,1,1,1);
}

void DrawMesh(NxShape* mesh, bool useShapeUserData)
{
	if(mesh->userData == NULL) return;

	NxMat34 pose = mesh->getGlobalPose();

	void* ptr = NULL;
	if (useShapeUserData) {
		ShapeUserData* sud = (ShapeUserData*)(mesh->userData);
		ptr = sud->mesh;
		if (!(sud && sud->mesh))  return;
	} else {
		ptr = mesh->userData;
	}

	NxTriangleMeshDesc meshDesc = *((NxTriangleMeshDesc*)(ptr));
//	mesh->isTriangleMesh()->getTriangleMesh().saveToDesc(meshDesc);	

	NxU32 nbVerts = meshDesc.numVertices;
	NxU32 nbTriangles = meshDesc.numTriangles;

	Point* points = (Point *)meshDesc.points;
	Triangle* triangles = (Triangle *)meshDesc.triangles;

	glPushMatrix();

	float glmat[16];	//4x4 column major matrix for OpenGL.
	pose.M.getColumnMajorStride4(&(glmat[0]));
	pose.t.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));

	DrawTriangleList(nbTriangles, triangles, points);
	
	glPopMatrix();
}

void DrawWheelShape(NxShape* wheel)
{
	// Get saved-away wheel shape pose to draw wheel shape at proper position
	if (wheel->getType() == NX_SHAPE_WHEEL)
	{
	    NxWheelShape* ws = (NxWheelShape *)wheel;

		float r = ws->getRadius();

		NxMat34 pose = ((ShapeUserData*)(ws->userData))->wheelShapePose;

		glPushMatrix();

		float glmat[16];	//4x4 column major matrix for OpenGL.
		pose.M.getColumnMajorStride4(&(glmat[0]));
		pose.t.get(&(glmat[12]));

		//clear the elements we don't need:
		glmat[3] = glmat[7] = glmat[11] = 0.0f;
		glmat[15] = 1.0f;

		glMultMatrixf(&(glmat[0]));

		glRotatef(90,0,1,0);
		glTranslatef(0,0,-r/2);
		glScalef(r, r, r);
		RenderCylinder();

	    glPopMatrix();
	}
}

void DrawArrow(const NxVec3& posA, const NxVec3& posB, const NxVec3& color)
{
	NxVec3 vec = posB - posA;
	NxVec3 t0, t1, t2;
	NxNormalToTangents(vec, t1, t2);

	t0 = posB - posA;
	t0.normalize();

	NxVec3 lobe1  = posB - t0*0.15 + t1 * 0.15;
	NxVec3 lobe2  = posB - t0*0.15 - t1 * 0.15;
	NxVec3 lobe3  = posB - t0*0.15 + t2 * 0.15;
	NxVec3 lobe4  = posB - t0*0.15 - t2 * 0.15;

	NxVec3 v3ArrowShape[] = {
		NxVec3(posA), NxVec3(posB),
		NxVec3(posB), NxVec3(lobe1),
		NxVec3(posB), NxVec3(lobe2),
		NxVec3(posB), NxVec3(lobe3),
		NxVec3(posB), NxVec3(lobe4),
	};

	glDisable(GL_LIGHTING);
	glLineWidth(3.0f);
	glColor4f(color.x,color.y,color.z,1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(NxVec3), &v3ArrowShape[0].x);
	glDrawArrays(GL_LINES, 0, sizeof(v3ArrowShape)/sizeof(NxVec3));
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

void DrawContactPoint(const NxVec3& pos, const NxReal radius, const NxVec3& color)
{
	NxMat34 pose;
	pose.t = pos;

	NxVec3 c0;	pose.M.getColumn(0, c0);
	NxVec3 c1;	pose.M.getColumn(1, c1);
	NxVec3 c2;	pose.M.getColumn(2, c2);
	DrawCircle(20, pose, color, radius);

	pose.M.setColumn(0, c1);
	pose.M.setColumn(1, c2);
	pose.M.setColumn(2, c0);
	DrawCircle(20, pose, color, radius);

	pose.M.setColumn(0, c2);
	pose.M.setColumn(1, c0);
	pose.M.setColumn(2, c1);
	DrawCircle(20, pose, color, radius);
}

void DrawWireShape(NxShape *shape, const NxVec3& color, bool useShapeUserData)
{
    switch(shape->getType())
    {
		case NX_SHAPE_PLANE:
			DrawWirePlane(shape, color);
		break;
		case NX_SHAPE_BOX:
			DrawWireBox(shape, color);
		break;
		case NX_SHAPE_SPHERE:
			DrawWireSphere(shape, color);
		break;
		case NX_SHAPE_CAPSULE:
			DrawWireCapsule(shape, color);
		break;
		case NX_SHAPE_CONVEX:
			DrawWireConvex(shape, color, useShapeUserData);
		break;		
		case NX_SHAPE_MESH:
			DrawWireMesh(shape, color, useShapeUserData);
			break;
	}
}

void DrawShape(NxShape* shape, bool useShapeUserData)
{
    switch(shape->getType())
    {
		case NX_SHAPE_PLANE:
			DrawPlane(shape);
		break;
		case NX_SHAPE_BOX:
			DrawBox(shape);
		break;
		case NX_SHAPE_SPHERE:
			DrawSphere(shape);
		break;
		case NX_SHAPE_CAPSULE:
			DrawCapsule(shape);
		break;
		case NX_SHAPE_CONVEX:
			DrawConvex(shape, useShapeUserData);
		break;
		case NX_SHAPE_MESH:
			DrawMesh(shape, useShapeUserData);
		break;
		case NX_SHAPE_WHEEL:
			DrawWheelShape(shape);
			break;
		case NX_SHAPE_HEIGHTFIELD:
			DrawHeightfield(shape);
			break;
	}
}

void DrawActor(NxActor* actor, NxActor* selectedActor, bool useShapeUserData)
{
	// We render some actors using light source 1 instead of light source 0
	ActorUserData* ud;
	ud = (ActorUserData*)(actor->userData);
    if (ud && (ud->flags & UD_RENDER_USING_LIGHT1))
	{
	    glDisable(GL_LIGHT0);
	    glEnable(GL_LIGHT1);
	}

	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		DrawShape(shapes[nShapes], useShapeUserData);
	}

	// We render some actors using light source 1 instead of light source 0
	ud = (ActorUserData*)(actor->userData);
    if (ud && (ud->flags & UD_RENDER_USING_LIGHT1))
	{
	    glDisable(GL_LIGHT1);
	    glEnable(GL_LIGHT0);
	}

	nShapes = actor->getNbShapes();
	if (actor == selectedActor) 
	{
		while (nShapes--)
		{
			DrawWireShape(shapes[nShapes], NxVec3(1,1,1), useShapeUserData);
		}
	}
}

static void DrawActorShadow(NxActor* actor, const float* ShadowMat, bool useShapeUserData)
{
	glPushMatrix();
	glMultMatrixf(ShadowMat);

	glDisable(GL_LIGHTING);
	glColor4f(0.05f, 0.1f, 0.15f, 1.0f);
	
	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		switch(shapes[nShapes]->getType())
		{
		    case NX_SHAPE_BOX:
			    DrawBox(shapes[nShapes]);
			break;
		    case NX_SHAPE_SPHERE:
			    DrawSphere(shapes[nShapes]);
			break;
		    case NX_SHAPE_CAPSULE:
			    DrawCapsule(shapes[nShapes]);
			break;
		    case NX_SHAPE_CONVEX:
			    DrawConvex(shapes[nShapes], useShapeUserData);
			break;		    
		}
	}	
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);

	glPopMatrix();
}


void DrawActorShadow(NxActor* actor, bool useShapeUserData)
{
	const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
	DrawActorShadow(actor, ShadowMat, useShapeUserData);
}

void DrawActorShadow2(NxActor* actor, bool useShapeUserData)
{
    const static float ShadowMat[]={ 1,0,0,0, 1,0,-0.2,0, 0,0,1,0, 0,0,0,1 };
	DrawActorShadow(actor, ShadowMat, useShapeUserData);
}

void DrawActorShadowZUp(NxActor* actor, bool useShapeUserData)
{
    const static float ShadowMat[]={ 1,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,1 };
	DrawActorShadow(actor, ShadowMat, useShapeUserData);
}

void DrawCloth(NxCloth* cloth, bool shadows)
{
	NxMeshData meshData = cloth->getMeshData();

	NxU32 numVertices = *meshData.numVerticesPtr;
	NxU32 numTriangles = *meshData.numIndicesPtr / 3;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, meshData.verticesPosBegin);
	glNormalPointer(GL_FLOAT, 0, meshData.verticesNormalBegin);

#ifdef __CELLOS_LV2__	
	glDrawRangeElements(GL_TRIANGLES, 0, numVertices-1, 3*numTriangles, GL_UNSIGNED_INT, meshData.indicesBegin);
#else
	glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT, meshData.indicesBegin);
#endif

	if (shadows) {
		const static float ShadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
		glPushMatrix();
		glMultMatrixf(ShadowMat);
		glDisable(GL_LIGHTING);
		glColor4f(0.05f, 0.1f, 0.15f,1.0f);

#ifdef __CELLOS_LV2__	
		glDrawRangeElements(GL_TRIANGLES, 0, numVertices-1, 3*numTriangles, GL_UNSIGNED_INT, meshData.indicesBegin);
#else
		glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT, meshData.indicesBegin);
#endif
		
		glColor4f(1.0f, 1.0f, 1.0f,1.0f);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void DrawActorShapeSelect(NxActor *actor, NxShape *shape, const bool shapeSelectMode, NxActor* selectedActor, bool useShapeUserData)
{
	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	if (actor == selectedActor) 
	{
		while (nShapes--)
		{
			if (shapeSelectMode)
			{
				if (shape && shape == shapes[nShapes])
					DrawWireShape(shapes[nShapes], NxVec3(1,0,0), useShapeUserData);
				else 		
					DrawWireShape(shapes[nShapes], NxVec3(1,1,1), useShapeUserData);
			}
			else
			{
				DrawWireShape(shapes[nShapes], NxVec3(1,1,1), useShapeUserData);
			}
		}
	}
	nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		DrawShape(shapes[nShapes], useShapeUserData);
	}
}


