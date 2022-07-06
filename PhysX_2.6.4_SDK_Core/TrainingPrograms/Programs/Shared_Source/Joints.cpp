// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//						 	   JOINT CREATION METHODS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include "Joints.h"

extern NxScene* gScene;

NxFixedJoint* CreateFixedJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxFixedJointDesc fixedDesc;
	fixedDesc.actor[0] = a0;
	fixedDesc.actor[1] = a1;
	fixedDesc.setGlobalAnchor(globalAnchor);
	fixedDesc.setGlobalAxis(globalAxis);

	return (NxFixedJoint*)gScene->createJoint(fixedDesc);
}

NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	revDesc.setGlobalAnchor(globalAnchor);
	revDesc.setGlobalAxis(globalAxis);

	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
}

NxRevoluteJoint* CreateRevoluteJoint2(NxActor* a0, NxActor* a1, const NxVec3& localAnchor0,const NxVec3& localAnchor1, const NxVec3& localAxis0, const NxVec3& localAxis1)
{
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	//revDesc.setGlobalAnchor(globalAnchor);
	//revDesc.setGlobalAxis(globalAxis);
	revDesc.localAnchor[0]=localAnchor0;
	revDesc.localAnchor[1]=localAnchor1;
	revDesc.localAxis[0]  =localAxis0;
	revDesc.localAxis[1]  =localAxis1;
	revDesc.projectionMode = NX_JPM_POINT_MINDIST;
	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
}


NxSphericalJoint* CreateSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxSphericalJointDesc sphericalDesc;
	sphericalDesc.actor[0] = a0;
	sphericalDesc.actor[1] = a1;
	sphericalDesc.setGlobalAnchor(globalAnchor);
	sphericalDesc.setGlobalAxis(globalAxis);

	return (NxSphericalJoint*)gScene->createJoint(sphericalDesc);
}

NxPrismaticJoint* CreatePrismaticJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxPrismaticJointDesc prismaticDesc;
	prismaticDesc.actor[0] = a0;
	prismaticDesc.actor[1] = a1;
	prismaticDesc.setGlobalAnchor(globalAnchor);
	prismaticDesc.setGlobalAxis(globalAxis);

	NxJoint* joint = gScene->createJoint(prismaticDesc);
	
//	joint->setLimitPoint(globalAnchor);
//	joint->addLimitPlane(-globalAxis, globalAnchor + 1.5*globalAxis);
//	joint->addLimitPlane(globalAxis, globalAnchor - 1.5*globalAxis);

	return (NxPrismaticJoint*)joint;
}

NxCylindricalJoint* CreateCylindricalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxCylindricalJointDesc cylDesc;
	cylDesc.actor[0] = a0;
	cylDesc.actor[1] = a1;
	cylDesc.setGlobalAnchor(globalAnchor);
	cylDesc.setGlobalAxis(globalAxis);

	NxJoint* joint = gScene->createJoint(cylDesc);

	return (NxCylindricalJoint*)joint;
}

NxPointOnLineJoint* CreatePointOnLineJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxPointOnLineJointDesc polDesc;
	polDesc.actor[0] = a0;
	polDesc.actor[1] = a1;
	polDesc.setGlobalAnchor(globalAnchor);
	polDesc.setGlobalAxis(globalAxis);
	polDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	NxJoint* joint = gScene->createJoint(polDesc);

	return (NxPointOnLineJoint*)joint;
}

NxPointInPlaneJoint* CreatePointInPlaneJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxPointInPlaneJointDesc pipDesc;
	pipDesc.actor[0] = a0;
	pipDesc.actor[1] = a1;
	pipDesc.setGlobalAnchor(globalAnchor);
	pipDesc.setGlobalAxis(globalAxis);
	pipDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	NxJoint* joint = gScene->createJoint(pipDesc);

	return (NxPointInPlaneJoint*)joint;
}

NxPulleyJoint* CreatePulleyJoint(NxActor* a0, NxActor* a1, const NxVec3& pulley0, const NxVec3& pulley1, const NxVec3& globalAxis, NxReal distance, NxReal ratio, const NxMotorDesc& motorDesc)
{
	NxPulleyJointDesc pulleyDesc;
	pulleyDesc.actor[0] = a0;
	pulleyDesc.actor[1] = a1;
	pulleyDesc.localAnchor[0] = NxVec3(0,2,0);
	pulleyDesc.localAnchor[1] = NxVec3(0,2,0);
	pulleyDesc.setGlobalAxis(globalAxis);

	pulleyDesc.pulley[0] = pulley0; 	// suspension points of two bodies in world space.
	pulleyDesc.pulley[1] = pulley1; 	// suspension points of two bodies in world space.
	pulleyDesc.distance = distance;		// the rest length of the rope connecting the two objects.  The distance is computed as ||(pulley0 - anchor0)|| +  ||(pulley1 - anchor1)|| * ratio.
	pulleyDesc.ratio = ratio;			// transmission ratio
	pulleyDesc.flags = NX_PJF_IS_RIGID;	// this is a combination of the bits defined by ::NxPulleyJointFlag. 
	pulleyDesc.motor = motorDesc;
	pulleyDesc.stiffness = 1;		    // how stiff the constraint is, between 0 and 1 (stiffest)

//	pulleyDesc.projectionMode = NX_JPM_NONE;
//	pulleyDesc.projectionMode = NX_JPM_POINT_MINDIST;

	pulleyDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	return (NxPulleyJoint*)gScene->createJoint(pulleyDesc);
}

