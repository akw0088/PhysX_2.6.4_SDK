// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                  LESSON 504: SCENE EXPORT
//
//   A serialization sample for the PhysX Physics SDK showing explicit export 
//   and import of a scene by Stephen Hatcher, based on work by Pierre Terdiman 
//   (01.01.04) and others
//
//						    Edited by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON504_H
#define LESSON504_H

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
void ExitCallback();
void InitGlut(int argc, char** argv);

NxActor* CreateGroundPlane();

void CreateCube(const NxVec3& pos, int size, const NxVec3* initial_velocity=NULL);
void CreateStack(int size);
void CreateTower(int size);

void InitBlankScene();

void InitializeHUD();
void InitializeSpecialHUD();

void InitNx();
void ReleaseNx();
void ResetNx();

void StartPhysics(NxU32 i);
void GetPhysicsResults(NxU32 i);

int main(int argc, char** argv);

#endif  // LESSON504_H



