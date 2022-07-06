/*----------------------------------------------------------------------------*\
|
|						     Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxPhysics.h"

class PrefsRenderer
{

private:
	bool m_profilerOn;

	static const unsigned int FPS_AVG_FRAMES=15;

	float fpsTimes[FPS_AVG_FRAMES];//we measure the fps as an average of the last n frames.
	int fpsIndex;

	float lastTime;

public:
	PrefsRenderer();
	void toggleEnable();
	void render(const NxProfileData* prof, int width, int height);

private:
	float computeFPS();

};
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
