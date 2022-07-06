// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    LESSON 206: BREAKABLE JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON206_H
#define LESSON206_H

#include "CommonCode.h"

void PrintControls();
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON206_H
