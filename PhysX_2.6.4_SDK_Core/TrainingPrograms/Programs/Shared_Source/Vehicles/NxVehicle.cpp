#include <cstdio>

#include <map>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/glut.h>

#include "NxVehicle.h"
#include "NxWheel.h"
#include "NxAllVehicles.h"

#include <NxActorDesc.h>
#include <NxAllocateable.h>
#include <NxBoxShape.h>
#include <NxConvexShape.h>
#include <NxConvexShapeDesc.h>
#include <NxConvexMesh.h>

#define TRAIL_FREQUENCY 0.1f

NxVehicle::NxVehicle() : _steeringWheelState(0), _nxScene(NULL), _carMaterial(NULL) 
	{ 
	memset(_trailBuffer, 0, sizeof(NxVec3) * NUM_TRAIL_POINTS);
	_nextTrailSlot = 0;
	_lastTrailTime = 0.0f;
	}

NxVehicle::~NxVehicle() {
	if (_carMaterial)
		_nxScene->releaseMaterial(*_carMaterial);

	if (_bodyActor)
		_nxScene->releaseActor(*_bodyActor);
	for (NxU32 i = 0; i < _wheels.size(); i++) {
		if (_wheels[i]) {
			delete _wheels[i];
			_wheels[i] = NULL;
		}
	}
}

NxVehicle* NxVehicle::_createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc) {
	//printf("Create Vehicle\n");

	if (vehicleDesc == NULL)
		return NULL;

	NxVehicle* vehicle = new NxVehicle;
	vehicle->userData = vehicleDesc->userData;

	if (!vehicleDesc->isValid()) {
		printf("Vehicle Desc not valid!!\n");
		return NULL;
	}
	
	vehicle->_nxScene = scene;

	if (vehicle->_carMaterial == NULL) {
		NxMaterialDesc carMaterialDesc;
		carMaterialDesc.dynamicFriction = 0.4f;
		carMaterialDesc.staticFriction = 0.4f;
		carMaterialDesc.restitution = 0;
		carMaterialDesc.frictionCombineMode = NX_CM_MULTIPLY;
		vehicle->_carMaterial = scene->createMaterial(carMaterialDesc);
	}
	
	NxActorDesc actorDesc;
	for (NxU32 i = 0; i < vehicleDesc->carShapes.size(); i++) {
		actorDesc.shapes.pushBack(vehicleDesc->carShapes[i]);
		if (actorDesc.shapes[i]->materialIndex == 0)
			actorDesc.shapes[i]->materialIndex = vehicle->_carMaterial->getMaterialIndex();
	}

	NxBodyDesc bodyDesc;
	bodyDesc.mass = vehicleDesc->mass;
	actorDesc.body = &bodyDesc;
	actorDesc.globalPose.t = vehicleDesc->position;

	vehicle->_bodyActor = scene->createActor(actorDesc);
	if (vehicle->_bodyActor == NULL) {
		delete vehicle;
		return NULL;
	}
	vehicle->_bodyActor->userData = vehicle;
	printf("Mass: %2.3f\n", vehicle->_bodyActor->getMass());

	if (vehicleDesc->motorDesc != NULL) {
		vehicle->_vehicleMotor = NxVehicleMotor::createMotor(*vehicleDesc->motorDesc);
		if (vehicle->_vehicleMotor == NULL) {
			delete vehicle;
			return NULL;
		}
		vehicle->_motorForce = 0;
	} else {
		vehicle->_vehicleMotor = NULL;
		vehicle->_motorForce = vehicleDesc->motorForce;
	}

	if (vehicleDesc->gearDesc != NULL) {
		vehicle->_vehicleGears = NxVehicleGears::createVehicleGears(*vehicleDesc->gearDesc);
		if (vehicle->_vehicleGears == NULL) {
			printf("Vehicle gear-creation failed\n");
			delete vehicle;
			return NULL;
		}
	} else {
		vehicle->_vehicleGears = NULL;
	}

	for (NxU32 i = 0; i < vehicleDesc->carWheels.size(); i++) {
		NxWheel* wheel = NxWheel::createWheel(vehicle->_bodyActor, vehicleDesc->carWheels[i]);
		if (wheel) {
			vehicle->_wheels.pushBack(wheel);
		} else {
			delete vehicle;
			return NULL;
		}
	}

	vehicle->_digitalSteeringDelta		= vehicleDesc->digitalSteeringDelta;
	vehicle->_steeringSteerPoint		= vehicleDesc->steeringSteerPoint;
	vehicle->_steeringTurnPoint			= vehicleDesc->steeringTurnPoint;
	vehicle->_steeringMaxAngleRad		= NxMath::degToRad(vehicleDesc->steeringMaxAngle);
	vehicle->_transmissionEfficiency	= vehicleDesc->transmissionEfficiency;
	vehicle->_differentialRatio			= vehicleDesc->differentialRatio;
	vehicle->_maxVelocity				= vehicleDesc->maxVelocity;
	vehicle->_cameraDistance			= vehicleDesc->cameraDistance;
	vehicle->_bodyActor->setCMassOffsetLocalPosition(vehicleDesc->centerOfMass);

	//don't go to sleep.
	vehicle->_bodyActor->wakeUp(1e10);

	vehicle->control(0, true, 0, true, false);
	return vehicle;
}

