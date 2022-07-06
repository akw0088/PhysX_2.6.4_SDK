/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Handles shanders.
*/

#include "ShaderManager.h"
#include "Renderer/RendererImp.h"

ShaderManager::ShaderManager(LvAllocator &allocator, RendererImp &renderer) : m_allocator(allocator), m_renderer(renderer)
{
  m_pHeadShader = 0;
}

ShaderManager::~ShaderManager(void)
{

}

/*!
**  In:       pcPath  - Path to shader.
**            profile - profile to use when compiling/assembling.
**
**  Return:   Shader Instance.
**
**  Notes:    Get an instance of a shader located in a file.
**/
Shader *ShaderManager::GetShaderFromFile(const LvChar *pcPath, LvrShaderProfileE profile)
{
  Shader* pShader = _FindShader(pcPath);
  
  if(!pShader)
  {
    // Shader needs to be created.
    pShader = m_renderer.CreateShader(pcPath, profile);
    //LV_ASSERT(pShader);
    if(pShader)
    {
      // Insert the Shader into the list.
      if(m_pHeadShader)
        m_pHeadShader->m_pPrev = pShader;
      pShader->m_pNext  = m_pHeadShader;
      pShader->m_pPrev  = 0;
      m_pHeadShader     = pShader;
    }
  }
  
  if(pShader)
  {
    pShader->m_iNumUsers++;
  }
  
  return pShader;
}

/*!
**  In:       pShader - Shader instance.
**
**  Notes:    Returns a Shader instance after finished being used.
**/
void ShaderManager::ReturnShader(Shader *pShader)
{
  LV_ASSERT(pShader);
  pShader->m_iNumUsers--;
  if(pShader->m_iNumUsers <= 0)
  {
    // Remove from List.
    if(pShader->m_pPrev)
      pShader->m_pPrev->m_pNext = pShader->m_pNext;
    else
      m_pHeadShader = pShader->m_pNext;
    if(pShader->m_pNext)
      pShader->m_pNext->m_pPrev = pShader->m_pPrev;

    // Free shader.
    m_renderer.ReleaseShader(pShader);
  }
}

/*!
**  In:       pcName  - Name of Shader.
**
**  Return:   Pointer to Shader.
**
**  Notes:    Find a Shader by name.
**/
Shader *ShaderManager::_FindShader(const LvChar *pcName)
{
  for(Shader *pCurr = m_pHeadShader; pCurr; pCurr = pCurr->m_pNext)
  {
    if(!stricmp(pCurr->m_pcName, pcName))
    {
      return pCurr;
    }
  }
  return 0;
}

/*!
**  Return:   Error code.
**
**  Notes:    Initialize the Shader manager.
**/
LvError ShaderManager::Init(void)
{
  m_pHeadShader = 0;
  return LV_OKAY;
}

/*!
**  Return:   Error code.
**
**  Notes:    Shutdown the Shader Manager.
**/
LvError ShaderManager::Shutdown(void)
{
  LV_ASSERT(!m_pHeadShader);
  return LV_OKAY;
}
