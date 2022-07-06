/*----------------------------------------------------------------------------*\
|
|						     AGEIA PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#ifndef DEBUGRENDER_H
#define DEBUGRENDER_H

class NxDebugRenderable;

class DebugRenderer
{
public:
	void renderData(const NxDebugRenderable& data) const;

private:
	static void renderBuffer(float* pVertList, float* pColorList, int type, int num);
};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005-2006 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
