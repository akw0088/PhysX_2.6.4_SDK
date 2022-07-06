// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						   LESSON 209: CYLINDRICAL JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON209_H
#define LESSON209_H

#include "CommonCode.h"

void PrintControls();
NxCylindricalJoint* CreateCylindricalJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON209_H