NxDistanceJoint* CreateDistanceJoint(NxActor* a0, NxActor* a1, const NxVec3& anchor0, const NxVec3& anchor1, const NxVec3& globalAxis)
{
   NxDistanceJointDesc distanceDesc;
   distanceDesc.actor[0] = a0;
   distanceDesc.actor[1] = a1;
   distanceDesc.localAnchor[0] = anchor0;
   distanceDesc.localAnchor[1] = anchor1;
   distanceDesc.setGlobalAxis(globalAxis);

   NxVec3 dist = a1->getGlobalPose()*anchor1 - a0->getGlobalPose()*anchor0;
   distanceDesc.maxDistance = dist.magnitude()*1.5f;  // maximum rest length of the rope or rod between the two anchor points
   distanceDesc.minDistance = dist.magnitude()*0.5f;  // minimum rest length of the rope or rod between the two anchor points
   NxSpringDesc spring;
   spring.spring = 100;
   spring.damper = 0.5;
   distanceDesc.spring = spring;  // makes the joint springy. The spring.targetValue field is not used.
   distanceDesc.flags = (NX_DJF_MIN_DISTANCE_ENABLED | NX_DJF_MAX_DISTANCE_ENABLED);  // combination of the bits defined by ::NxDistanceJointFlag
   distanceDesc.flags |= NX_DJF_SPRING_ENABLED;

//    distanceDesc.projectionMode = NX_JPM_NONE;
//    distanceDesc.projectionMode = NX_JPM_POINT_MINDIST;

   distanceDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

   return (NxDistanceJoint*)gScene->createJoint(distanceDesc);
}

NxSphericalJoint* CreateRopeSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxSphericalJointDesc sphericalDesc;
	sphericalDesc.actor[0] = a0;
	sphericalDesc.actor[1] = a1;
	sphericalDesc.setGlobalAnchor(globalAnchor);
	sphericalDesc.setGlobalAxis(globalAxis);

	sphericalDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;
	sphericalDesc.twistLimit.low.value = -(NxReal)0.1*NxPi;
	sphericalDesc.twistLimit.high.value = (NxReal)0.1*NxPi;

	sphericalDesc.flags |= NX_SJF_TWIST_SPRING_ENABLED;
	NxSpringDesc ts;
	ts.spring = 500;
	ts.damper = 0.5;
	ts.targetValue = 0;
	sphericalDesc.twistSpring = ts;

	sphericalDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
	sphericalDesc.swingLimit.value = (NxReal)0.3*NxPi;

	sphericalDesc.flags |= NX_SJF_SWING_SPRING_ENABLED;
	NxSpringDesc ss;
	ss.spring = 500;
	ss.damper = 0.5;
	ss.targetValue = 0;
	sphericalDesc.swingSpring = ss;

	return (NxSphericalJoint*)gScene->createJoint(sphericalDesc);
}

NxSphericalJoint* CreateClothSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxSphericalJointDesc sphericalDesc;
	sphericalDesc.actor[0] = a0;
	sphericalDesc.actor[1] = a1;
	sphericalDesc.setGlobalAnchor(globalAnchor);
	sphericalDesc.setGlobalAxis(globalAxis);

	sphericalDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;
	sphericalDesc.twistLimit.low.value = -(NxReal)0.025*NxPi;
	sphericalDesc.twistLimit.low.hardness = 0.5;
	sphericalDesc.twistLimit.low.restitution = 0.5;
	sphericalDesc.twistLimit.high.value = (NxReal)0.025*NxPi;
	sphericalDesc.twistLimit.high.hardness = 0.5;
	sphericalDesc.twistLimit.high.restitution = 0.5;

	sphericalDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
	sphericalDesc.swingLimit.value = (NxReal)0.25*NxPi;
	sphericalDesc.swingLimit.hardness = 0.5;
	sphericalDesc.swingLimit.restitution = 0.5;

	sphericalDesc.flags |= NX_SJF_TWIST_SPRING_ENABLED;
	sphericalDesc.twistSpring.spring = 0.5;
	sphericalDesc.twistSpring.damper = 1;

	sphericalDesc.flags |= NX_SJF_SWING_SPRING_ENABLED;
	sphericalDesc.swingSpring.spring = 0.5;
	sphericalDesc.swingSpring.damper = 1;

//	sphericalDesc.flags |= NX_SJF_JOINT_SPRING_ENABLED;
//	sphericalDesc.jointSpring.spring = 0.5;
//	sphericalDesc.jointSpring.damper = 1;

	sphericalDesc.projectionDistance = (NxReal)0.15;
	sphericalDesc.projectionMode = NX_JPM_POINT_MINDIST;

	return (NxSphericalJoint*)gScene->createJoint(sphericalDesc);
}

