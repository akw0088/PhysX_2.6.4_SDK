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
void DrawSphere(NxShape* sphere);

void DrawWireCapsule(NxShape* capsule, const NxVec3& color);
void DrawWireCapsule(const NxCapsule& capsule, const NxVec3& color);
void DrawCapsule(NxShape* capsule);
void DrawCapsule(const NxVec3& color, NxF32 r, NxF32 h);

void DrawWireConvex(NxShape* mesh, const NxVec3& color);
void DrawConvex(NxShape* mesh);

void DrawWireMesh(NxShape* mesh, const NxVec3& color);
void DrawMesh(NxShape* mesh);
void DrawWheelShape(NxShape* wheel);

void DrawArrow(const NxVec3& posA, const NxVec3& posB, const NxVec3& color);
void DrawContactPoint(const NxVec3& pos, const NxReal radius, const NxVec3& color);

void DrawWireShape(NxShape* shape, const NxVec3& color);
void DrawShape(NxShape* shape);
void DrawActor(NxActor* actor, NxActor* gSelectedActor=NULL);
void DrawWireActor(NxActor* actor);
void DrawActorShadow(NxActor* actor);
void DrawActorShadow2(NxActor* actor);
void DrawActorShadowZUp(NxActor* actor);

void DrawCloth(NxCloth *cloth,bool shadows);

#endif  // DRAWOBJECTS_H
