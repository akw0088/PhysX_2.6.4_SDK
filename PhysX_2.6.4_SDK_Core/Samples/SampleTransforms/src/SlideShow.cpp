// An app demonstrating reference frames.
// by Adam

#include <stdio.h>
#include <GL/glut.h>

// Physics code
#undef random
#include "NxPhysics.h"

#ifdef _XBOX
#define EXIT_EXAMPLE(x) glutExitExt(x)
#else
#define EXIT_EXAMPLE(x) exit(x)
#endif

extern char displayString[512];
extern void CreateBunny(const NxVec3 & pos);
extern NxPhysicsSDK*	gPhysicsSDK;
extern NxScene*			gScene;

class Slide
	{
	NxReal duration;

	public:
	Slide(NxReal d) : duration(d)
		{
		}

	virtual void start() {}
	virtual void finish() {}
	virtual void render() {}
	virtual void animate(NxReal dt) {}
	bool tick(NxReal dt)
		{
		animate(dt);
		render();
		//autoadvance isn't so good so I disable it.
		//duration -= dt;	
		//return (duration < 0);
		return false;

		}
	};

class Intro : public Slide	{	public:
	Intro() : Slide(5.0f)		{	}
	virtual void render()
		{
		sprintf(displayString, "Hello! This is a self-running tutorial that will teach you about coordinate\nframes in the AGEIA PhysX SDK. You can go to the next slide by pressing space.\nEnjoy!");
		}
	} intro;
class Bunny : public Slide	{	public:
	Bunny() : Slide(5.0f)		{	}
	virtual void start()
		{
		CreateBunny(NxVec3(1.5f, 3.0f, -0.5f));
		}
	virtual void render()
		{
		sprintf(displayString, "Let's look at this cute litte bunny. It is resting on a fixed floor. Both the\nbunny and the floor are Actors. The bunny is dynamic (moveable) while the\nfloor is fixed (static).");
		}
	} bunny;
class VisOn : public Slide	{	public:
	VisOn() : Slide(5.0f)		{	}
	virtual void start()
		{
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);

		}
	virtual void render()
		{
		sprintf(displayString, "We will be using the SDK's debug visualization feature to display some physics\nproperties.  Now you can see some lines representing the floor and the mesh\nstructure of the bunny.");
		}
	} visOn;

class ActorFrame : public Slide	{	public:
	ActorFrame() : Slide(5.0f)		{	}
	virtual void start()
		{
		gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1.0f);

		}
	virtual void render()
		{
		sprintf(displayString, "We now turn on the display of the actor frames. This is the frame of reference\nof each actor, and actors are placed in the world by moving around this frame.");
		}
	} actorFrame;

class ActorFrame2 : public Slide	{	public:
	ActorFrame2() : Slide(5.0f)		{	}
	virtual void start()
		{
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 0);
		

		}
	virtual void render()
		{
		sprintf(displayString, "Actor frames should be determined in a way that is most convenient for placing\nthe object. It is usually selected to be at the bottom of objects that are\nplaced on the ground.");
		}
	} actorFrame2;

class ActorFrame3 : public Slide	{	public:
	ActorFrame3() : Slide(5.0f)		{	}
	virtual void start()
		{
		gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 0.5f);
		CreateBunny(NxVec3(-1.5f, 0.0f, -0.5f));

		}
	virtual void render()
		{
		sprintf(displayString, "Previously we have dropped the bunny from above, but now we'd like to place\nanother one directly on the floor. Because of the convenient location of the\nactor frame, we don't need to worry about the size of the bunny,we can just\ncreate it at height = 0.");
		}
	} actorFrame3;

class ActorFrame4 : public Slide	{	public:
	ActorFrame4() : Slide(7.0f)		{	}
	virtual void animate(NxReal dt)
		{
		static NxReal time;
		time += dt;
		static bool n = false;
		if (time > 0.5f)
			{
			n = !n;
			time = 0;
			gScene->getActors()[1]->setGlobalPosition(NxVec3(1.5f, 0.0f, n ? -0.5f: 0.5f));
			}

		}
	void finish()
		{
		gScene->getActors()[1]->setGlobalPosition(NxVec3(1.5f, 0.0f, -0.5f));
		}

