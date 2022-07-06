// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					       LESSON 411: FRAGMENTING OBJECTS
//
//						  Written by Sam Thompson, 3-15-05
//						    Edited by Bob Schade, 5-1-06
// ===============================================================================

#ifndef LESSON411_H
#define LESSON411_H

void PrintControls();
void ReleaseRandomShape();
NxActor* CreateBoxPiece(NxMat34& mGlobalPose);
NxActor* CreateMainObject();
int main(int argc, char** argv);

#endif  // LESSON411_H
