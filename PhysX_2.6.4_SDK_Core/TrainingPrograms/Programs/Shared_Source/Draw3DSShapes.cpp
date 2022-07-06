// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							   SHAPE DRAWING ROUTINES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "NxPhysics.h"
#include "DrawObjects.h"

#include "GLTexture.h"
#include "Model_3DS.h"

#include "UserData.h"

#include <GL/glut.h>

extern NxActor* gSelectedActor;

// Draw 3ds mesh
void Draw3DSMesh(NxShape* shape, Model_3DS* model)
{
	NxMat34 pose;
	pose = shape->getGlobalPose();

	// Get saved-away wheel shape pose to draw wheel shape at proper position
	if (shape->getType() == NX_SHAPE_WHEEL)
	{
	    NxWheelShape* ws = (NxWheelShape *)shape;
		pose = ((ShapeUserData*)(ws->userData))->wheelShapePose;
	}

	glPushMatrix();

	float glmat[16];	//4x4 column major matrix for OpenGL.
	pose.M.getColumnMajorStride4(&(glmat[0]));
	pose.t.get(&(glmat[12]));

	//clear the elements we don't need:
	glmat[3] = glmat[7] = glmat[11] = 0.0f;
	glmat[15] = 1.0f;

	glMultMatrixf(&(glmat[0]));

	model->Draw();

	glPopMatrix();
	glDisable( GL_TEXTURE_2D );
}

void Draw3DSMeshActor(NxActor *actor)
{
	NxShape*const* shapes = actor->getShapes();
	NxU32 nShapes = actor->getNbShapes();
	while (nShapes--)
	{
		ShapeUserData* sud = (ShapeUserData*)(shapes[nShapes]->userData);
		if (sud && sud->model)
			Draw3DSMesh(shapes[nShapes], ((Model_3DS*)(sud->model)));  //only set one shape in 3ds model.
		else
		    DrawShape(shapes[nShapes], true);
	}
	nShapes = actor->getNbShapes();
	if (gSelectedActor && actor == gSelectedActor) 
	{
		while (nShapes--)
		{
			if (shapes[nShapes]->getFlag(NX_TRIGGER_ENABLE))
				DrawWireShape(shapes[nShapes], NxVec3(0,0,1), true);
			else
				DrawWireShape(shapes[nShapes], NxVec3(1,1,1), true);
		}
	}
}

