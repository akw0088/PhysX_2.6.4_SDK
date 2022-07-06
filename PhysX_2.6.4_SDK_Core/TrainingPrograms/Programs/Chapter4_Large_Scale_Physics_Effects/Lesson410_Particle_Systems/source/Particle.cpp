// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//					            PARTICLE SYSTEM CLASS
//
//						  Written by Sam Thompson, 3-17-05
//						    Edited by Bob Schade, 5-1-06
// ===============================================================================

#include "Particle.h"

Particle::Particle()
{
	setActor(NULL);
	_vThermoDynamicForce.zero();
}

Particle::Particle(NxActor* pToSet, NxVec3& vThermoDynamicAcceleration)
{
	setActor(pToSet);

	// F = M*a
	_vThermoDynamicForce = pToSet->getMass()*vThermoDynamicAcceleration;
}

void Particle::setActor(NxActor* pToSet)
{
	_pActor = pToSet;
}

void Particle::update(NxReal fDeltaTime)
{
	// particles have a false, thermodynamic force applied which pushes them up every frame
	_pActor->addForce(_vThermoDynamicForce);

	// Use our down-caster to get a sphere pointer (and make sure it is a sphere)
	NxSphereShape* pSphere = _pActor->getShapes()[0]->isSphere();
	const NxReal fMaxRadius = 3.0f;
	const NxReal fGrowthRate = 0.6f;
	if (pSphere)
	{
		// Grow the radius at growth rate fGrowthRate m/s, until it reaches fMaxRadius meters
		NxReal fCurrentRadius = pSphere->getRadius();
		if (fCurrentRadius < fMaxRadius)
		{
			pSphere->setRadius(pSphere->getRadius() + fGrowthRate*fDeltaTime);
			if (pSphere->getRadius() > fMaxRadius)
				pSphere->setRadius(fMaxRadius);
		}
	}
}

NxActor& Particle::getActorReference( )
{
	return *_pActor;
}

ParticleEmitter::ParticleEmitter(NxScene*& pSceneToSet, NxVec3 vStartingPos, NxReal& fStartingVelScale, NxVec3 vThermoDynamicAcceleration)
{
	_iHeadIndex = 0;
	_iTailIndex = 0;
	_iParticleCount = 0;
	_pScene = pSceneToSet;
	_vStartingPos = vStartingPos;
	_fStartingVelScale = (1.0f / NxReal(RAND_MAX)) * fStartingVelScale;
	_fNewParticleTimer = 0.0f;
	_vThermoDynamicAcceleration = vThermoDynamicAcceleration;
}

ParticleEmitter::~ParticleEmitter()
{
	removeAllParticles();
}

void ParticleEmitter::removeAllParticles()
{
	while (_iParticleCount > 0)
		removeParticle();
}

void ParticleEmitter::update(NxReal fDeltaTime)
{
	_fNewParticleTimer -= fDeltaTime;
	if (_fNewParticleTimer < 0.0f)
	{
		addParticle();
		_fNewParticleTimer += cfTimeBetweenParticles;
	}

	// Update all particles
	int iParticlesUpdated = 0;
	for (int iParticleIndex = _iTailIndex; iParticlesUpdated < _iParticleCount; iParticleIndex = ((iParticleIndex+1) % ciMaxParticles))
	{
		_aParticleArray[iParticleIndex]->update(fDeltaTime);
		++iParticlesUpdated;
	}
}

void ParticleEmitter::addParticle()
{

	if ((_iTailIndex == _iHeadIndex) && (_iParticleCount != 0)) // FIFO is full
	{
		removeParticle();
		// Now there is a slot free
	}

	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = 1.0f;
	sphereDesc.group = cParticleCollisionGroup; // this group does not collide with itself
	actorDesc.shapes.pushBack(&sphereDesc);

	actorDesc.body = &bodyDesc;
	actorDesc.density = 10.0f;
	actorDesc.globalPose.t = _vStartingPos;

	NxActor* pNewActor =  _pScene->createActor(actorDesc);

	// Give it an initial linear velocity, scaled by _fStartingVelScale
	NxVec3 vRandVec(NxReal(rand()*_fStartingVelScale), NxReal(rand()*_fStartingVelScale), NxReal(rand()*_fStartingVelScale));
	pNewActor->setLinearVelocity(vRandVec);

	// Turn off gravity for smoke
	pNewActor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);

	_aParticleArray[_iHeadIndex] = new Particle(pNewActor, _vThermoDynamicAcceleration);

	_iHeadIndex = (_iHeadIndex+1) % ciMaxParticles;
	++_iParticleCount;
}

void ParticleEmitter::removeParticle( )
{
	// release the actor the tail particle owns
	_pScene->releaseActor(_aParticleArray[_iTailIndex]->getActorReference());
	// delete it
	delete _aParticleArray[_iTailIndex];
	// increment the tail
	_iTailIndex = (_iTailIndex+1) % ciMaxParticles;
	--_iParticleCount;
}