NxVehicle* NxVehicle::createVehicle(NxScene* scene, NxVehicleDesc* vehicleDesc) {
	if (vehicleDesc == NULL)
		return NULL;
	NxVehicle* vehicle = NxVehicle::_createVehicle(scene, vehicleDesc);
	NxAllVehicles::AddVehicle(vehicle);
	if (NxAllVehicles::getActiveVehicleNumber() != -1 || NxAllVehicles::getNumberOfVehicles() == 1)
		NxAllVehicles::setActiveVehicle(NxAllVehicles::getNumberOfVehicles()-1);

	for (NxU32 i = 0; i < vehicleDesc->children.size(); i++) {
		NxVehicle* child = NxVehicle::_createVehicle(scene, vehicleDesc->children[i]);
		if (child != NULL) {
			vehicle->addChild(child);
			NxAllVehicles::AddChildVehicle(child);
		} else {
			fprintf(stderr, "Warning, child %d could not be created\n", i);
		}
	}

	return vehicle;
}

void NxVehicle::handleContactPair(NxContactPair& pair, NxU32 carIndex) {
	NxContactStreamIterator i(pair.stream);
	
	while(i.goNextPair()) {
		NxShape * s = i.getShape(carIndex);
		
		while(i.goNextPatch()) {
			const NxVec3& contactNormal = i.getPatchNormal();
			
			while(i.goNextPoint()) {
				//user can also call getPoint() and getSeparation() here
	
				const NxVec3& contactPoint = i.getPoint();

				//add forces:

				//assuming front wheel drive we need to apply a force at the wheels.
				if (s->is(NX_SHAPE_CAPSULE) && s->userData != NULL) {
					//assuming only the wheels of the car are capsules, otherwise we need more checks.
					//this branch can't be pulled out of loops because we have to do a full iteration through the stream
				
					NxQuat local2global = s->getActor().getGlobalOrientationQuat();
					NxWheel* w = (NxWheel*)s->userData;
					if (!w->getWheelFlag(NX_WF_USE_WHEELSHAPE))
						{
						NxWheel1 * wheel = static_cast<NxWheel1*>(w);
						wheel->contactInfo.otherActor = pair.actors[1-carIndex];
						wheel->contactInfo.contactPosition = contactPoint;
						
						wheel->contactInfo.contactPositionLocal = contactPoint;
						wheel->contactInfo.contactPositionLocal -= _bodyActor->getGlobalPosition();
						local2global.inverseRotate(wheel->contactInfo.contactPositionLocal);
						
						wheel->contactInfo.contactNormal = contactNormal;
						if (wheel->contactInfo.otherActor->isDynamic()) 
							{
							NxVec3 globalV = s->getActor().getLocalPointVelocity(wheel->getWheelPos());
							globalV -= wheel->contactInfo.otherActor->getLinearVelocity();
							local2global.inverseRotate(globalV);
							wheel->contactInfo.relativeVelocity = globalV.x;
							//printf("%2.3f (%2.3f %2.3f %2.3f)\n", wheel->contactInfo.relativeVelocity,
							//	globalV.x, globalV.y, globalV.z);
							} 
						else 
							{
							NxVec3 vel = s->getActor().getLocalPointVelocity(wheel->getWheelPos());
							local2global.inverseRotate(vel);
							wheel->contactInfo.relativeVelocity = vel.x;
							wheel->contactInfo.relativeVelocitySide = vel.z;
							}
						NX_ASSERT(wheel->hasGroundContact());
						//printf(" Wheel %x is touching\n", wheel);
						}
				}
			}
		}		
	}
	//printf("----\n");
}

