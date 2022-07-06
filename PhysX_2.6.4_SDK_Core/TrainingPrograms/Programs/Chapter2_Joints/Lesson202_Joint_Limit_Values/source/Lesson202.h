// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					       LESSON 202: JOINT LIMIT VALUES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON202_H
#define LESSON202_H

#include "CommonCode.h"

void PrintControls();
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON202_H

