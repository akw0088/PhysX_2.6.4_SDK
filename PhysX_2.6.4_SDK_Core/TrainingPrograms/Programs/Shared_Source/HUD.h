// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                       HEADS-UP DISPLAY
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#ifndef HUD_H
#define HUD_H

#include <GL/glut.h>
#include <stdio.h>

#include "NxPhysics.h"

class DisplayString
{
public:

    char m_string[512];
	NxReal m_xpos;
	NxReal m_ypos;

	DisplayString()  {}
	~DisplayString()  {}

	void Set(char* s, NxReal x, NxReal y)
	{
        sprintf(m_string, s);
		m_xpos = x;
		m_ypos = y;
	}
};

class HUD
{
public:
	NxArray<DisplayString> m_DisplayString;

	HUD();
    ~HUD();

    void AddDisplayString(char* s, NxReal x, NxReal y);
    void SetDisplayString(NxU32 i, char* s, NxReal x, NxReal y);
    void Render();
};


#endif  // HUD_H

