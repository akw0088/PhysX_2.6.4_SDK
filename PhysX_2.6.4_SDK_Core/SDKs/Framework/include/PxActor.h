#ifndef NX_FRAMEWORK_PXACTOR
#define NX_FRAMEWORK_PXACTOR

#include "Nx.h"
#include "NxArray.h"

#include "px_lowlevel.h"
#include "PxScene.h"

class PxScene;
class PxInteraction;
class PxActor;

// The interaction marker represents an interaction between two actors that are not used or is deactivated
// (i.e. filtered overlap interactions). Each actor tracks a list of markers that contain a pointer to the 
// second actor of the interaction. 
class PxInteractionMarker
{
	friend class PxActor;
public:
	PxInteractionMarker(PxActor& actor) : next(NULL), previous(NULL), twin(NULL), otherActor(actor), dirty(false) { }
	~PxInteractionMarker() { NX_ASSERT(!next && !previous);  }
	NX_INLINE PxActor& getActor() const { return otherActor; }
	NX_INLINE void detach();
	NX_INLINE void setTwinMarker(PxInteractionMarker* twinMarker) { twin = twinMarker; }
	NX_INLINE PxInteractionMarker* getTwinMarker() const { return twin; }
	NX_INLINE PxInteractionMarker* getNext() const { return next; }
	NX_INLINE PxInteractionMarker* getPrevious() const { return previous; }
	NX_INLINE bool isDirty() const { return dirty; }
	NX_INLINE void setDirty(bool value) { dirty = value; }
private:
	PxInteractionMarker* next;
	PxInteractionMarker* previous;
	PxInteractionMarker* twin;
	PxActor& otherActor;
	bool dirty;
};


// The actor represents a simulated item that can be
// connected to other actors through interactions. An actor
// can be either active or inactive. 

enum
{
	PX_ACTOR_TYPE_DYNAMIC = 0,
	PX_ACTOR_TYPE_STATIC,
	PX_ACTOR_TYPE_RAWBOUNDS,
	PX_ACTOR_TYPE_PAGEBOUNDS,
	PX_ACTOR_TYPE_COUNT,
};

class PxActor
{
	friend class PxInteraction;
	friend class PxScene;
public:
	//Actors automatically unregister themselves from the scene here
	virtual void destroy();

	// Get the scene the actor resides in
	NX_INLINE PxScene& getScene() const { return ownerScene; }

	// Get whether the actor is active or not
	NX_INLINE bool isActive() const { return active; }

	// Get the number of interactions connected to the actor
	NX_INLINE unsigned int getInteractionCount() const { return interactions.size(); }

	// Get an iterator to the interactions connected to the actor
	NX_INLINE PxIterator<PxInteraction*const> getInteractions() const { return PxIterator<PxInteraction*const>(interactions.begin(), interactions.end()); }

	// Get the type ID of the actor
	NX_INLINE NxU32 getType() const { return actorType; }

	// Set active state - should not be called in the midst of a simulation step
	void setActive(bool active);

	//Actors automatically register themselves in the scene here
	PxActor(PxScene& scene, bool active, NxU32 actorType);

	// Queried whenever a body wants to be activated
	virtual bool canActivate() const { return false; }

	// Queried every step for deactivation readiness
	virtual bool canDeactivate() const = 0;

	// Gets the low level volume of the actor
	NX_INLINE PxdVolume getLowLevelVolume() const { return llVolume; }

	// Sets the low level volume of the actor
	NX_INLINE void setLowLevelVolume(PxdVolume volume) { llVolume = volume; }

	// Increase static touch counter
	NX_INLINE void incStaticTouch() { staticTouchCount++; }

	// Decrease static touch counter
	NX_INLINE void decStaticTouch() { staticTouchCount--; }

	// Returns the static touch counter
	NX_INLINE NxU32 getStaticTouch() { return staticTouchCount; }

	// Get the number of interactions of a specific type
	NX_INLINE NxU32 getNumInteractionsOfType(NxU32 interactionType) const { NX_ASSERT(interactionType<PX_INTERACTION_TYPE_COUNT); return numInteractionsOfType[interactionType]; }

	// Get the index of the island that the actors is currently part of (Only valid for actors that have a valid island)
	NX_INLINE NxU32 getIslandIndex() const { return islandIndex; }

	// Returns whether the actor is valid (not pending deletion) or not
	NX_INLINE NxU32 isValid() const { return id != -1; }

