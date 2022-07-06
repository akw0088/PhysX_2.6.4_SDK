// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						  LESSON 605: CHARACTER CONTROLLER
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON605_H
#define LESSON605_H

#include <GL/glut.h>
#include <stdio.h>

#include "NxCooking.h"

#include "NxPhysics.h"
#include "DrawObjects.h"
#include "Actors.h"
#include "UserData.h"
#include "HUD.h"

#include "DebugRenderer.h"
#include "UserAllocator.h"
#include "ErrorStream.h"

#include "NxController.h"
#include "CharacterControl.h"

//#include "MouseFilter.h"
#include "Terrain.h"
#include "TerrainRender.h"

#include "Stream.h"

void StartJump(NxF32 v0);
void StopJump();
NxF32 GetHeight(NxF32 elapsedTime);

void PrintControls();
NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode);
bool IsSelectable(NxActor* actor);
void SelectNextActor();
void ProcessKeys();

void SetupCamera();
void RenderActors(bool shadows);
void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color);

void RenderCallback();
void ReshapeCallback(int width, int height);
void IdleCallback();
void KeyboardCallback(unsigned char key, int x, int y);
void KeyboardUpCallback(unsigned char key, int x, int y);
void SpecialCallback(int key, int x, int y);
void MouseCallback(int button, int state, int x, int y);
void MotionCallback(int x, int y);
void ExitCallback();
void InitGlut(int argc, char** argv);

void CreateRandomTerrain();

void InitializeHUD();

void InitNx();
void ReleaseNx();
void ResetNx();

NxReal UpdateTime();
void UpdateCharacter(NxReal deltaTime);
void PostUpdateCharacter();
void RunPhysics();

int main(int argc, char** argv);

#endif  // LESSON605_H


