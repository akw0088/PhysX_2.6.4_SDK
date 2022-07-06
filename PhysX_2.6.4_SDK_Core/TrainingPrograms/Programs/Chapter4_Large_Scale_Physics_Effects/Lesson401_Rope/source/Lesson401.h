// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					              LESSON 401: ROPE
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON401_H
#define LESSON401_H

#include "CommonCode2.h"

void PrintControls();
void CreateRope(const NxVec3& pos, const NxReal segHeight, const NxReal segRadius, const int nbSegments, const NxReal density);
int main(int argc, char** argv);

#endif  // LESSON401_H
