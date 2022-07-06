// ===============================================================================
//						    PHYSX SDK TRAINING PROGRAMS
//						 LESSON 805: USER CONTROLLED THREADS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON805_H
#define LESSON805_H

#include <GL/glut.h>
#include <stdio.h>

#include <deque>
#include <vector>

#include "NxPhysics.h"
#include "Actors.h"
#include "DrawObjects.h"
#include "UserData.h"
#include "HUD.h"

#include "ErrorStream.h"
#include "UserAllocator.h"
#include "PerfRenderer.h"

#include "DebugRenderer.h"

#include "CustomScheduler.h"
#include "PollingThreads.h"

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

void InitNx();
void ReleaseNx();
void ResetNx();

void StartPhysics();
void GetPhysicsResults();

void DeleteTriangleMeshDesc(NxTriangleMeshDesc *desc);
NxTriangleMeshDesc *CloneTriangleMeshDesc(const NxTriangleMeshDesc &desc);

int main(int argc, char** argv);

#endif  // LESSON805_H


