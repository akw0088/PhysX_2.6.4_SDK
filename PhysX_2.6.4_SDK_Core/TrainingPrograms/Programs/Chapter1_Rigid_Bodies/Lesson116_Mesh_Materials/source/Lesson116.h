// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						 	 LESSON 116: MESH MATERIALS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON116_H
#define LESSON116_H

#include "CommonCode.h"
#include "RaycastReport.h"

void PrintControls();
void RenderTerrain();
void SmoothTriangle(NxU32 a, NxU32 b, NxU32 c);
void ChooseTrigMaterial(NxU32 faceIndex);
NxActor* CreateTerrain(NxTriangleMeshDesc& tmd);
void AddTerrainLight();
int main(int argc, char** argv);

#endif  // LESSON116_H



