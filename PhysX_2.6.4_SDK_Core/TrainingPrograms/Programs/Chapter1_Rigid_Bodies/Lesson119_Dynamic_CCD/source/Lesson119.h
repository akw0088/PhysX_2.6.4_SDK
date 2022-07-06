// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			      LESSON 119: DYNAMIC CONTINUOUS COLLISION DETECTION
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON119_H
#define LESSON119_H

#include "CommonCode.h"

void PrintControls();

void CreateStack(int size);
void CreateTower(int size);
NxCCDSkeleton* CreateCCDSkeleton(float size);
NxActor* CreateCCDBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density, bool doDynamicCCD);

int main(int argc, char** argv);

#endif  // LESSON119_H