	// Really ugly hack to get from PxActor to NvBody in the HSM. Needs to be redone, but there's no time now...
	virtual void* getAppData() { return 0; }

	// Returns whether this actor has a valid island, i.e. participated in last generateIslands or has an island explicitly generated aftwards
	NX_INLINE bool hasValidIsland() const { return timestamp == ownerScene.timestamp && islandIndex != -1; }

	// Temporary fix. Needed so that kinematics can tell whether they have active interactions or not.
	NX_INLINE bool hasActiveInteraction() const { return numActiveInteractions != 0; }

	// Updates the state (activity transfer) of the interaction
	void notifyInteractionTransferChange(PxInteraction* interaction);

	// Returns whether this actor should be able to build islands on its own or not
	virtual bool isIslandGenerating() const { return true; }

	NX_INLINE void incTransferCount();
	NX_INLINE void decTransferCount();

	NX_INLINE void addOverlapMarker(PxInteractionMarker* marker);
	NX_INLINE void removeOverlapMarker(PxInteractionMarker* marker);
	NX_INLINE NxU32 getOverMarkerCount() const { return overlapMarkerCount; }
	NX_INLINE PxInteractionMarker* getOverlapMarkerRoot() const { return overlapMarkerRoot; }
protected:
	virtual ~PxActor();

	// Called by the framework when an actor is deactivated
	virtual void onActivate() {}
	// Called by the framework when an actor is deactivated
	virtual void onDeactivate() {}

	// Temporary fix. Needed so that kinematics can tell whether they have active interactions or not.
	// Called by the framework when the last active interaction deactivates or the first inactive interaction activates.
	virtual void onAllInteractionsInactiveChange(bool allInactive) {};

private:

	//These are called from interaction creation/destruction
	void registerInteraction(PxInteraction* interaction);
	void unregisterInteraction(PxInteraction* interaction);

	// Temporary fix. Needed so that kinematics can tell wether they have active interactions or not.
	void notifyInteractionActiveChange(PxInteraction* interaction);
	NX_INLINE void incActiveInteractions();
	NX_INLINE void decActiveInteractions();

	NxU32										id; // Used by PxScene 

	NxU32										actorType;

	PxScene&									ownerScene;
	NxFoundation::NxArraySDK<PxInteraction*>	interactions;
	NxU32										numInteractionsOfType[PX_INTERACTION_TYPE_COUNT];
	NxU32										numTransferringInteractions;
	NxU32										numActiveInteractions;
	PxInteractionMarker*						overlapMarkerRoot;
	NxU32										overlapMarkerCount;

	PxdVolume									llVolume;

	NxU32										timestamp;
	NxU32										staticTouchCount;
	NxU32										islandIndex;
	NxU32										transferCount;

	bool										active;
};

//////////////////////////////////////////////////////////////////////////

NX_INLINE void PxActor::incActiveInteractions()
{
	++numActiveInteractions;
}

NX_INLINE void PxActor::decActiveInteractions()
{
	NX_ASSERT(numActiveInteractions>0);
	--numActiveInteractions;
}

NX_INLINE void PxActor::incTransferCount()
{
	++transferCount;
}

NX_INLINE void PxActor::decTransferCount()
{
	NX_ASSERT(transferCount>0);
	--transferCount;
}

NX_INLINE void PxActor::addOverlapMarker(PxInteractionMarker* marker)
{
	// Insert before root
	NX_ASSERT(!marker->next && !marker->previous);
	if (overlapMarkerRoot)
	{
		NX_ASSERT(!overlapMarkerRoot->previous);
		marker->next = overlapMarkerRoot;
		overlapMarkerRoot->previous = marker;
	}
	overlapMarkerRoot = marker;
	++overlapMarkerCount;
}

NX_INLINE void PxActor::removeOverlapMarker(PxInteractionMarker* marker)
{
	if (!overlapMarkerRoot)
		return;
	else if (overlapMarkerRoot == marker)
	{
		if (overlapMarkerRoot->next)
		{
			overlapMarkerRoot = marker->next;
			marker->next->previous = NULL;
		}
		else
			overlapMarkerRoot = NULL;
	}
	marker->detach();
	--overlapMarkerCount;
}

//////////////////////////////////////////////////////////////////////////

NX_INLINE void PxInteractionMarker::detach()
{
	if (previous)
		previous->next = next;
	if (next)
		next->previous = previous;
	next = NULL;
	previous = NULL;
}

#endif // NX_FRAMEWORK_PXACTOR