	virtual void render()
		{
		sprintf(displayString, "Because typically the actor frame is made to coincide with the pose of your\ngame object, you can call getGlobalPose() to update the position of your game\nobject in preparation for rendering.  To move an actor manually by placing its\nactor frame in a different place, use NxActor::setGlobalPose(). ");
		}
	} actorFrame4;

class ActorFrame5 : public Slide	{	public:
	ActorFrame5() : Slide(7.0f)		{	}
	void start()
		{
		gScene->getActors()[2]->raiseBodyFlag(NX_BF_KINEMATIC);
		}
	void finish()
		{
		gScene->getActors()[2]->clearBodyFlag(NX_BF_KINEMATIC);
		gScene->getActors()[2]->setGlobalPosition(NxVec3(-1.5f, 0.0f, -0.5f));
		}
	virtual void animate(NxReal dt)
		{
		static NxReal time;
		time += dt;
		gScene->getActors()[2]->moveGlobalPosition(NxVec3(-1.5f, 0.0f, -0.5f * NxMath::sin(time)));
		}
	virtual void render()
		{
		sprintf(displayString, "If you want to move a body around continuously, you should make it kinematic\nusing raiseBodyFlag(BF_KINEMATIC),and then use moveGlobalPose().");
		}
	} actorFrame5;


class BodyFrame : public Slide	{	public:
	BodyFrame() : Slide(5.0f)		{	}
	virtual void start()
		{
  		gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_AXES, 1.0f);
		}
	virtual void render()
		{
		sprintf(displayString, "Dynamic actors move according to the laws of physics.They have a certain mass\nand spin around their center of mass when thrown.  Now we also display the\ncenter of mass. The ground plane is a fixed object so it has no center of mass.");
		}
	} bodyFrames;

class BodyFrame2 : public Slide	{	public:
	BodyFrame2() : Slide(5.0f)		{	}
	virtual void start()
		{
		}
	virtual void render()
		{
		sprintf(displayString, "Note how the center of mass, just like the actor frame, has a certain\norientation, not just a position. The basis vectors that make up this\norientation are known as the moments of inertia. This is a physics\nimplementation detail that you usually don't have to worry about.");
		}
	} bodyFrames2;


class BodyFrame3 : public Slide	{	public:
	BodyFrame3() : Slide(5.0f)		{	}
	virtual void render()
		{
		sprintf(displayString, "So far the center of mass was computed automatically for the bunny given the\nshape of the bunny, and the user did not have to worry about it. Sometimes\nits nice to manually change the center of mass to get certain special effects.");
		}
	} bodyFrames3;

class BodyFrame4 : public Slide	{	public:
	BodyFrame4() : Slide(5.0f)		{	}
	virtual void start()
		{
		gScene->getActors()[1]->setCMassOffsetLocalPosition(NxVec3(0,0.5f,0.8f));
		}
	virtual void render()
		{
		sprintf(displayString, "Here we have moved the bunny's center of mass into its nose - observe how it\ntips over. We used setCMassOffsetLocalPosition (NxVec3(0,0.5,0.8))to do\nthis. The vector passed is relative to the actor frame.");
		}
	} bodyFrames4;

class BodyFrame5 : public Slide	{	public:
	BodyFrame5() : Slide(5.0f)		{	}
	virtual void start()
		{
		gScene->getActors()[2]->setCMassOffsetGlobalPosition(NxVec3(-1.5f,0.5f,0.8f-0.5f));
		}
	virtual void render()
		{
		sprintf(displayString, "We do the same to the other bunny, but this time we specify the new center of\nmass in world coordinates, using setCMassOffsetGlobalPosition().");
		}
	} bodyFrames5;

class BodyFrame6 : public Slide	{	public:
	BodyFrame6() : Slide(5.0f)		{	}
	virtual void start()
		{
		gScene->getActors()[1]->updateMassFromShapes(0, 10);
		gScene->getActors()[2]->updateMassFromShapes(0, 10);
		}
	virtual void render()
		{
		sprintf(displayString, "To compute the correct center of masses based on the shapes again, we use\nupdateMassFromShapes()");
		}
	} bodyFrames6;

