/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Handles a mass of Shaders.
*/

#ifndef LV__SHADERMANAGER_H
#define LV__SHADERMANAGER_H

#include <Lava/LvLava.h>

#include "Shader.h"

// Shader GUID.
typedef LvGUID LvsGUID;

/*!
**  Notes:    Handles a mass of Shaders.
**/
class ShaderManager
{
  public:
    ShaderManager(LvAllocator &allocator, RendererImp &renderer);
    ~ShaderManager(void);
    
  public:
    /*!
    **  In:       pcPath  - Path to shader.
    **            profile - profile to use when compiling/assembling.
    **
    **  Return:   Shader Instance.
    **
    **  Notes:    Get an instance of a shader located in a file.
    **/
    Shader *GetShaderFromFile(const LvChar *pcPath, LvrShaderProfileE profile);
    
    /*!
    **  In:       pShader - Shader instance.
    **
    **  Notes:    Returns a Shader instance after finished being used.
    **/
    void ReturnShader(Shader *pShader);
    
  private:
    /*!
    **  In:       pcName  - Name of Shader.
    **
    **  Return:   Pointer to Shader.
    **
    **  Notes:    Find a Shader by name.
    **/
    Shader *_FindShader(const LvChar *pcName);
    
  public:
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Initialize the Shader manager.
    **/
    LvError Init(void);
    
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Shutdown the Shader Manager.
    **/
    LvError Shutdown(void);
    
  private:
    LvAllocator    &m_allocator;
    RendererImp    &m_renderer;
    Shader         *m_pHeadShader;
};

#endif