void NxVehicle::updateVehicle(NxReal lastTimeStepSize) {
	//printf("updating %x\n", this);
	
	NxReal distanceSteeringAxisCarTurnAxis = _steeringSteerPoint.x  - _steeringTurnPoint.x;
	NX_ASSERT(_steeringSteerPoint.z == _steeringTurnPoint.z);
	NxReal distance2 = 0;
	if (NxMath::abs(_steeringWheelState) > 0.01f)
		distance2 = distanceSteeringAxisCarTurnAxis / NxMath::tan(_steeringWheelState * _steeringMaxAngleRad);

	//printf("d1 = %2.3f, d2 = %2.3f, a1 = %2.3f, a2 = %2.3f\n",
	//	distanceSteeringAxisCarTurnAxis, distance2,
	//	_steeringWheelState, _steeringWheelState * _steeringMaxAngleRad);


	_lastTrailTime += lastTimeStepSize;

	if (_lastTrailTime > TRAIL_FREQUENCY)
		{
		_lastTrailTime = 0.0f;
		}

	NxU32 nbTouching = 0;
	NxU32 nbNotTouching = 0;
	NxU32 nbHandBrake = 0;
	for (NxU32 i = 0; i < _wheels.size(); i++) {
		NxWheel* wheel = _wheels[i];



		if (_lastTrailTime  == 0.0f)
			{
			if (++_nextTrailSlot >= NUM_TRAIL_POINTS)
				_nextTrailSlot = 0;
			_trailBuffer[_nextTrailSlot] = _bodyActor->getGlobalPose() * _wheels[i]->getWheelPos();
			}



		if (wheel->getWheelFlag(NX_WF_STEERABLE_INPUT)) {
			if (distance2 != 0) {
				NxReal xPos = wheel->getWheelPos().x;
				NxReal zPos = wheel->getWheelPos().z;
				NxReal dz = -zPos + distance2;
				NxReal dx = xPos - _steeringTurnPoint.x;
				wheel->setAngle(NxMath::atan(dx/dz));
			} else {
				wheel->setAngle(0.f);
			}
			//printf("%2.3f\n", wheel->getAngle());

		} else if (wheel->getWheelFlag(NX_WF_STEERABLE_AUTO)) {
			NxVec3 localVelocity = _bodyActor->getLocalPointVelocity(wheel->getWheelPos());
			NxQuat local2Global = _bodyActor->getGlobalOrientationQuat();
			local2Global.inverseRotate(localVelocity);
//			printf("%2.3f %2.3f %2.3f\n", wheel->getWheelPos().x,wheel->getWheelPos().y,wheel->getWheelPos().z);
			localVelocity.y = 0;
			if (localVelocity.magnitudeSquared() < 0.01f) {
				wheel->setAngle(0.0f);
			} else {
				localVelocity.normalize();
//				printf("localVelocity: %2.3f %2.3f\n", localVelocity.x, localVelocity.z);
				if (localVelocity.x < 0)
					localVelocity = -localVelocity;
				NxReal angle = NxMath::clamp(atan(localVelocity.z / localVelocity.x), 0.3f, -0.3f);
				wheel->setAngle(angle);
			}
		}

		// now the acceleration part
		if (!wheel->getWheelFlag(NX_WF_ACCELERATED))
			continue;

		if (_handBrake && wheel->getWheelFlag(NX_WF_AFFECTED_BY_HANDBRAKE)) {
			nbHandBrake++;
		} else if (!wheel->hasGroundContact()) {
			nbNotTouching++;
		} else {
			nbTouching++;
		}
	}
	
	NxReal motorTorque = 0.0f; 
	if (nbTouching && NxMath::abs(_accelerationPedal) > 0.01f) 
		{
		NxReal axisTorque = _computeAxisTorque();
		NxReal wheelTorque = axisTorque / (NxReal)(_wheels.size() - nbHandBrake);
		NxReal wheelTorqueNotTouching = nbNotTouching>0?wheelTorque*(NxMath::pow(0.5f, (NxReal)nbNotTouching)):0;
		NxReal wheelTorqueTouching = wheelTorque - wheelTorqueNotTouching;
		motorTorque = wheelTorqueTouching / (NxReal)nbTouching; 
		}
	else 
		{
		_updateRpms();
		}
//printf("wt: %f %f\n", motorTorque, _brakePedal);
	for (NxU32 i = 0; i < _wheels.size(); i++) 
		{
		NxWheel* wheel = _wheels[i];
		wheel->tick(_handBrake, motorTorque, _brakePedal, lastTimeStepSize);
		}


	//printf("---\n");
}