NxSphericalJoint* CreateBodySphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxSphericalJointDesc sphericalDesc;
	sphericalDesc.actor[0] = a0;
	sphericalDesc.actor[1] = a1;
	sphericalDesc.setGlobalAnchor(globalAnchor);
	sphericalDesc.setGlobalAxis(globalAxis);

	sphericalDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;
	sphericalDesc.twistLimit.low.value = -(NxReal)0.025*NxPi;
	sphericalDesc.twistLimit.low.hardness = 0.5;
	sphericalDesc.twistLimit.low.restitution = 0.5;
	sphericalDesc.twistLimit.high.value = (NxReal)0.025*NxPi;
	sphericalDesc.twistLimit.high.hardness = 0.5;
	sphericalDesc.twistLimit.high.restitution = 0.5;

	sphericalDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
	sphericalDesc.swingLimit.value = (NxReal)0.25*NxPi;
	sphericalDesc.swingLimit.hardness = 0.5;
	sphericalDesc.swingLimit.restitution = 0.5;

	sphericalDesc.flags |= NX_SJF_TWIST_SPRING_ENABLED;
	sphericalDesc.twistSpring.spring = 0.5;
	sphericalDesc.twistSpring.damper = 1;

	sphericalDesc.flags |= NX_SJF_SWING_SPRING_ENABLED;
	sphericalDesc.swingSpring.spring = 0.5;
	sphericalDesc.swingSpring.damper = 1;

//	sphericalDesc.flags |= NX_SJF_JOINT_SPRING_ENABLED;
//	sphericalDesc.jointSpring.spring = 0.5;
//	sphericalDesc.jointSpring.damper = 1;

	sphericalDesc.projectionDistance = (NxReal)0.15;
	sphericalDesc.projectionMode = NX_JPM_POINT_MINDIST;

	return (NxSphericalJoint*)gScene->createJoint(sphericalDesc);
}

NxRevoluteJoint* CreateWheelJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	revDesc.setGlobalAnchor(globalAnchor);
	revDesc.setGlobalAxis(globalAxis);

	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
}

NxRevoluteJoint* CreateStepJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	revDesc.setGlobalAnchor(globalAnchor);
	revDesc.setGlobalAxis(globalAxis);

	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
}

NxRevoluteJoint* CreateChassisJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	revDesc.setGlobalAnchor(globalAnchor);
	revDesc.setGlobalAxis(globalAxis);

	revDesc.flags |= NX_RJF_LIMIT_ENABLED;

	NxJointLimitPairDesc limitDesc;
	limitDesc.high.value = (NxReal)0.01*NxPi;
	limitDesc.low.value = -(NxReal)0.01*NxPi;;

	revDesc.limit = limitDesc;

	revDesc.flags |= NX_RJF_SPRING_ENABLED;
	NxSpringDesc springDesc;
	springDesc.targetValue = 0;
	springDesc.spring = 5000;

//	motorDesc.maxForce = 100;
//	motorDesc.freeSpin = 0;
//
//	revDesc.motor = motorDesc;

	return (NxRevoluteJoint*)gScene->createJoint(revDesc);
}

NxFixedJoint* CreateCannonJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxFixedJointDesc fixDesc;

	fixDesc.actor[0] = a0;
	fixDesc.actor[1] = a1;
	fixDesc.setGlobalAnchor(globalAnchor);
	fixDesc.setGlobalAxis(globalAxis);

	return (NxFixedJoint*)gScene->createJoint(fixDesc);
}

NxSphericalJoint* CreateBladeLink(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis)
{
	NxSphericalJointDesc sphericalDesc;
	sphericalDesc.actor[0] = a0;
	sphericalDesc.actor[1] = a1;
	sphericalDesc.setGlobalAnchor(globalAnchor);
	sphericalDesc.setGlobalAxis(globalAxis);

	sphericalDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;
	sphericalDesc.swingLimit.value = (NxReal)0.05*NxPi;
	sphericalDesc.swingLimit.restitution = 0.75;
	sphericalDesc.swingLimit.hardness = 0.5;

	sphericalDesc.flags |= NX_SJF_SWING_SPRING_ENABLED;
	sphericalDesc.swingSpring.spring = 0.75;
	sphericalDesc.swingSpring.damper = 1;
	
	sphericalDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;
	sphericalDesc.twistLimit.low.value = -(NxReal)0.05*NxPi;
	sphericalDesc.twistLimit.low.restitution = 0.75;
	sphericalDesc.twistLimit.low.hardness = 0.5;
	sphericalDesc.twistLimit.high.value = (NxReal)0.05*NxPi;
	sphericalDesc.twistLimit.high.restitution = 0.75;
	sphericalDesc.twistLimit.high.hardness = 0.5;

	sphericalDesc.flags |= NX_SJF_TWIST_SPRING_ENABLED;
	sphericalDesc.twistSpring.spring = 0.75;
	sphericalDesc.twistSpring.damper = 1;

	return (NxSphericalJoint*)gScene->createJoint(sphericalDesc);
}



