// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						  LESSON 211: POINT-IN-PLANE JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON211_H
#define LESSON211_H

#include "CommonCode.h"

void PrintControls();
NxActor* CreateRegularPyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxPointInPlaneJoint* CreatePointInPlaneJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON211_H
