/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/


#include "OpenGLExtShader.h"
#ifdef LV_APPLE
#include "aglGetProcAddress.h"
#endif

PFNGLDELETEOBJECTARBPROC                    glDeleteObjectARB           = 0;
PFNGLGETHANDLEARBPROC                       glGetHandleARB              = 0;
PFNGLDETACHOBJECTARBPROC                    glDetachObjectARB           = 0;
PFNGLCREATESHADEROBJECTARBPROC              glCreateShaderObjectARB     = 0;
PFNGLSHADERSOURCEARBPROC                    glShaderSourceARB           = 0;
PFNGLCOMPILESHADERARBPROC                   glCompileShaderARB          = 0;
PFNGLCREATEPROGRAMOBJECTARBPROC             glCreateProgramObjectARB    = 0;
PFNGLATTACHOBJECTARBPROC                    glAttachObjectARB           = 0;
PFNGLLINKPROGRAMARBPROC                     glLinkProgramARB            = 0;
PFNGLUSEPROGRAMOBJECTARBPROC                glUseProgramObjectARB       = 0;
PFNGLVALIDATEPROGRAMARBPROC                 glValidateProgramARB        = 0;

PFNGLUNIFORM1FARBPROC                       glUniform1fARB              = 0;
PFNGLUNIFORM2FARBPROC                       glUniform2fARB              = 0;
PFNGLUNIFORM3FARBPROC                       glUniform3fARB              = 0;
PFNGLUNIFORM4FARBPROC                       glUniform4fARB              = 0;
PFNGLUNIFORM1IARBPROC                       glUniform1iARB              = 0;
PFNGLUNIFORM2IARBPROC                       glUniform2iARB              = 0;
PFNGLUNIFORM3IARBPROC                       glUniform3iARB              = 0;
PFNGLUNIFORM4IARBPROC                       glUniform4iARB              = 0;
PFNGLUNIFORM1FVARBPROC                      glUniform1fvARB             = 0;
PFNGLUNIFORM2FVARBPROC                      glUniform2fvARB             = 0;
PFNGLUNIFORM3FVARBPROC                      glUniform3fvARB             = 0;
PFNGLUNIFORM4FVARBPROC                      glUniform4fvARB             = 0;
PFNGLUNIFORM1IVARBPROC                      glUniform1ivARB             = 0;
PFNGLUNIFORM2IVARBPROC                      glUniform2ivARB             = 0;
PFNGLUNIFORM3IVARBPROC                      glUniform3ivARB             = 0;
PFNGLUNIFORM4IVARBPROC                      glUniform4ivARB             = 0;
PFNGLUNIFORMMATRIX2FVARBPROC                glUniformMatrix2fvARB       = 0;
PFNGLUNIFORMMATRIX3FVARBPROC                glUniformMatrix3fvARB       = 0;
PFNGLUNIFORMMATRIX4FVARBPROC                glUniformMatrix4fvARB       = 0;

PFNGLGETOBJECTPARAMETERFVARBPROC            glGetObjectParameterfvARB   = 0;
PFNGLGETOBJECTPARAMETERIVARBPROC            glGetObjectParameterivARB   = 0;
PFNGLGETINFOLOGARBPROC                      glGetInfoLogARB             = 0;
PFNGLGETATTACHEDOBJECTSARBPROC              glGetAttachedObjectsARB     = 0;
PFNGLGETUNIFORMLOCATIONARBPROC              glGetUniformLocationARB     = 0;
PFNGLGETACTIVEUNIFORMARBPROC                glGetActiveUniformARB       = 0;
PFNGLGETUNIFORMFVARBPROC                    glGetUniformfvARB           = 0;
PFNGLGETUNIFORMIVARBPROC                    glGetUniformivARB           = 0;
PFNGLGETSHADERSOURCEARBPROC                 glGetShaderSourceARB        = 0;

PFNGLBINDATTRIBLOCATIONARBPROC              glBindAttribLocationARB     = 0;
PFNGLGETACTIVEATTRIBARBPROC                 glGetActiveAttribARB        = 0;
PFNGLGETATTRIBLOCATIONARBPROC               glGetAttribLocationARB      = 0;


#ifdef LV_WINDOWS
#define LV_LOADGLEXT(_Type, _Name) (((_Name = (_Type) wglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error)
#endif

