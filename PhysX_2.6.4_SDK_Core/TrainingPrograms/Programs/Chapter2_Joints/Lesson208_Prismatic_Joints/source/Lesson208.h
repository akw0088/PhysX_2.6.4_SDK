// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    LESSON 208: PRISMATIC JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON208_H
#define LESSON208_H

#include "CommonCode.h"

void PrintControls();
NxPrismaticJoint* CreatePrismaticJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON208_H
