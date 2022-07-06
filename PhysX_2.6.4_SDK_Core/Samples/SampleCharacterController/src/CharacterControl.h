#ifndef CHARACTERCONTROL_H
#define CHARACTERCONTROL_H

	enum GameGroup
		{
		GROUP_NON_COLLIDABLE,
		GROUP_COLLIDABLE_NON_PUSHABLE,
		GROUP_COLLIDABLE_PUSHABLE,
		};

	#define COLLIDABLE_MASK	(1<<GROUP_COLLIDABLE_NON_PUSHABLE) | (1<<GROUP_COLLIDABLE_PUSHABLE)

	void					initCharacterControllers(NxU32 nbCharacters, const NxVec3* startPos, NxScene& scene);
	void					releaseCharacterControllers(NxScene& scene);

	void					updateControllers();

	NxU32					moveCharacter(NxU32 characterIndex, NxScene& scene, const NxVec3& dispVector, NxF32 elapsedTime, NxU32 collisionGroups, NxF32 heightDelta);
	const NxExtendedVec3&	getCharacterPos(NxU32 characterIndex);
	NxActor*				getCharacterActor(NxU32 characterIndex);
	bool					resetCharacterPos(NxU32 characterIndex, const NxVec3& pos);
	bool					updateCharacterExtents(NxU32 characterIndex, bool& increase);
	void					reportSceneChanged(NxU32 characterIndex);

	void					renderCharacters();

#endif
