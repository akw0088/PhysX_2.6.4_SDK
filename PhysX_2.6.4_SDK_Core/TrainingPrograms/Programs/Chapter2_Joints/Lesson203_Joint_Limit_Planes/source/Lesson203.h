// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					       LESSON 203: JOINT LIMIT PLANES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON203_H
#define LESSON203_H

#include "CommonCode.h"

void PrintControls();
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON203_H

