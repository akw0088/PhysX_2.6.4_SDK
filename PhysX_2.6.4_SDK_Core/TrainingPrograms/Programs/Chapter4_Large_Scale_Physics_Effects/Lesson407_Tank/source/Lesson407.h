// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					              LESSON 407: TANK
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON407_H
#define LESSON407_H

#include "CommonCode2.h"

void PrintControls();
void NewLight();
void SetMotorOnTread(int i, const NxMotorDesc& mDesc);
void ProcessMotorKeys();
void InitMotors();
int main(int argc, char** argv);

#endif  // LESSON407_H
