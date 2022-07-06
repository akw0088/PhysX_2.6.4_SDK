// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					   LESSON 214: SPRING AND DAMPER EFFECTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON214_H
#define LESSON214_H

#include "CommonCode2.h"

void PrintControls();
NxSpringAndDamperEffector* CreateSpring(NxActor* actor1, const NxVec3& pos1, NxActor* actor2, const NxVec3& pos2);
int main(int argc, char** argv);

#endif  // LESSON214_H
