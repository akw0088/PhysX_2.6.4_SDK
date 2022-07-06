/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/

#include "OpenGLExtVBO.h"
#ifdef LV_APPLE
#include "aglGetProcAddress.h"
#endif

PFNGLGENBUFFERSARBPROC            glGenBuffersARB             = 0;
PFNGLBINDBUFFERARBPROC            glBindBufferARB             = 0;
PFNGLBUFFERDATAARBPROC            glBufferDataARB             = 0;
PFNGLDELETEBUFFERSARBPROC         glDeleteBuffersARB          = 0;
PFNGLGETBUFFERPARAMETERIVARBPROC  glGetBufferParameterivARB   = 0;

#ifdef LV_WINDOWS
#define LV_LOADGLEXT(_Type, _Name) (((_Name = (_Type) wglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error)
#endif

#ifdef LV_APPLE
#define LV_LOADGLEXT(_Type, _Name) (((_Name = (_Type) aglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error)
#endif

/*!
**  Notes:    Load the extensions.
**/
LvError glextLoadVBOExt(void)
{
  LvError error = LV_OKAY;
  
  error = LV_LOADGLEXT(PFNGLGENBUFFERSARBPROC,            glGenBuffersARB);
  error = LV_LOADGLEXT(PFNGLBINDBUFFERARBPROC,            glBindBufferARB);
  error = LV_LOADGLEXT(PFNGLBUFFERDATAARBPROC,            glBufferDataARB);
  error = LV_LOADGLEXT(PFNGLDELETEBUFFERSARBPROC,         glDeleteBuffersARB);
  error = LV_LOADGLEXT(PFNGLGETBUFFERPARAMETERIVARBPROC,  glGetBufferParameterivARB);
  
  return error;
}
