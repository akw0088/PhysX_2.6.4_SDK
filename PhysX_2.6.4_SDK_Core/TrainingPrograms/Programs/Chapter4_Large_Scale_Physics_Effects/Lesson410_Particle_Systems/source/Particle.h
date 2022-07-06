// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					            PARTICLE SYSTEM CLASS
//
//						  Written by Sam Thompson, 3-17-05
//						    Edited by Bob Schade, 5-1-06
// ===============================================================================

#ifndef PARTICLE_H
#define PARTICLE_H

#include "NxActor.h"
#include "NxSphereShape.h"
#include "NxSphereShapeDesc.h"
#include "NxScene.h"
#include <stdio.h>

const int ciMaxParticles = 256;
const NxReal cfTimeBetweenParticles = 0.05f;

const NxCollisionGroup cParticleCollisionGroup = 1;

// Make a simple particle class
class Particle 
{
	public:
		Particle();
		Particle( NxActor* pToSet, NxVec3& vThermoDynamicAcceleration );
		void update( NxReal fDeltaTime );
		void setActor( NxActor* pToSet );
		NxActor& getActorReference( );
	private:
		NxActor* _pActor;
		NxVec3 _vThermoDynamicForce;
};

class ParticleEmitter 
{
	public:
		ParticleEmitter(NxScene*& pSceneToSet, NxVec3 vStartingPos, NxReal& fStartingVelScale, NxVec3 vThermoDynamicAcceleration);
		~ParticleEmitter();
		void removeAllParticles();
		void update(NxReal fDeltaTime);
		void addParticle();
		void removeParticle();
	private:
		NxScene* _pScene;
		Particle* _aParticleArray[ciMaxParticles];
		int _iHeadIndex;
		int _iTailIndex;
		int _iParticleCount;
		NxVec3 _vStartingPos;
		NxReal _fStartingVelScale;
		NxReal _fNewParticleTimer;
		NxVec3 _vThermoDynamicAcceleration;
};

#endif
