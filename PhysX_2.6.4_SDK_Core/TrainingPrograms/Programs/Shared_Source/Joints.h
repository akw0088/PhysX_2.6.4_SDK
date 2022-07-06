// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						 	   JOINT CREATION METHODS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef JOINTS_H
#define JOINTS_H

#include "NxPhysics.h"

NxFixedJoint* CreateFixedJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxSphericalJoint* CreateSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxPrismaticJoint* CreatePrismaticJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxCylindricalJoint* CreateCylindricalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxPointOnLineJoint* CreatePointOnLineJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxPointInPlaneJoint* CreatePointInPlaneJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxPulleyJoint* CreatePulleyJoint(NxActor* a0, NxActor* a1, const NxVec3& pulley0, const NxVec3& pulley1, const NxVec3& globalAxis, NxReal distance, NxReal ratio, const NxMotorDesc& motorDesc);
NxDistanceJoint* CreateDistanceJoint(NxActor* a0, NxActor* a1, const NxVec3& anchor0, const NxVec3& anchor1, const NxVec3& globalAxis);

NxSphericalJoint* CreateRopeSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxSphericalJoint* CreateClothSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxSphericalJoint* CreateBodySphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxRevoluteJoint* CreateWheelJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxRevoluteJoint* CreateStepJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);

NxRevoluteJoint* CreateChassisJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);
NxFixedJoint* CreateCannonJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);

NxSphericalJoint* CreateBladeLink(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis);

NxRevoluteJoint* CreateRevoluteJoint2(NxActor* a0, NxActor* a1,const NxVec3& localAnchor0,const NxVec3& localAnchor1,const NxVec3& localAxis0,const NxVec3& localAxis1);

#endif  // JOINTS_H