void NxVehicle::_computeMostTouchedActor()	//finds the actor that is touched by most wheels.
	{
	std::map<NxActor*, NxU32> actors;
	typedef std::map<NxActor*, NxU32> Map;
	for (NxU32 i = 0; i < _wheels.size(); i++) {
		NxActor* curActor = _wheels[i]->getTouchedActor();
		Map::iterator it = actors.find(curActor);
		if (it == actors.end()) {
			actors[curActor] = 1;
		} else {
			it->second++;
		}
	}

	NxU32 count = 0;
	_mostTouchedActor = NULL;
	for (Map::iterator it = actors.begin(); it != actors.end(); ++it) {
		if (it->second > count) {
			count = it->second;
			_mostTouchedActor = it->first;
		}
	}
}

void NxVehicle::_controlSteering(NxReal steering, bool analogSteering) {
	if (analogSteering) {
		_steeringWheelState = steering;
	} else if (NxMath::abs(steering) > 0.0001f) {
		_steeringWheelState += NxMath::sign(steering) * _digitalSteeringDelta;
	} else if (NxMath::abs(_steeringWheelState) > 0.0001f) {
		_steeringWheelState -= NxMath::sign(_steeringWheelState) * _digitalSteeringDelta;
	}
	_steeringWheelState = NxMath::clamp(_steeringWheelState, 1.f, -1.f);
	//printf("SteeringWheelState: %2.3f\n", _steeringWheelState);
}

void NxVehicle::_computeLocalVelocity() {
	_computeMostTouchedActor();
	NxVec3 relativeVelocity;
	if (_mostTouchedActor == NULL || !_mostTouchedActor->isDynamic()) {
		relativeVelocity = _bodyActor->getLinearVelocity();
	} else {
		relativeVelocity = _bodyActor->getLinearVelocity() - _mostTouchedActor->getLinearVelocity();
	}
	NxQuat rotation = _bodyActor->getGlobalOrientationQuat();
	NxQuat global2Local;
	_localVelocity = relativeVelocity;
	rotation.inverseRotate(_localVelocity);
	//printf("Velocity: %2.3f %2.3f %2.3f\n", _localVelocity.x, _localVelocity.y, _localVelocity.z);
}

void NxVehicle::_controlAcceleration(NxReal acceleration, bool analogAcceleration) {
	if (NxMath::abs(acceleration) < 0.001f)
		_releaseBraking = true;
	if (!_braking) {
		_accelerationPedal = NxMath::clamp(acceleration, 1.f, -1.f);
		_brakePedalChanged = _brakePedal == 0;
		_brakePedal = 0;
	} else {
		_accelerationPedal = 0;
		NxReal newv = NxMath::clamp(NxMath::abs(acceleration), 1.f, 0.f);
		_brakePedalChanged = _brakePedal == newv;
		_brakePedal = newv;
	}
	//printf("Acceleration: %2.3f, Braking %2.3f\n", _accelerationPedal, _brakePedal);
}

void NxVehicle::control(NxReal steering, bool analogSteering, NxReal acceleration, bool analogAcceleration, bool handBrake) {
	_controlSteering(steering, analogSteering);
	_computeLocalVelocity();
	if (!_braking || _releaseBraking) {
		_braking = _localVelocity.x * acceleration < (-0.1f /* NxMath::sign(-acceleration)*/);
		_releaseBraking = false;
	}
	//printf("Braking: %s, Handbrake: %s\n", _braking?"true":"false", handBrake?"true":"false");
	if (_handBrake != handBrake) {
		_handBrake = handBrake;
		_brakePedalChanged;
	}
	_controlAcceleration(acceleration, analogAcceleration);


}

