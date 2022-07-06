/*!
**  Author:   James Dolan (jdolan@gmail.com)
**            Taylor White (taylorcwhite@gmail.com)
**
**  Notes:    Cg Language implementation.
*/

#include "CGGLShader.h"

#include <stdio.h>

#if CGGL_OK

CGGLShader::CGGLShader(LvAllocator &allocator, LvUserIO &userio, CGcontext context) : Shader(allocator, userio)
{
  m_context         = context;
  m_profile         = CG_PROFILE_UNKNOWN;
  m_program         = 0;
}

CGGLShader::~CGGLShader(void)
{
  if(m_program && cgIsProgramCompiled(m_program))
    cgDestroyProgram(m_program);
}

/*!
**  In:       pcPath  - Path to shader.
**            profile - Profile to use.
**
**  Return:   Error code.
**
**  Notes:    Load Cg Shader from File.
**/
LvError CGGLShader::LoadFromFile(const LvChar *pcPath, CGprofile profile)
{
  LvError   ret     = LV_IGNORED;
  CGprogram program = 0;
  
  if(cgGLIsProfileSupported(profile))
  {
  #if 0 // Old Skool.
    program = cgCreateProgramFromFile(m_context, CG_SOURCE, pcPath, profile, "main", NULL);
  #else
    LvFile *file = m_userio.LV_FileOpen(pcPath, LV_FALSE);
    if(file)
    {
      LvUInt64 len = file->GetSize();
      LvChar *buffer = (LvChar*)LV_MALLOC(m_allocator, len+1);
      if(buffer)
      {
        len = file->ReadBuffer(buffer, 1, len);
        buffer[len] = 0;
      #if defined(LV_PLAYSTATION3)
        program = cgCreateProgram(m_context, CG_BINARY, buffer, profile, "main", NULL);
      #else
        program = cgCreateProgram(m_context, CG_SOURCE, buffer, profile, "main", NULL);
      #endif
        LV_FREE(m_allocator, buffer);
      }
      m_userio.LV_FileClose(file);
    }
  #endif
    if(!cgIsProgramCompiled(program))
      cgCompileProgram(program);
    if(cgIsProgramCompiled(program))
    {
      cgGLEnableProfile(profile);
      cgGLLoadProgram(program);

      ret = LV_OKAY;
    }
  }
  
  if(ret == LV_OKAY)
  {
    m_pcName = (LvChar*)LV_MALLOC(m_allocator, strlen(pcPath)+1);
    strcpy(m_pcName, pcPath);
    m_iNumUsers = 0;
    m_profile = profile;
    m_program = program;
    cgGLUnbindProgram(m_profile);
    cgGLDisableProfile(m_profile);
  }
  
  return ret;
}

/*!
**  Notes:    Starts the use of this shader.
**/
void CGGLShader::Begin(void)
{
  LV_ASSERT(cgIsProgramCompiled(m_program));

  cgGLEnableProfile(m_profile);
  cgGLBindProgram(m_program);
}

/*!
**  Notes:    End the use of this shader.
**/
void CGGLShader::End(void)
{
  cgGLUnbindProgram(m_profile);
  cgGLDisableProfile(m_profile);
}

LvGUID CGGLShader::GetVariable(const LvChar *pcName)
{
  LvGUID ret = 0;
  ret = (LvGUID)cgGetNamedParameter(m_program, pcName);
  return ret;
}

LvError CGGLShader::SetReal(LvGUID gVar, LvReal fReal)
{
  LvError ret = LV_BAD_INPUT;
  CGparameter param = (CGparameter)gVar;
  if(param)
  {
    cgGLSetParameter1f(param, fReal);
    ret = LV_OKAY;
  }
  return ret;
}

LvError CGGLShader::SetVector2(LvGUID gVar, const LvVector2 &vec)
{
  LvError ret = LV_BAD_INPUT;
  CGparameter param = (CGparameter)gVar;
  if(param)
  {
    cgGLSetParameter2fv(param, &vec.x);
    ret = LV_OKAY;
  }
  return ret;
}

LvError CGGLShader::SetVector3(LvGUID gVar, const LvVector3 &vec)
{
  LvError ret = LV_BAD_INPUT;
  CGparameter param = (CGparameter)gVar;
  if(param)
  {
    cgGLSetParameter3fv(param, &vec.x);
    ret = LV_OKAY;
  }
  return ret;
}

LvError CGGLShader::SetVector4(LvGUID gVar, const LvVector4 &vec)
{
  LvError ret = LV_BAD_INPUT;
  CGparameter param = (CGparameter)gVar;
  if(param)
  {
    cgGLSetParameter4fv(param, &vec.x);
    ret = LV_OKAY;
  }
  return ret;
}

LvError CGGLShader::SetStateMatrix(LvGUID gVar, const LvUInt8 state)
{
  LvError ret = LV_BAD_INPUT;
  CGparameter param = (CGparameter)gVar;
  if(param)
  {
    switch(state)
    {
	  case LVR_MODELVIEW_PROJECTION_MATRIX:
      {
        cgGLSetStateMatrixParameter(param, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
        break;
      }
	  case LVR_MODELVIEW_MATRIX:
      {
        cgGLSetStateMatrixParameter(param, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
        break;
      }
      case LVR_PROJECTION_MATRIX:
      {
        cgGLSetStateMatrixParameter(param, CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
        break;
      }
    }
    
    ret = LV_OKAY;
  }
  return ret;
}

#endif
