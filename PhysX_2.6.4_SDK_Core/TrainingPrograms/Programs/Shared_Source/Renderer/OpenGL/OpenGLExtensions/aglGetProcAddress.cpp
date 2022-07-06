/*
 *  aglGetProcAddress.cpp
 *  Lava
 *
 *  Created by James Dolan on 2/6/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "aglGetProcAddress.h"

static CFBundleRef gBundleRefOpenGL = NULL;

// -------------------------

OSStatus aglInitEntryPoints (void)
{
    OSStatus err = noErr;
    const Str255 frameworkName = "\pOpenGL.framework";
    FSRefParam fileRefParam;
    FSRef fileRef;
    CFURLRef bundleURLOpenGL;
    
    memset(&fileRefParam, 0, sizeof(fileRefParam));
    memset(&fileRef, 0, sizeof(fileRef));
    
    fileRefParam.ioNamePtr  = frameworkName;
    fileRefParam.newRef = &fileRef;
    
    // Frameworks directory/folder
    err = FindFolder (kSystemDomain, kFrameworksFolderType, false,
                      &fileRefParam.ioVRefNum, &fileRefParam.ioDirID);
    if (noErr != err) {
        DebugStr ("\pCould not find frameworks folder");
        return err;
    }
    err = PBMakeFSRefSync (&fileRefParam); // make FSRef for folder
    if (noErr != err) {
        DebugStr ("\pCould make FSref to frameworks folder");
        return err;
    }
    // create URL to folder
    bundleURLOpenGL = CFURLCreateFromFSRef (kCFAllocatorDefault,
                                            &fileRef);
    if (!bundleURLOpenGL) {
        DebugStr ("\pCould create OpenGL Framework bundle URL");
        return paramErr;
    }
    // create ref to GL's bundle
    gBundleRefOpenGL = CFBundleCreate (kCFAllocatorDefault,
                                       bundleURLOpenGL);
    if (!gBundleRefOpenGL) {
        DebugStr ("\pCould not create OpenGL Framework bundle");
        return paramErr;
    }
    CFRelease (bundleURLOpenGL); // release created bundle
                                 // if the code was successfully loaded, look for our function.
    if (!CFBundleLoadExecutable (gBundleRefOpenGL)) {
        DebugStr ("\pCould not load MachO executable");
        return paramErr;
    }
    return err;
}

// -------------------------

void aglDellocEntryPoints (void)
{
    if (gBundleRefOpenGL != NULL) {
        // unload the bundle's code.
        CFBundleUnloadExecutable (gBundleRefOpenGL);
        CFRelease (gBundleRefOpenGL);
        gBundleRefOpenGL = NULL;
    }
}

// -------------------------

void * aglGetProcAddress (char * pszProc)
{
    return CFBundleGetFunctionPointerForName (gBundleRefOpenGL, CFStringCreateWithCStringNoCopy (NULL, pszProc, CFStringGetSystemEncoding (), NULL));
}
