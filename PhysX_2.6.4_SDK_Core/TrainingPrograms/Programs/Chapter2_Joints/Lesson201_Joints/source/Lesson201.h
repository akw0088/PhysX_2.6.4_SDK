// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						         LESSON 201: JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON201_H
#define LESSON201_H

#include "CommonCode.h"

void PrintControls();
NxFixedJoint* CreateFixedJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON201_H


