#ifndef NX_FRAMEWORK_PXINTERACTION
#define NX_FRAMEWORK_PXINTERACTION

#include "Nx.h"
#include "PxActor.h"
#include "Allocateable.h"

// Interactions are used for connecting actors into activation
// groups. An interaction always connects exactly two actors. 
// An interaction is implicitly active if at least one of the two 
// actors it connects is active.
// Todo: we might need an interaction callback mechanism

class PxInteraction : public NxFoundation::NxAllocateable
{
	friend class PxScene;
	friend class PxActor;
public:
	//Interactions automatically unregister themselves from the actors here
	virtual ~PxInteraction();
 
	NX_INLINE PxActor& getActor0() const { return actor0; }
	NX_INLINE PxActor& getActor1() const { return actor1; }

	// Returns true if the interaction is active
	virtual bool isActive() const = 0;

	// Get the type ID of the interaction
	NX_INLINE PxInteractionType getType() const { return interactionType; }

protected:
	// Interactions automatically register themselves in the actors here
	// Even though in theory you could have actors from different scenes here
	// it should not be allowed
	PxInteraction(PxActor& actor0, PxActor& actor1, PxInteractionType interactionType);

	// HACK - Needs to be called by overloaded constructor to get around virtual "isActive" call
	void init();
	void deinit();
	
	// Called by the framework when an actor changes activity
	// This is needed because HL needs interactions to be active if at least one actor is active.
	// LL managers can currently only be active if both atoms are active.
	virtual void onActorActiveChange(PxActor* actor) {};


	NX_INLINE void registerInteraction() { actor0.getScene().registerInteraction(this); }
	NX_INLINE void unregisterInteraction() { actor0.getScene().unregisterInteraction(this); }

	// Called by the framework when an interaction is activated
	virtual void onActivate() {}
	// Called by the framework when an interaction is deactivated
	virtual void onDeactivate() {}

	PxInteractionType interactionType;

	NX_INLINE void setActorId (PxActor *actor, NxU32 id) { if (&actor0 == actor) actorId0 = id; else actorId1 = id; }
	NX_INLINE NxU32 getActorId (PxActor *actor) { if (&actor0 == actor) return actorId0; else return actorId1; }

private:
	PxActor &actor0;
	PxActor &actor1;

	NxU32 sceneId;
	NxU32 actorId0;
	NxU32 actorId1;

protected:
	// Should be false in case of a disjoint interaction (e.g. filtered SIPs)
	bool canTransferActivity;
};

#endif // NX_FRAMEWORK_PXINTERACTION
