// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					        LESSON 402: SPRING-MASS CLOTH
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON402_H
#define LESSON402_H

#include "CommonCode2.h"

void PrintControls();
void RenderCloth();
NxSpringAndDamperEffector* CreateSpring();
NxActor* CreateClothMass(const NxVec3& pos, const NxReal radius, const NxReal density);
void CreateCloth(NxVec3& pos, int width, int height, NxReal patchHeight, NxReal patchWidth);
int main(int argc, char** argv);

#endif  // LESSON402_H
