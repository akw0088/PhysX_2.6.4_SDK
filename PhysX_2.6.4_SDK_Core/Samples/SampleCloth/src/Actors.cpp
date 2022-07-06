/*----------------------------------------------------------------------------*\
|
|						     AGEIA PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

//Actor creation methods for sample programs

#include "Actors.h"

#include <stdio.h>
#include "NxCooking.h"
#include "Stream.h"

extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxScene*          gScene2;

// GEOMETRIC SHAPE ACTORS: PLANE, BOX, SPHERE, AND CAPSULE
NxActor* CreateGroundPlane()
{
    // Create a plane with default descriptor
    NxPlaneShapeDesc planeDesc;
    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack(&planeDesc);
    return gScene->createActor(actorDesc);
}

NxActor* CreateBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a box
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(boxDim.x,boxDim.y,boxDim.z);
	boxDesc.localPose.t = NxVec3(0,boxDim.y,0);
	actorDesc.shapes.pushBack(&boxDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateSphere(const NxVec3& pos, const NxReal radius, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a sphere
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.localPose.t = NxVec3(0,radius,0);
	actorDesc.shapes.pushBack(&sphereDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateCapsule(const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a capsule
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.height = height;
	capsuleDesc.radius = radius;
	capsuleDesc.localPose.t = NxVec3(0,radius+(NxReal)0.5*height,0);
	actorDesc.shapes.pushBack(&capsuleDesc);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}


NxActor** CreateStack(const NxVec3& pos, const NxVec3& stackDim, const NxVec3& boxDim, NxReal density)
{
	NxActor** stack = new NxActor*[(int)(8*stackDim.x*stackDim.y*stackDim.z)]; 

	NxVec3 offset = NxVec3(boxDim.x,0,boxDim.z) + pos;

	for (int i = -(int)stackDim.x; i < (int)stackDim.x; i++)
		for (int j = 0; j < (int)stackDim.y; j++)
			for (int k = -(int)stackDim.z; k < (int)stackDim.z; k++)
			{
				NxVec3 boxPos = NxVec3(i*boxDim.x*2,j*boxDim.y*2,k*boxDim.z*2);
				*stack++ = CreateBox(boxPos + offset, boxDim, density);
			}

	return stack;
}


NxActor** CreateTower(const NxVec3& pos, const int heightBoxes, const NxVec3& boxDim, NxReal density)
{
	NxActor** tower = new NxActor*[heightBoxes]; 
	
	float spacing = 0.01f;
	
	NxVec3 boxPos = pos;
	//boxPos.y = pos.y + boxDim.y;
	
	for (int i = 0; i < heightBoxes; i++)
	{
	      tower[i] = CreateBox(boxPos, boxDim, density);
		  boxPos.y += 2.0f * boxDim.y + spacing;
	}
	
  return tower;	 
}

NxQuat AnglesToQuat(const NxVec3& angles)
{
	NxQuat quat;
	NxVec3 a;
	NxReal cr, cp, cy, sr, sp, sy, cpcy, spsy;

	a.x = (NxPi/(NxReal)360.0) * angles.x;    // Pitch
	a.y = (NxPi/(NxReal)360.0) * angles.y;    // Yaw
	a.z = (NxPi/(NxReal)360.0) * angles.z;    // Roll

	cy = NxMath::cos(a.z);
	cp = NxMath::cos(a.y);
	cr = NxMath::cos(a.x);

	sy = NxMath::sin(a.z);
	sp = NxMath::sin(a.y);
	sr = NxMath::sin(a.x);

	cpcy = cp * cy;
	spsy = sp * sy;
	quat.w = cr * cpcy + sr * spsy;		
	quat.x = sr * cpcy - cr * spsy;		
	quat.y = cr * sp * cy + sr * cp * sy;	
	quat.z = cr * cp * sy - sr * sp * cy;

	return quat;
}




NxActor* CreateFrame(const NxVec3& pos, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxBoxShapeDesc boxDesc1;
	boxDesc1.dimensions.set(8,(NxReal)0.5,(NxReal)0.05);
	boxDesc1.localPose.t = NxVec3(0,0,(NxReal)1.7);
	actorDesc.shapes.pushBack(&boxDesc1);

	NxBoxShapeDesc boxDesc2;
	boxDesc2.dimensions.set(8,(NxReal)0.5,(NxReal)0.05);
	boxDesc2.localPose.t = NxVec3(0,0,-(NxReal)1.7);
	actorDesc.shapes.pushBack(&boxDesc2);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

NxActor* CreateStep(const NxVec3& pos, const NxVec3& boxDim, const NxReal density)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxBoxShapeDesc boxDesc1;
	boxDesc1.dimensions.set(boxDim.x/2,boxDim.y/2,boxDim.z/2);
	boxDesc1.localPose.t = NxVec3(0,0,0);
	actorDesc.shapes.pushBack(&boxDesc1);

	NxBoxShapeDesc boxDesc2;
	boxDesc2.dimensions.set(boxDim.x/6,boxDim.y/16,(NxReal)0.125);
	boxDesc2.localPose.t = NxVec3(0,0,-(NxReal)0.1);
	actorDesc.shapes.pushBack(&boxDesc2);

	NxBoxShapeDesc boxDesc3;
	boxDesc3.dimensions.set((NxReal)0.1,boxDim.y/2,(NxReal)0.5);
	boxDesc3.localPose.t = NxVec3(boxDim.x/2,0,(NxReal)(0.201-0.25));
	actorDesc.shapes.pushBack(&boxDesc3);

	NxBoxShapeDesc boxDesc4;
	boxDesc4.dimensions.set((NxReal)0.1,boxDim.y/2,(NxReal)0.5);
	boxDesc4.localPose.t = NxVec3(-boxDim.x/2,0,(NxReal)(0.201-0.25));
	actorDesc.shapes.pushBack(&boxDesc4);

	if (density)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = density;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}

void SetActorGroup(NxActor *actor, NxCollisionGroup group)
{
	NxU32 nbShapes = actor->getNbShapes();
	NxShape * const*shapes = actor->getShapes();

	while (nbShapes--)
	{
		shapes[nbShapes]->setGroup(group);
	}
}

void SetActorMaterial(NxActor *actor, NxMaterialIndex index)
{
	NxU32 nbShapes = actor->getNbShapes();
	NxShape * const*shapes = actor->getShapes();

	while (nbShapes--)
	{
		shapes[nbShapes]->setMaterial(index);
	}
}


NxActor* CreateBall(const NxVec3& pos, const NxReal radius, const NxReal mass)
{
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	// The actor has one shape, a sphere
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.localPose.t = NxVec3(0,radius,0);
	actorDesc.shapes.pushBack(&sphereDesc);

	if (mass)
	{
		bodyDesc.mass = mass;
		actorDesc.body = &bodyDesc;
	}
	else
	{
		actorDesc.body = NULL;
	}
	actorDesc.globalPose.t = pos;
	return gScene->createActor(actorDesc);	
}


