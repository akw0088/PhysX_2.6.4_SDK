// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    LESSON 109: COLLISION GROUPS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON109_H
#define LESSON109_H

#include "CommonCode.h"
#include "ContactReport.h"

void PrintControls();
void CheckContacts();
void SetActorCollisionGroup(NxActor *actor, NxCollisionGroup group);
void InitContactReports();
void InitContactReportsPerPair();
void InitContactReportsPerGroup();
int main(int argc, char** argv);

#endif  // LESSON109_H


