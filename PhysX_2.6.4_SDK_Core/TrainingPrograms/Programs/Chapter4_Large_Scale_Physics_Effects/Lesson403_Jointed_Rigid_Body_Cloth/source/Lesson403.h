// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					    LESSON 403: JOINTED RIGID BODY CLOTH
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON403_H
#define LESSON403_H

#include "CommonCode2.h"

void PrintControls();
NxActor* CreatePatch(const NxVec3& pos, const NxVec3& patchDim, const NxReal density);
void CreateMattress(const NxVec3& pos, const int width, const int height, const NxVec3& patchDim);
int main(int argc, char** argv);

#endif  // LESSON403_H
