#ifndef NX_EXTENSION_H
#define NX_EXTENSION_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

enum NxExtensionType
{
	NX_EXT_SCREEN_SURFACE,
	NX_EXT_LAST
};

class NxExtension
{
public:
  virtual NxExtensionType getExtensionType(void)    const = 0;
  virtual unsigned int    getExtensionVersion(void) const = 0;
};


#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
