#ifndef OBJECTS_H
#define OBJECTS_H

void renderCube(const NxVec3& color, NxF32 size);
void renderSphere(const NxVec3& color, NxF32 size);
void renderCapsule(const NxVec3& color, NxF32 r, NxF32 h);

// Base wrapper object kept in NxActor's user-data field
class NxObject// : public NxAllocateable
{
	public:
								NxObject(NxActor* actor);
	virtual						~NxObject();

	virtual		void			render()	const	= 0;

	NX_INLINE	NxActor*		getOwner()	const			{ return owner;	}
	NX_INLINE	const NxVec3&	getColor()	const			{ return color;	}
	NX_INLINE	void			setColor(const NxVec3& c)	{ color = c;	}

	protected:
				NxActor*		owner;
				NxVec3			color;
};

// Cube wrapper
class NxCubeObject : public NxObject
{
	public:
								NxCubeObject(NxActor* actor, NxF32 s);
	virtual						~NxCubeObject();

	virtual		void			render()	const;

				NxF32			size;
};

// Sphere wrapper
class NxSphereObject : public NxObject
{
	public:
								NxSphereObject(NxActor* actor, NxF32 s);
	virtual						~NxSphereObject();

	virtual		void			render()	const;

				NxF32			size;
};

// Capsule wrapper
class NxCapsuleObject : public NxObject
{
	public:
								NxCapsuleObject(NxActor* actor, NxF32 radius, NxF32 height);
	virtual						~NxCapsuleObject();

	virtual		void			render()	const;

				NxF32			radius;
				NxF32			height;
};

// Convex wrapper
class NxConvexObject : public NxObject
{
	public:
								NxConvexObject(NxActor* actor);
	virtual						~NxConvexObject();

	virtual		void			render()	const;
};

#endif
