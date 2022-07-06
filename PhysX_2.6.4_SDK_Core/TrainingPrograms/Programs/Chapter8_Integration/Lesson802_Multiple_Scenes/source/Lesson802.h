// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                 LESSON 802: MULTIPLE SCENES
//
//						   Written by Bob Schade, 12-15-05
// ===============================================================================

#ifndef LESSON802_H
#define LESSON802_H

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"
#include "DrawObjects.h"
#include "UserData.h"
#include "HUD.h"

#include "Stream.h"
#include "NxCooking.h"

#include "DebugRenderer.h"
#include "UserAllocator.h"
#include "ErrorStream.h"


void PrintControls();
bool IsSelectable(NxActor* actor);
void SelectNextActor();
void ProcessCameraKeys();
void SetupCamera();
void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color);

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode);
void ProcessForceKeys();
void RenderActors(bool shadows);
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

NxActor* CreateGroundPlane(int sceneIndex);
NxActor* CreateBox(int sceneIndex, const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateSphere(int sceneIndex, const NxVec3& pos, const NxReal radius, const NxReal density);
NxActor* CreateCapsule(int sceneIndex, const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density);
NxActor* CreatePyramid(int sceneIndex, const NxVec3& pos, const NxVec3& boxDim, const NxReal density, NxConvexMeshDesc& cmd);

void InitNx();
void ReleaseNx();
void ResetNx();

NxReal UpdateTime();
void StartPhysics();
void GetPhysicsResults();

int main(int argc, char** argv);

#endif  // LESSON402_H



