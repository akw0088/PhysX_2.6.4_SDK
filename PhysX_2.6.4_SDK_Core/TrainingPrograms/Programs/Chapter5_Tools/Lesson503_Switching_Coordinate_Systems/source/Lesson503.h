// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			           LESSON 503: SWITCHING COORDINATE SYSTEMS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON503_H
#define LESSON503_H

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "Actors.h"
#include "DrawObjects.h"
#include "UserData.h"
#include "HUD.h"

#include "DebugRenderer.h"
#include "UserAllocator.h"
#include "ErrorStream.h"

#include "Stream.h"
#include "NxCooking.h"

#include "array.h"
#include "stringclass.h"
#include "nxml.h"

void PrintControls();
bool IsSelectable(NxActor* actor);
void SelectNextActor();

void ProcessCameraKeys();
void SetupCamera();

void RenderActors(bool shadows);
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
void InitGlut(int argc, char** argv, char* lessonTitle);

void InitializeHUD();

void InitNx();
void ReleaseNx();
void ResetNx();

NxReal UpdateTime();
void StartPhysics();
void GetPhysicsResults();

int main(int argc, char** argv);

#endif  // LESSON503_H



