// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							  LESSON 205: JOINT MOTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON205_H
#define LESSON205_H

#include "CommonCode.h"

void PrintControls();
void ShiftMotorVelocity();
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON205_H