class BodyFrame7 : public Slide	{	public:
	BodyFrame7() : Slide(5.0f)		{	}
	virtual void start()
		{
		NxMat34 m;
		m.t.set(1.5,1,-0.5);
		gScene->getActors()[1]->setCMassGlobalPose(m);
		m.t.set(-1.5,1,-0.5);
		gScene->getActors()[2]->setCMassGlobalPose(m);
		}
	virtual void render()
		{
		sprintf(displayString, "It is also possible to *move* the actor by saying where its center of mass\nshould be in world space,using setCMassGlobalPose()");
		}
	} bodyFrames7;

class Shapes : public Slide	{	public:
	Shapes() : Slide(5.0f)		{	}
	virtual void start()
		{
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AXES, 1.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 0.3f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_AXES, 0.4f);
		}
	virtual void render()
		{
		sprintf(displayString, "The triangle mesh representing the bunny is a kind of shape. Other shapes\ninclude boxes and spheres. Each shape also has a coordinate frame. For\ntriangle meshes, the triangle vertices are defined relative to this shape\nframe. We now visualize the shape frames too.");
		}
	} shapes;

class Shapes15 : public Slide	{	public:
	Shapes15() : Slide(5.0f)		{	}
	virtual void start()
		{
		NxSphereShapeDesc sd;
		sd.radius = 0.7f;
		sd.localPose.t.set(0,0.8f,-0.9f);

		gScene->getActors()[2]->createShape(sd);
		}
	virtual void render()
		{
		sprintf(displayString, "You can attach several shapes to an actor.  Here we added a fashionable tail\nornament.  The shape is placed by providing coordinates relative to the actor\nframe.  This way we can use the same coordinates to add more shapes to any\nbunny, regardless of its location in the world or its center of mass, which\nmay have been shifted due to previous added shapes.");
		}
	} shapes15;


class Shapes1 : public Slide	{	public:
	Shapes1() : Slide(5.0f)		{	}
	virtual void render()
		{
		sprintf(displayString, "When you add new shapes to the actor, the mass properties (including the\ncenter of mass) are not automatically updated.  You can see that both bunnies\nstill have the same center of mass despite the added sphere.");
		}
	} shapes1;

class Shapes2 : public Slide	{	public:
	Shapes2() : Slide(5.0f)		{	}
	virtual void start()
		{
		gScene->getActors()[2]->updateMassFromShapes(0, 10);

		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_AXES, 0.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 0);
		gPhysicsSDK->setParameter(NX_VISUALIZE_BODY_AXES, 1.0f);
		}
	virtual void render()
		{
		sprintf(displayString, "We now call updateMassFromShapes() to recompute the mass properties.  As a\nresult, the center of mass of the second bunny is moved backwards.");
		}
	} shapes2;

class End : public Slide	{	public:
	End() : Slide(5.0f)		{	}
	virtual void start()
		{
		}
	virtual void render()
		{
		sprintf(displayString, "The END -- for now.");
		}
	} end;

/*
class Intro : public Slide	{	public:
	Intro() : Slide(5.0f)		{	}
	virtual void render()
		{
		sprintf(displayString, "Hello World!Isn't this cool?");
		}
	} intro;
*/
class Last : public Slide	{	public:
	Last() : Slide(0.0f)		{	}
	} last;


Slide * slides[] = { &intro, &bunny, &visOn, 
	&actorFrame, &actorFrame2, &actorFrame3, &actorFrame4, &actorFrame5, 
	&bodyFrames, &bodyFrames2, &bodyFrames3, &bodyFrames4, &bodyFrames5, &bodyFrames6, &bodyFrames7, 
	&shapes, &shapes15, &shapes1, &shapes2,
	&end, &last };


void tick(float dt, bool click)
	{
	static NxU32 current = 0;

	if (slides[current]->tick(dt) || click)
		{
		slides[current]->finish();
		if (slides[current] == &last)
			EXIT_EXAMPLE(0);
		current++;
		slides[current]->start();
		}
	}