#ifdef LV_APPLE
#define LV_LOADGLEXT(_Type, _Name) (((_Name = (_Type) aglGetProcAddress(#_Name)) == 0) ? LV_BAD_OUTPUT : error)
#endif
/*!
**  Notes:    Load the extensions.
**/
LvError glextLoadShaderObject(void)
{
  LvError error = LV_OKAY;
  
  error = LV_LOADGLEXT(PFNGLDELETEOBJECTARBPROC        , glDeleteObjectARB);
  error = LV_LOADGLEXT(PFNGLGETHANDLEARBPROC           , glGetHandleARB);
  error = LV_LOADGLEXT(PFNGLDETACHOBJECTARBPROC        , glDetachObjectARB);
  error = LV_LOADGLEXT(PFNGLCREATESHADEROBJECTARBPROC  , glCreateShaderObjectARB);
  error = LV_LOADGLEXT(PFNGLSHADERSOURCEARBPROC        , glShaderSourceARB);
  error = LV_LOADGLEXT(PFNGLCOMPILESHADERARBPROC       , glCompileShaderARB);
  error = LV_LOADGLEXT(PFNGLCREATEPROGRAMOBJECTARBPROC , glCreateProgramObjectARB);
  error = LV_LOADGLEXT(PFNGLATTACHOBJECTARBPROC        , glAttachObjectARB);
  error = LV_LOADGLEXT(PFNGLLINKPROGRAMARBPROC         , glLinkProgramARB);
  error = LV_LOADGLEXT(PFNGLUSEPROGRAMOBJECTARBPROC    , glUseProgramObjectARB);
  error = LV_LOADGLEXT(PFNGLVALIDATEPROGRAMARBPROC     , glValidateProgramARB);
  error = LV_LOADGLEXT(PFNGLGETOBJECTPARAMETERFVARBPROC, glGetObjectParameterfvARB);
  error = LV_LOADGLEXT(PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameterivARB);
  error = LV_LOADGLEXT(PFNGLGETINFOLOGARBPROC          , glGetInfoLogARB);
  error = LV_LOADGLEXT(PFNGLGETATTACHEDOBJECTSARBPROC  , glGetAttachedObjectsARB);
  error = LV_LOADGLEXT(PFNGLGETUNIFORMLOCATIONARBPROC  , glGetUniformLocationARB);
  error = LV_LOADGLEXT(PFNGLGETACTIVEUNIFORMARBPROC    , glGetActiveUniformARB);
  error = LV_LOADGLEXT(PFNGLGETUNIFORMFVARBPROC        , glGetUniformfvARB);
  error = LV_LOADGLEXT(PFNGLGETUNIFORMIVARBPROC        , glGetUniformivARB);
  error = LV_LOADGLEXT(PFNGLGETSHADERSOURCEARBPROC     , glGetShaderSourceARB);
                                                      
  error = LV_LOADGLEXT(PFNGLUNIFORM1FARBPROC           , glUniform1fARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM2FARBPROC           , glUniform2fARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM3FARBPROC           , glUniform3fARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM4FARBPROC           , glUniform4fARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM1IARBPROC           , glUniform1iARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM2IARBPROC           , glUniform2iARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM3IARBPROC           , glUniform3iARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM4IARBPROC           , glUniform4iARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM1FVARBPROC          , glUniform1fvARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM2FVARBPROC          , glUniform2fvARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM3FVARBPROC          , glUniform3fvARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM4FVARBPROC          , glUniform4fvARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM1IVARBPROC          , glUniform1ivARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM2IVARBPROC          , glUniform2ivARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM3IVARBPROC          , glUniform3ivARB);
  error = LV_LOADGLEXT(PFNGLUNIFORM4IVARBPROC          , glUniform4ivARB);
  error = LV_LOADGLEXT(PFNGLUNIFORMMATRIX2FVARBPROC    , glUniformMatrix2fvARB);
  error = LV_LOADGLEXT(PFNGLUNIFORMMATRIX3FVARBPROC    , glUniformMatrix3fvARB);
  error = LV_LOADGLEXT(PFNGLUNIFORMMATRIX4FVARBPROC    , glUniformMatrix4fvARB);
  
  error = LV_LOADGLEXT(PFNGLBINDATTRIBLOCATIONARBPROC  , glBindAttribLocationARB);
  error = LV_LOADGLEXT(PFNGLGETACTIVEATTRIBARBPROC     , glGetActiveAttribARB);
  error = LV_LOADGLEXT(PFNGLGETATTRIBLOCATIONARBPROC   , glGetAttribLocationARB);
  
  return error;
}