void NxVehicle::draw(bool debug) 
	{
	glPushMatrix();

	glDisable(GL_LIGHTING);
	glColor4f(1,1,1,1);
	glBegin(GL_POINTS);
	for (NxU32 x = 0; x < NUM_TRAIL_POINTS; x++)
		{
		glVertex3fv(&(_trailBuffer[x].x));
		}
	glEnd();

	glEnable(GL_LIGHTING);

	float glmat[16]; //OpenGL matrix
	_bodyActor->getGlobalPose().getColumnMajor44(glmat);
	glMultMatrixf(glmat); //send to OpenGL
//	if (debug)
//		glDisable(GL_LIGHTING);

	glPointSize(5.f);
	for (NxU32 i = 0; i < _wheels.size(); i++) 
		{
		_wheels[i]->drawWheel(5, debug);
	}
	if (!debug) {
		glColor4f(0.8f,0.8f,0.8f,1.0f);
		for (NxU32 i = 0; i < _bodyActor->getNbShapes(); i++) {
			NxShape* curShape = _bodyActor->getShapes()[i];
			if (curShape->userData)
				continue;
			glPushMatrix();
			if (NxBoxShape* box = curShape->isBox()) {
				float mat[16];
				box->getLocalPose().getColumnMajor44(mat);
				glMultMatrixf(mat);
				NxVec3 dim = box->getDimensions();
				glScalef(dim.x, dim.y, dim.z);
				glutSolidCube(2);
			}
			if (NxConvexShape* convex = curShape->isConvexMesh()) {
				NxConvexShapeDesc desc;
				convex->saveToDesc(desc);
				//printf("%d\n", desc.meshData->getSubmeshCount());
				for (NxU32 sub = 0; sub < desc.meshData->getSubmeshCount(); sub++) {
					//printf("- %d\n", desc.meshData->getStride(sub, NX_ARRAY_VERTICES));
					//printf("- %d\n", desc.meshData->getStride(sub, NX_ARRAY_TRIANGLES));
					//printf("- %d\n", desc.meshData->getStride(sub, NX_ARRAY_NORMALS));
					//printf("-- %d\n", desc.meshData->getCount(sub, NX_ARRAY_VERTICES));
					//printf("-- %d\n", desc.meshData->getCount(sub, NX_ARRAY_TRIANGLES));
					//printf("-- %d\n", desc.meshData->getCount(sub, NX_ARRAY_NORMALS));
					NxVec3*	vertices	= (NxVec3*)desc.meshData->getBase(sub, NX_ARRAY_VERTICES);
					NxU32 *	faces		= (NxU32*)desc.meshData->getBase(sub, NX_ARRAY_TRIANGLES);
					NxVec3*	normals		= (NxVec3*)desc.meshData->getBase(sub, NX_ARRAY_NORMALS);
					NxU32 nbFaces = desc.meshData->getCount(sub, NX_ARRAY_TRIANGLES);
					glBegin(GL_TRIANGLES);
					for (NxU32 i = 0; i < nbFaces; i++) {
						int index = i*3;
						glNormal3fv(&normals [faces[index+0]].x);
						glVertex3fv(&vertices[faces[index+0]].x);
						
						glNormal3fv(&normals [faces[index+1]].x);
						glVertex3fv(&vertices[faces[index+1]].x);

						glNormal3fv(&normals [faces[index+2]].x);
						glVertex3fv(&vertices[faces[index+2]].x);
					}
					glEnd();
				}
			}
			if (NxCapsuleShape* capsule = curShape->isCapsule()) {
				float mat[16];
				capsule->getLocalPose().getColumnMajor44(mat);
				glMultMatrixf(mat);
				NxReal radius = capsule->getRadius();
				NxReal height = capsule->getHeight();
				static GLUquadricObj* quadric = NULL;
				if (quadric == NULL) {
					quadric = gluNewQuadric();
					gluQuadricDrawStyle(quadric, GLU_FILL);
				}
				glTranslatef(0,-height/2.f, 0);
				glRotatef(90, -1,0,0);
				glutSolidSphere(radius, 40, 20);
				gluCylinder(quadric, radius, radius, height, 40, 1);
				glTranslatef(0,0,height);
				glutSolidSphere(radius, 40, 20);

			}
			glPopMatrix();
		}
	}
	//if (debug)
	glEnable(GL_LIGHTING);

	glPopMatrix();
}

