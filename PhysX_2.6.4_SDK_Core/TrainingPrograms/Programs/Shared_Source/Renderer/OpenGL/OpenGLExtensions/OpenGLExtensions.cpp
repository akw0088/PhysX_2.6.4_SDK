/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/

#include "OpenGLExtensions.h"

PFNGLPOINTPARAMETERFVPROC                       glPointParameterfvARB                     = 0;
PFNGLPOINTPARAMETERFPROC                        glPointParameterfARB                      = 0;

PFNGLACTIVESTENCILFACEEXTPROC                   glActiveStencilFaceEXT                    = 0;
PFNGLSTENCILOPSEPARATEATIPROC                   glStencilOpSeparateATI                    = 0;
PFNGLSTENCILFUNCSEPARATEATIPROC                 glStencilFuncSeparateATI                  = 0;

PFNGLISRENDERBUFFEREXTPROC                      glIsRenderbufferEXT                       = 0;
PFNGLBINDRENDERBUFFEREXTPROC                    glBindRenderbufferEXT                     = 0;
PFNGLDELETERENDERBUFFERSEXTPROC                 glDeleteRenderbuffersEXT                  = 0;
PFNGLGENRENDERBUFFERSEXTPROC                    glGenRenderbuffersEXT                     = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC                 glRenderbufferStorageEXT                  = 0;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC          glGetRenderbufferParameterivEXT           = 0;
PFNGLISFRAMEBUFFEREXTPROC                       glIsFramebufferEXT                        = 0;
PFNGLBINDFRAMEBUFFEREXTPROC                     glBindFramebufferEXT                      = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC                  glDeleteFramebuffersEXT                   = 0;
PFNGLGENFRAMEBUFFERSEXTPROC                     glGenFramebuffersEXT                      = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC              glCheckFramebufferStatusEXT               = 0;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC                glFramebufferTexture1DEXT                 = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC                glFramebufferTexture2DEXT                 = 0;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC                glFramebufferTexture3DEXT                 = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC             glFramebufferRenderbufferEXT              = 0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT  = 0;
PFNGLGENERATEMIPMAPEXTPROC                      glGenerateMipmapEXT                       = 0;


#ifdef LV_WINDOWS
#define LV_LOADGLEXT(_Type, _Name) (((_Name = (_Type) wglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error)
#endif

#ifdef LV_APPLE
#define LV_LOADGLEXT(_Type, _Name) (((_Name = (_Type) aglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error)
#endif


/*!
**  Notes:    Load the extensions.
**/
LvError glextLoad(void)
{
  LvError error = LV_OKAY;
  LvError temp;
  
  temp = glextLoadShaderObject(); error = temp ? temp : error;
  temp = glextLoadMultText();     error = temp ? temp : error;
  temp = glextLoadVBOExt();       error = temp ? temp : error;
  
  #if defined(LV_WINDOWS)
  temp = glextLoadWGLExt();       error = temp ? temp : error;
  #endif
  
  LV_LOADGLEXT(PFNGLPOINTPARAMETERFVPROC,       glPointParameterfvARB);
  LV_LOADGLEXT(PFNGLPOINTPARAMETERFPROC,        glPointParameterfARB);
  
  LV_LOADGLEXT(PFNGLACTIVESTENCILFACEEXTPROC,   glActiveStencilFaceEXT);
  
  LV_LOADGLEXT(PFNGLSTENCILOPSEPARATEATIPROC,   glStencilOpSeparateATI);
  LV_LOADGLEXT(PFNGLSTENCILFUNCSEPARATEATIPROC, glStencilFuncSeparateATI); 
  
  LV_LOADGLEXT(PFNGLISRENDERBUFFEREXTPROC,                      glIsRenderbufferEXT);
  LV_LOADGLEXT(PFNGLBINDRENDERBUFFEREXTPROC,                    glBindRenderbufferEXT);
  LV_LOADGLEXT(PFNGLDELETERENDERBUFFERSEXTPROC,                 glDeleteRenderbuffersEXT);
  LV_LOADGLEXT(PFNGLGENRENDERBUFFERSEXTPROC,                    glGenRenderbuffersEXT);
  LV_LOADGLEXT(PFNGLRENDERBUFFERSTORAGEEXTPROC,                 glRenderbufferStorageEXT);
  LV_LOADGLEXT(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC,          glGetRenderbufferParameterivEXT);
  LV_LOADGLEXT(PFNGLISFRAMEBUFFEREXTPROC,                       glIsFramebufferEXT);
  LV_LOADGLEXT(PFNGLBINDFRAMEBUFFEREXTPROC,                     glBindFramebufferEXT);
  LV_LOADGLEXT(PFNGLDELETEFRAMEBUFFERSEXTPROC,                  glDeleteFramebuffersEXT);
  LV_LOADGLEXT(PFNGLGENFRAMEBUFFERSEXTPROC,                     glGenFramebuffersEXT);
  LV_LOADGLEXT(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC,              glCheckFramebufferStatusEXT);
  LV_LOADGLEXT(PFNGLFRAMEBUFFERTEXTURE1DEXTPROC,                glFramebufferTexture1DEXT);
  LV_LOADGLEXT(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,                glFramebufferTexture2DEXT);
  LV_LOADGLEXT(PFNGLFRAMEBUFFERTEXTURE3DEXTPROC,                glFramebufferTexture3DEXT);
  LV_LOADGLEXT(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC,             glFramebufferRenderbufferEXT);
  LV_LOADGLEXT(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC, glGetFramebufferAttachmentParameterivEXT);
  LV_LOADGLEXT(PFNGLGENERATEMIPMAPEXTPROC,                      glGenerateMipmapEXT);
  
 
  return error;
}
