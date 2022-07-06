/*
 *  aglGetProcAddress.h
 *  Lava
 *
 *  Created by Brian Gilman on 1/17/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef AGLGETPROCADDRESS_H
#define AGLGETPROCADDRESS_H

#include <Carbon/Carbon.h>

// -------------------------

OSStatus aglInitEntryPoints (void);

// -------------------------

void aglDellocEntryPoints (void);

// -------------------------

void * aglGetProcAddress (char * pszProc);

#endif
