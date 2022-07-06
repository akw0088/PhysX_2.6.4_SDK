/*----------------------------------------------------------------------------*\
|
|						     Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

// A PhysX demo application, using debug visualization and joints
// by Simon Schirm

NxMaterialIndex wheelMaterialIndex = 1;

class Truck
{
public:

	Truck(): force(0), steerAngle(0) {}

	struct Wheel
	{
		void create(NxScene& scene, const NxVec3& pos, float rad, NxActor* holder)
		{
			NxActorDesc actorDesc;
			NxBodyDesc bodyDesc;
			NxSphereShapeDesc sphereDesc;
			
			bodyDesc.solverIterationCount = 20;

			// wheel
			sphereDesc.radius = rad;
			sphereDesc.materialIndex = wheelMaterialIndex;
			actorDesc.shapes.pushBack(&sphereDesc);
			bodyDesc.mass = 400;
			actorDesc.body = &bodyDesc;
			actorDesc.globalPose.t = pos;
			wheel = scene.createActor(actorDesc);

			// roll axis
			bodyDesc.mass = 50;
			bodyDesc.massSpaceInertia = NxVec3(1,1,1);
			actorDesc.body = &bodyDesc;
			actorDesc.shapes.clear();
			actorDesc.globalPose.t  = pos;
			rollAxis = scene.createActor(actorDesc);
			
			// revolute joint connecting wheel with rollAxis
			NxRevoluteJointDesc revJointDesc;
			revJointDesc.projectionMode = NX_JPM_POINT_MINDIST;
			revJointDesc.actor[0] = wheel;
			revJointDesc.actor[1] = rollAxis;
			revJointDesc.setGlobalAnchor(pos);
			revJointDesc.setGlobalAxis(NxVec3(0,0,1));
			rollJoint = (NxRevoluteJoint*)scene.createJoint(revJointDesc);

			// prismatic joint connecting rollAxis with holder
			NxPrismaticJointDesc prisJointDesc;
			prisJointDesc.actor[0] = rollAxis;
			prisJointDesc.actor[1] = holder;
			prisJointDesc.setGlobalAnchor(pos);
			prisJointDesc.setGlobalAxis(NxVec3(0,1,0));
			scene.createJoint(prisJointDesc);

			// add springs and dampers to the suspension (i.e. the related actors)
			float springLength = 0.1f;
			NxSpringAndDamperEffector * springNdamp = scene.createSpringAndDamperEffector(NxSpringAndDamperEffectorDesc());

			springNdamp->setBodies(rollAxis, pos, holder, pos + NxVec3(0,springLength,0));
			springNdamp->setLinearSpring(0, springLength, 2*springLength, 100000, 100000);
			springNdamp->setLinearDamper(-1, 1, 1e5, 1e5);

			// disable collision detection 
			scene.setActorPairFlags(*wheel, *holder, NX_IGNORE_PAIR);
		}

		NxActor* wheel;
		NxActor* rollAxis;
		NxRevoluteJoint* rollJoint;
	};


	struct SteerWheel
	{
		void create(NxScene& scene, const NxVec3& pos, float rad, NxActor* holder)
		{
			NxActorDesc actorDesc;
			NxBodyDesc bodyDesc;

			bodyDesc.solverIterationCount = 20;

			// steer axis
			bodyDesc.mass = 50;
			bodyDesc.massSpaceInertia = NxVec3(1,1,1);
			actorDesc.body = &bodyDesc;
			actorDesc.shapes.clear();

			actorDesc.globalPose.t = pos;
			steerAxis = scene.createActor(actorDesc);
			wheel.create(scene, pos, rad, steerAxis);

			// revolute joint connecting steerAxis with the holder
			NxRevoluteJointDesc revJointDesc;
			revJointDesc.projectionMode = NX_JPM_POINT_MINDIST;
			revJointDesc.actor[0] = steerAxis;
			revJointDesc.actor[1] = holder;
			revJointDesc.setGlobalAnchor(pos);
			revJointDesc.setGlobalAxis(NxVec3(0,1,0));
			steerJoint = (NxRevoluteJoint*)scene.createJoint(revJointDesc);

			// disable collision detection 
			scene.setActorPairFlags(*wheel.wheel, *holder, NX_IGNORE_PAIR);
		}
		
		Wheel wheel;	
		NxActor* steerAxis;
		NxRevoluteJoint* steerJoint;
	};

	void create(NxScene& scene, NxActor& ground)
	{
		NxBodyDesc bodyDesc;	
		NxBoxShapeDesc boxDesc0;
		NxBoxShapeDesc boxDesc1;

		bodyDesc.solverIterationCount = 20;
		
		// tractor actor (two shapes)
		NxActorDesc tractorDesc;
		bodyDesc.mass = 5000;
		tractorDesc.body = &bodyDesc;
		tractorDesc.globalPose.t = NxVec3(0,0,0);

		boxDesc0.dimensions = NxVec3(3.5,3,4);
		boxDesc0.localPose.t = NxVec3(3.5,4,0);
		tractorDesc.shapes.pushBack(&boxDesc0);
		
		boxDesc1.dimensions = NxVec3(8.5,0.7,3);
		boxDesc1.localPose.t = NxVec3(8.7,1.3,0);
		tractorDesc.shapes.pushBack(&boxDesc1);

		tractor = scene.createActor(tractorDesc);

		// trailer actor (also two shapes)
		NxActorDesc trailerDesc;
		bodyDesc.mass = 8000;
		trailerDesc.body = &bodyDesc;
		trailerDesc.globalPose.t = NxVec3(0,0,0);
		
		boxDesc0.dimensions = NxVec3(14.5,4.3,4);
		boxDesc0.localPose.t = NxVec3(24,7.6,0);
		trailerDesc.shapes.pushBack(&boxDesc0);
		
		boxDesc1.dimensions = NxVec3(5.5,0.7,3);
		boxDesc1.localPose.t = NxVec3(31,2,0);
		trailerDesc.shapes.pushBack(&boxDesc1);

		trailer = scene.createActor(trailerDesc);

		// remove collision between lower truck parts and ground
		scene.setShapePairFlags(*tractor->getShapes()[1], *ground.getShapes()[0], NX_IGNORE_PAIR);
		scene.setShapePairFlags(*trailer->getShapes()[1], *ground.getShapes()[0], NX_IGNORE_PAIR);

		// wheels
		float staticFriction  = 1.2f;
		float dynamicFriction = 1.0f;

		steerWheels[0].create(scene, NxVec3(3.5,1.5,4), 1.5, tractor);
		steerWheels[1].create(scene, NxVec3(3.5,1.5,-4), 1.5, tractor);
		frontWheels[0].create(scene, NxVec3(16,1.5,4), 1.5, tractor);
		frontWheels[1].create(scene, NxVec3(16,1.5,-4), 1.5, tractor);

		//additionally remove collision between front wheels and the trailer
		scene.setActorPairFlags(*frontWheels[0].wheel, *trailer, NX_IGNORE_PAIR);
		scene.setActorPairFlags(*frontWheels[1].wheel, *trailer, NX_IGNORE_PAIR);

		backWheels[0].create(scene, NxVec3(30.5,1.5, 4), 1.5, trailer);
		backWheels[1].create(scene, NxVec3(30.5,1.5,-4), 1.5, trailer);
		backWheels[2].create(scene, NxVec3(33.6,1.5, 4), 1.5, trailer);
		backWheels[3].create(scene, NxVec3(33.6,1.5,-4), 1.5, trailer);

		// create rotation joint for the tractor trailer connection
		NxSphericalJointDesc sphJointDesc;
		sphJointDesc.actor[0] = tractor;
		sphJointDesc.actor[1] = trailer;
		sphJointDesc.setGlobalAnchor(NxVec3(10,2.5,0));		
		sphJointDesc.setGlobalAxis(NxVec3(0,1,0));
		sphJointDesc.swingAxis.set(0,0,1);

		sphJointDesc.twistLimit.low.value = NxMath::degToRad(-46.0f);
		sphJointDesc.twistLimit.high.value = NxMath::degToRad(46.0f);
		sphJointDesc.swingLimit.value = NxMath::degToRad(10.0f);

		sphJointDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;
		sphJointDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;

		// NxJointLimitDesc swingLimit;
		scene.createJoint(sphJointDesc);
	}

	void control(bool keyLeft, bool keyRight, bool keyUp, bool keyDown, float dt)
	{
		const NxReal maxSteerAngle = 30.0f;
		NxReal sangle; 

		if (keyLeft ) steerAngle += 1.0f;
		if (keyRight) steerAngle -= 1.0f;
		
		if (steerAngle > 0.0) 
			sangle = NxMath::min(steerAngle,  maxSteerAngle);
		else 
			sangle = NxMath::max(steerAngle, -maxSteerAngle);	
		steerAngle = sangle;

		steerWheels[0].steerJoint->setSpring(NxSpringDesc(5e4, 100, NxMath::degToRad(sangle)));
		steerWheels[1].steerJoint->setSpring(NxSpringDesc(5e4, 100, NxMath::degToRad(sangle)));
		
		float gdvAcc = dt * 30.0f;
		float gdvBrk = dt * 10.0f;

		if (keyUp && !keyDown) 
			force += gdvAcc;
		else if (!keyUp &&  keyDown) 
			force -= gdvBrk;				
		else 
			force = 0.0f;

		unsigned i;
		float maxvel = NxMath::sign(force) * 1e5f;
		for (i=0; i<2; i++)
		{
			frontWheels[i].rollJoint->setMotor(NxMotorDesc(maxvel, 1.0f * NxMath::abs(force), false));
		}
		for (i=0; i<2; i++)
		{
			steerWheels[i].wheel.rollJoint->setMotor(NxMotorDesc(maxvel, 1.0f * NxMath::abs(force), false));
		}
	}

	NxActor* tractor;
	NxActor* trailer;
	SteerWheel steerWheels[2];
	Wheel frontWheels[2];
	Wheel backWheels[4];

	float force;
	float steerAngle;
};

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright © 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
