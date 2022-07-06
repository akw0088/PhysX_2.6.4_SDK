// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						     LESSON 304: TRIGGER REPORT
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON304_H
#define LESSON304_H

#include "CommonCode.h"
#include "TriggerReport.h"
#include "UserData.h"

void PrintControls();
void ApplyForcefieldToActors(const NxVec3 force);
NxActor* CreateTriggerBox(const NxVec3& pos, const NxVec3& boxDim);
int main(int argc, char** argv);

#endif  // LESSON304_H
