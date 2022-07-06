// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			               LESSON 110: COLLISION FILTERING
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON110_H
#define LESSON110_H

#include "CommonCode.h"

void PrintControls();
NxActor* CreateFilteredGroundPlane();
NxActor* CreateFilteredBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor** CreateFilteredStack(const NxVec3& pos, const NxVec3& stackDim, const NxVec3& boxDim, NxReal density);
int main(int argc, char** argv);

#endif  // LESSON110_H



