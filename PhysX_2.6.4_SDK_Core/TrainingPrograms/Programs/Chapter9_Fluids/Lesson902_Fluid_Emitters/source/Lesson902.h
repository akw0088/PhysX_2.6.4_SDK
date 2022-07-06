// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                 LESSON 902: FLUID EMITTERS
// 
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON902_H
#define LESSON902_H

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "Actors.h"
#include "DrawObjects.h"
#include "UserData.h"
#include "HUD.h"

#include "DebugRenderer.h"
#include "UserAllocator.h"

#include "Stream.h"
#include "NxCooking.h"

void PrintControls();
bool IsSelectable(NxActor* actor);
void SelectNextActor();

void ProcessCameraKeys();
void SetupCamera();

void RenderActors(bool shadows);
void RenderFluid();
void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color);

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode);
void ProcessForceKeys();
void ProcessInputs();

void RenderCallback();
void ReshapeCallback(int width, int height);
void IdleCallback();
void KeyboardCallback(unsigned char key, int x, int y);
void KeyboardUpCallback(unsigned char key, int x, int y);
void SpecialCallback(int key, int x, int y);
void MouseCallback(int button, int state, int x, int y);
void MotionCallback(int x, int y);
void ExitCallback();
void InitGlut(int argc, char** argv, char* lessonTitle);

void CreateStaticFluidMesh( float restParticlesPerMeter, float kernelRadiusMultiplier, float motionLimitMultiplier, unsigned packetSizeMultiplier, NxScene* scene);
NxFluid* CreateFluid(const NxVec3& pos, const NxU32 sideNum, const NxReal distance);
void ReleaseFluid();

void InitializeHUD();

void InitNx();
void ReleaseNx();
void ResetNx();

void StartPhysics();
void GetPhysicsResults();

int main(int argc, char** argv);

#endif  // LESSON902_H



