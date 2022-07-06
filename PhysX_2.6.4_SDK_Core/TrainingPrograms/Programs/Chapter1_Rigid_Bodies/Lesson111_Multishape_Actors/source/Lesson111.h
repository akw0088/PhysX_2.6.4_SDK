// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						    LESSON 111: MULTISHAPE ACTORS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON111_H
#define LESSON111_H

#include "CommonCode.h"

void PrintControls();
NxActor* CreateWoodenTable(const NxVec3& pos, const NxVec3& tableDim, const NxReal topRatio, const NxReal legRatio, const NxReal density);
int main(int argc, char** argv);

#endif  // LESSON111_H
