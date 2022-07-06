#ifndef CHARACTERCONTROL_H
#define CHARACTERCONTROL_H

	enum GameGroup
	{
		GROUP_NON_COLLIDABLE,
		GROUP_COLLIDABLE_NON_PUSHABLE,
		GROUP_COLLIDABLE_PUSHABLE,
	};

	#define COLLIDABLE_MASK	(1<<GROUP_COLLIDABLE_NON_PUSHABLE) | (1<<GROUP_COLLIDABLE_PUSHABLE)

	NxController* InitCharacterControllers(NxU32 nbCharacters, NxVec3 pos, NxReal scale, NxScene& scene);
	void ReleaseCharacterControllers(NxScene& scene);

	void UpdateControllers();

	NxU32 MoveCharacter(NxU32 characterIndex, NxScene& scene, const NxVec3& dispVector, NxF32 elapsedTime, NxU32 collisionGroups, NxF32 heightDelta);
	const NxExtendedVec3& GetCharacterPos(NxU32 characterIndex);
	NxActor* GetCharacterActor(NxU32 characterIndex);
	bool ResetCharacterPos();
	bool UpdateCharacterExtents();

	void RenderCharacters();

#endif
