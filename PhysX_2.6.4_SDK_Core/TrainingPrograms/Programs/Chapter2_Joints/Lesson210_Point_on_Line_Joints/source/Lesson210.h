// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						  LESSON 210: POINT-ON-LINE JOINTS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON210_H
#define LESSON210_H

#include "CommonCode.h"

void PrintControls();
NxActor* CreateRegularPyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateDownWedge(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxPointOnLineJoint* CreatePointOnLineJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis);
int main(int argc, char** argv);

#endif  // LESSON210_H