NxReal NxVehicle::_computeAxisTorque() {
	if (_vehicleMotor != NULL) {
		NxReal rpm = _computeRpmFromWheels();
		NxReal motorRpm = _computeMotorRpm(rpm);
		_vehicleMotor->setRpm(motorRpm);
		NxReal torque = _accelerationPedal * _vehicleMotor->getTorque();
		NxReal v = _bodyActor->getLinearVelocity().magnitude();
		//printf("v: %2.3f m/s (%2.3f km/h)\n", v, v*3.6f);
		//printf("rpm %2.3f, motorrpm %2.3f, torque %2.3f, realtorque %2.3f\n",
		//	rpm, motorRpm, torque, torque*_getGearRatio()*_differentialRatio*_transmissionEfficiency);
		return torque * _getGearRatio() * _differentialRatio * _transmissionEfficiency;
	} else {
		_computeRpmFromWheels();
		return _accelerationPedal * _motorForce;
	}
}


NxReal NxVehicle::_computeRpmFromWheels() {
	NxReal wheelRpms = 0;
	NxI32 nbWheels = 0;
	for (NxU32 i = 0; i < _wheels.size(); i++) {
		NxWheel* wheel = _wheels[i];
		if (wheel->getWheelFlag(NX_WF_ACCELERATED)) {
			nbWheels++;
			wheelRpms += wheel->getRpm();
		}
	}
	return wheelRpms / (NxReal)nbWheels;
}

NxReal NxVehicle::_computeMotorRpm(NxReal rpm) {
	NxReal temp = _getGearRatio() * _differentialRatio;
	NxReal motorRpm = rpm * temp;
	if (_vehicleMotor) {
		NxI32 change;
		if (_vehicleGears && (change = _vehicleMotor->changeGears(_vehicleGears, 0.2f))) {
			if (change == 1) {
				gearUp();
			} else {
				NX_ASSERT(change == -1);
				gearDown();
			}
		}
		temp = _getGearRatio() * _differentialRatio;
		motorRpm = NxMath::max(rpm * temp, _vehicleMotor->getMinRpm());
	}
	return motorRpm;
}

NxReal NxVehicle::_getGearRatio() {
	if (_vehicleGears == NULL) {
		return 1;
	} else {
		return _vehicleGears->getCurrentRatio();
	}
}

void NxVehicle::gearUp() {
	if (_vehicleGears) {
		printf("Changing gear from %d to", _vehicleGears->getGear());
		_vehicleGears->gearUp();
		printf(" %d\n", _vehicleGears->getGear());
	} else {
		fprintf(stderr, "gearUp not supported if no gears available\n");
	}
}
void NxVehicle::gearDown() {
	if (_vehicleGears) {
		printf("Changing gear from %d to", _vehicleGears->getGear());
		_vehicleGears->gearDown();
		printf(" %d\n", _vehicleGears->getGear());
	} else {
		fprintf(stderr, "gearDown not supported if no gears available\n");
	}
}

void NxVehicle::applyRandomForce() {
	NxVec3 pos(NxMath::rand(-4.f,4.f),NxMath::rand(-4.f,4.f),NxMath::rand(-4.f,4.f));
	NxReal force = NxMath::rand(_bodyActor->getMass()*0.5f, _bodyActor->getMass() * 2.f);
	_bodyActor->addForceAtLocalPos(NxVec3(0, force*100.f, 0), pos);
}

void NxVehicle::standUp() {
	NxVec3 pos = getActor()->getGlobalPosition() + NxVec3(0,2,0);
	NxQuat rot = getActor()->getGlobalOrientationQuat();
	NxVec3 front(1,0,0);
	rot.rotate(front);
	front.y = 0;
	front.normalize();

	//NxReal dotproduct = front.dot(NxVec3(1,0,0); // too slow
	NxReal dotproduct  = front.x; // equivalent;

	NxReal angle = NxMath::sign(-front.z) * NxMath::acos(dotproduct);

	rot.fromAngleAxis(NxMath::radToDeg(angle), NxVec3(0,1,0));
	getActor()->setGlobalPosition(pos);
	getActor()->setGlobalOrientationQuat(rot);
	getActor()->setLinearVelocity(NxVec3(0,0,0));
	getActor()->setAngularVelocity(NxVec3(0,0,0));
}

void NxVehicle::_updateRpms() {
	NxReal rpm = _computeRpmFromWheels();
	if (_vehicleMotor != NULL) {
		NxReal motorRpm = _computeMotorRpm(rpm);
		_vehicleMotor->setRpm(motorRpm);
	}
}
