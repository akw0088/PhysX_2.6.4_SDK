// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							   OBJECT DRAWING ROUTINES
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef DRAWOBJECTS_H
#define DRAWOBJECTS_H

class NxShape;
class NxActor;

void SetupGLMatrix(const NxVec3& pos, const NxMat33& orient);
void DrawLine(const NxVec3& p0, const NxVec3& p1, const NxVec3& color, float lineWidth=2.0f);
void DrawTriangle(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2, const NxVec3& color);
void DrawCircle(NxU32 nbSegments, const NxMat34& matrix, const NxVec3& color, const NxF32 radius, const bool semicircle = false);
void DrawEllipse(NxU32 nbSegments, const NxMat34& matrix, const NxVec3& color, const NxF32 radius1, const NxF32 radius2, const bool semicircle = false);

void DrawWirePlane(NxShape* plane, const NxVec3& color);
void DrawPlane(NxShape* plane);

void DrawWireBox(NxShape* box, const NxVec3& color, float lineWidth=2.0f);
void DrawWireBox(const NxBox& obb, const NxVec3& color, float lineWidth=2.0f);
void DrawBox(NxShape* box);

void DrawWireSphere(NxShape* sphere, const NxVec3& color);
void DrawWireSphere(NxSphere* sphere, const NxVec3& color);
void DrawSphere(NxShape* sphere);

void DrawWireCapsule(NxShape* capsule, const NxVec3& color);
void DrawWireCapsule(const NxCapsule& capsule, const NxVec3& color);
void DrawCapsule(NxShape* capsule);
void DrawCapsule(const NxVec3& color, NxF32 r, NxF32 h);

void DrawWireConvex(NxShape* mesh, const NxVec3& color, bool useShapeUserData);
void DrawConvex(NxShape* mesh, bool useShapeUserData);

void DrawWireMesh(NxShape* mesh, const NxVec3& color, bool useShapeUserData);
void DrawMesh(NxShape* mesh, bool useShapeUserData);
void DrawWheelShape(NxShape* wheel);

void DrawArrow(const NxVec3& posA, const NxVec3& posB, const NxVec3& color);
void DrawContactPoint(const NxVec3& pos, const NxReal radius, const NxVec3& color);

void DrawWireShape(NxShape* shape, const NxVec3& color, bool useShapeUserData);
void DrawShape(NxShape* shape, bool useShapeUserData);
void DrawActor(NxActor* actor, NxActor* selectedActor, bool useShapeUserData);
void DrawActorShadow(NxActor* actor, bool useShapeUserData);
void DrawActorShadow2(NxActor* actor, bool useShapeUserData);
void DrawActorShadowZUp(NxActor* actor, bool useShapeUserData);

void DrawCloth(NxCloth *cloth,bool shadows);

void DrawActorShapeSelect(NxActor* actor, NxShape* shape, const bool shapeSelectMode, NxActor* selectedActor, bool useShapeUserData);


#endif  // DRAWOBJECTS_H
