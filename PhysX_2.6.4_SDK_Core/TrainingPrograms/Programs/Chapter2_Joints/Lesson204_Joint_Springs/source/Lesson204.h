// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						      LESSON 204: JOINT SPRINGS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON204_H
#define LESSON204_H

#include "CommonCode.h"

void PrintControls();
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON204_H

