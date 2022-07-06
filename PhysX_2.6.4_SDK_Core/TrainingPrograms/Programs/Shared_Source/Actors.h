// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						 	   ACTOR CREATION METHODS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef ACTORS_H
#define ACTORS_H

#include "NxPhysics.h"

#include <stdio.h>
#include "Stream.h"
#include "NxCooking.h"

NxActor* CreateGroundPlane();
NxActor* CreateBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateSphere(const NxVec3& pos, const NxReal radius, const NxReal density);
NxActor* CreateCapsule(const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density);

NxActor* CreateHalfPyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreatePyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateDownWedge(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);

NxActor* CreateFlatSurface(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride);
NxActor* CreateFlatHeightfield(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride);

NxActor* CreateConvexObjectComputeHull(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateConvexObjectSupplyHull(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateConcaveObject(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);

NxActor* CreateBunny(const NxVec3& pos, NxTriangleMesh* triangleMesh, const NxReal density);

NxActor** CreateStack(const NxVec3& pos, const NxVec3& stackDim, const NxVec3& boxDim, NxReal density);
NxActor** CreateTower(const NxVec3& pos, const NxU32 heightBoxes, const NxVec3& boxDim, NxReal density);

NxQuat AnglesToQuat(const NxVec3& angles);
NxActor* CreateBoxGear(const NxVec3& pos, const NxReal minRadius, const NxReal maxRadius, const NxReal height, const NxU32 numTeeth, const NxReal density);
NxActor* CreateWheel(const NxVec3& pos, const NxReal minRadius, const NxReal maxRadius, const NxReal height, const NxU32 numTeeth, const NxReal density);
NxActor* CreateFrame(const NxVec3& pos, const NxReal density);
NxActor* CreateStep(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);

NxActor* CreateChassis(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateTurret(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateCannon(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);

NxActor* CreateBlade(const NxVec3& pos, const NxVec3& boxDim, const NxReal mass);
NxActor* CreateBall(const NxVec3& pos, const NxReal radius, const NxReal mass);

void SetActorCollisionGroup(NxActor *actor, NxCollisionGroup group);
void SetActorMaterial(NxActor *actor, NxMaterialIndex index);

void PageInHardwareMeshes(NxScene* scene);
void SwitchCoordinateSystem(NxScene* scene, NxMat34 mat);
#endif  // ACTORS_H
