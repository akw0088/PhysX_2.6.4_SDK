/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/

#include "OpenGLExtwgl.h"

#if defined(LV_WINDOWS)


PFNWGLCREATEPBUFFERARBPROC            wglCreatePbufferARB           = 0;
PFNWGLGETPBUFFERDCARBPROC             wglGetPbufferDCARB            = 0;
PFNWGLRELEASEPBUFFERDCARBPROC         wglReleasePbufferDCARB        = 0;
PFNWGLDESTROYPBUFFERARBPROC           wglDestroyPbufferARB          = 0;
PFNWGLQUERYPBUFFERARBPROC             wglQueryPbufferARB            = 0;

PFNWGLBINDTEXIMAGEARBPROC             wglBindTexImageARB            = 0;
PFNWGLRELEASETEXIMAGEARBPROC          wglReleaseTexImageARB         = 0;
PFNWGLSETPBUFFERATTRIBARBPROC         wglSetPbufferAttribARB        = 0;

PFNWGLGETPIXELFORMATATTRIBIVARBPROC   wglGetPixelFormatAttribivARB  = 0;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC   wglGetPixelFormatAttribfvARB  = 0;
PFNWGLCHOOSEPIXELFORMATARBPROC        wglChoosePixelFormatARB       = 0;


#ifdef LV_WINDOWS
#define LV_LOADGLEXT(_Type, _Name) temp = (((_Name = (_Type) wglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error); error = temp ? temp : error;
#endif


/*!
**  Notes:    Load the extensions.
**/
LvError glextLoadWGLExt(void)
{
  LvError error = LV_OKAY;
  LvError temp;
  
  LV_LOADGLEXT(PFNWGLCREATEPBUFFERARBPROC,              wglCreatePbufferARB);
  LV_LOADGLEXT(PFNWGLGETPBUFFERDCARBPROC,               wglGetPbufferDCARB);
  LV_LOADGLEXT(PFNWGLRELEASEPBUFFERDCARBPROC,           wglReleasePbufferDCARB);
  LV_LOADGLEXT(PFNWGLDESTROYPBUFFERARBPROC,             wglDestroyPbufferARB);
  LV_LOADGLEXT(PFNWGLQUERYPBUFFERARBPROC,               wglQueryPbufferARB);
  
  LV_LOADGLEXT(PFNWGLBINDTEXIMAGEARBPROC,               wglBindTexImageARB);
  LV_LOADGLEXT(PFNWGLRELEASETEXIMAGEARBPROC,            wglReleaseTexImageARB);
  LV_LOADGLEXT(PFNWGLSETPBUFFERATTRIBARBPROC,           wglSetPbufferAttribARB);
  
  LV_LOADGLEXT(PFNWGLGETPIXELFORMATATTRIBIVARBPROC,     wglGetPixelFormatAttribivARB);
  LV_LOADGLEXT(PFNWGLGETPIXELFORMATATTRIBFVARBPROC,     wglGetPixelFormatAttribfvARB);
  LV_LOADGLEXT(PFNWGLCHOOSEPIXELFORMATARBPROC,          wglChoosePixelFormatARB);
  
  return error;
}

#endif
