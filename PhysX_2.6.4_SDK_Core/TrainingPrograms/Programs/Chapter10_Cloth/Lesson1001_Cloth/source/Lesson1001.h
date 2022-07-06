// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                     LESSON 1001: CLOTH
//
//                     Written by Matthias Müller-Fischer, 3-8-05
//						     Edited by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON1001_H
#define LESSON1001_H

#include "NxPhysics.h"
#include "DrawObjects.h"
#include "UserData.h"
#include "HUD.h"

#include "Stream.h"
#include "NxCooking.h"

#include "DebugRenderer.h"
#include "UserAllocator.h"
#include "ErrorStream.h"

#include "Actors.h"
#include "MyCloth.h"

void PrintControls();
bool IsSelectable(NxActor* actor);
void SelectNextActor();
void SelectNextShapeOnActor(NxActor* actor);

void ProcessCameraKeys();
void SetupCamera();

void RenderActors(bool shadows);
void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color);
void DrawForceAtShape(NxActor* actor, NxShape* shape, NxVec3& forceVec, const NxVec3& color);

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode);
void ProcessForceKeys();

NxVec3 ApplyForceToActorAtShape(NxActor* actor, NxShape* shape, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode, bool actorSelectMode);
void ProcessForceAtShapeKeys();

NxVec3 ApplyVelocityToActor(NxActor* actor, const NxVec3& velDir, const NxReal velStrength, bool velMode);
NxVec3 MoveActor(NxActor* actor, const NxVec3& moveDir, const NxReal moveStrength, bool moveMode);
void ProcessMoveKeys();

void ProcessInputs();

void RenderCallback();
void ReshapeCallback(int width, int height);
void IdleCallback();
void KeyboardCallback(unsigned char key, int x, int y);
void KeyboardUpCallback(unsigned char key, int x, int y);
void SpecialKeys(unsigned char key, int x, int y);
void SpecialCallback(int key, int x, int y);
void MouseCallback(int button, int state, int x, int y);
void MotionCallback(int x, int y);
void ExitCallback();
void InitGlut(int argc, char** argv, char* lessonTitle);

NxActor* CreateGroundPlane();
NxActor* CreateBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateSphere(const NxVec3& pos, const NxReal radius, const NxReal density);
NxActor* CreateCapsule(const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density);
NxActor* CreatePyramid(const NxVec3& pos, const NxVec3& boxDim, const NxReal density);

void InitializeHUD();
void InitializeSpecialHUD();

void InitNx();
void ReleaseNx();
void ResetNx();

NxReal UpdateTime();
void StartPhysics();
void GetPhysicsResults();

int main(int argc, char** argv);

#endif  // LESSON1001_H



