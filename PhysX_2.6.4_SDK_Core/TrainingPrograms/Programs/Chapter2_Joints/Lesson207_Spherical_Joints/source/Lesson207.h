// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    LESSON 207: SPHERICAL JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON207_H
#define LESSON207_H

#include "CommonCode.h"

void PrintControls();
NxSphericalJoint* CreateSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
int main(int argc, char** argv);

#endif  // LESSON207_H
