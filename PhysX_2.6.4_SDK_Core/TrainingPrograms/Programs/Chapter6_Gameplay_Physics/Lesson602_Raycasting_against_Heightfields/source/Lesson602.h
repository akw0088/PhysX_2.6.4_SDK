// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					LESSON 602: RAYCASTING AGAINST HEIGHTFIELDS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON602_H
#define LESSON602_H

#include "CommonCode2.h"
#include "Heightfield.h"
#include "RaycastReport.h"

void PrintControls();
void AddTerrainLight();
void RaycastClosestShapeFromActor(NxActor* actor, NxU32 groupFlag);
void RaycastAllShapesFromActor(NxActor* actor, NxU32 groupFlag);
int main(int argc, char** argv);

#endif  // LESSON602_H
