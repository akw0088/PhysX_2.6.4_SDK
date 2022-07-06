/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    High Level Shader Language implementation. (Direct3D)
*/

#include "HLSLShader.h"

// Don't Compile D3D Support if we are on the wrong platform.
#if defined(LV_WINDOWS) && HLSL_OK

/*!
**  Notes:    Constructor.
**/
HLSLShader::HLSLShader(void)
{
  m_iNumUsers = 0;
  m_pcName = 0;
}

/*!
**  In:       pcPath  - Path to Shader.
**
**  Return:   Error code.
**
**  Notes:    Load Shader from File.
**/
LvError HLSLShader::LoadFromFile(const LvChar *pcPath, LvUInt16 uShaderType)
{
  m_pcName = LV_NEW LvChar[strlen(pcPath)+1];
  return LV_IGNORED;
}

/*!
**  Notes:    Free Shader contents from memory.
**            Does not actually delete the class.
**/
void HLSLShader::Free(void)
{
  if(m_pcName)
  {
    LV_DELETE [] m_pcName;
    m_pcName = 0;
  }
}

/*!
**  Return:   Error code.
**
**  Notes:    Starts the use of this shader.
**/
LvError HLSLShader::Begin(void)
{
  return LV_IGNORED;
}

/*!
**  Notes:    End the use of this shader.
**/
void HLSLShader::End(void)
{

}

#endif
