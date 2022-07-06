// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			               LESSON 306: INTERSECTION TESTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON306_H
#define LESSON306_H

#include "CommonCode.h"
#include "UserData.h"
#include "EntityReport.h"

void PrintControls();
void RenderIntersectedEntities();
NxActor* CreateFlatHeightfield(const NxVec3& pos, const NxU32 length, const NxU32 width, const NxReal stride, NxTriangleMeshDesc& tmd);
int main(int argc, char** argv);

#endif  // LESSON306_H



