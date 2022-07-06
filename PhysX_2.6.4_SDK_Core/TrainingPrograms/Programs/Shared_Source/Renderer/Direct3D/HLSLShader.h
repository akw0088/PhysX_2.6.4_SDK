/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    High Level Shader Language implementation. (Direct3D)
*/

#ifndef LV__HLSLSHADER_H
#define LV__HLSLSHADER_H

#include "Renderer/MaterialManager/Shader.h"

// Don't Compile D3D Support if we are on the wrong platform.
#if defined(LV_WINDOWS) && HLSL_OK

/*!
**  Notes:    High Level Shader Language implementation. (Direct3D)
**/
class HLSLShader : public Shader
{
  public:
    /*!
    **  Notes:    Constructor.
    **/
    HLSLShader(void);
    
  public:
    /*!
    **  In:       pcPath  - Path to Shader.
    **
    **  Return:   Error code.
    **
    **  Notes:    Load Shader from File.
    **/
    LvError LoadFromFile(const LvChar *pcPath, LvUInt16 uShaderType);
    
    /*!
    **  Notes:    Free Shader contents from memory.
    **            Does not actually delete the class.
    **/
    void Free(void);
    
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Starts the use of this shader.
    **/
    LvError Begin(void);
    
    /*!
    **  Notes:    End the use of this shader.
    **/
    void End(void);
    
  private:
    
    
};

#endif

#endif
