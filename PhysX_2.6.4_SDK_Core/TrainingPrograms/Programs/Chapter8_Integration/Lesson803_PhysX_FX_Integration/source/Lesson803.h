// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			              LESSON 803: PHYSX FX INTEGRATION
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON803_H
#define LESSON803_H

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
void InitGlut(int argc, char** argv, char* lessonTitle);

NxActor* CreateGroundPlane(int sceneIndex);
NxActor* CreateBox(int sceneIndex, const NxVec3& pos, const NxVec3& boxDim, const NxReal density);
NxActor* CreateSphere(int sceneIndex, const NxVec3& pos, const NxReal radius, const NxReal density);
NxActor* CreateCapsule(int sceneIndex, const NxVec3& pos, const NxReal height, const NxReal radius, const NxReal density);
NxActor* CreatePyramid(int sceneIndex, const NxVec3& pos, const NxVec3& boxDim, const NxReal density);

NxActor** CreateStack(int sceneIndex, const NxVec3& pos, const NxVec3& stackDim, const NxVec3& boxDim, NxReal density);

// PhysX FX actor globals
class PFXActor
{
public:
	PFXActor() {}
	~PFXActor() {}

	NxActor* actor[2];
};

void UpdatePFXActors();
void AddPFXActor(PFXActor* nfxActor, NxActor* actor);
void CreatePFXActors();

void InitializeHUD();

void InitNx();
void ReleaseNx();
void ResetNx();

NxReal UpdateTime();
void StartPhysics(NxU32 sceneNum, NxReal deltaTime);
void GetPhysicsResults(NxU32 sceneNum);

int main(int argc, char** argv);

#endif  // LESSON803_H



