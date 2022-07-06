// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//		                       LESSON 305: RAYCASTING
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON305_H
#define LESSON305_H

#include "CommonCode.h"
#include "RaycastReport.h"
#include "UserData.h"

void PrintControls();
void RaycastClosestShapeFromActor(NxActor* actor, NxU32 groupFlag);
void RaycastAllShapesFromActor(NxActor* actor, NxU32 groupFlag);
void SetActorCollisionGroup(NxActor* actor, NxCollisionGroup group);
int main(int argc, char** argv);

#endif  // LESSON305_H



