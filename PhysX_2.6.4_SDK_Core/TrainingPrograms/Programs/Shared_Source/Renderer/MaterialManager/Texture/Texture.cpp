/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Texture loader definition.
*/

#include "Renderer/RendererImp.h"
#include "Renderer/MaterialManager/TextureManager.h"

#include "Texture.h"

/*!
**  Notes:    Free texture from memory.
**/
void Texture::Free(void)
{
  if(m_gGUID)
  {
    m_renderer.DeleteTexture(m_gGUID);
    m_gGUID = 0;
  }
  if(m_pcName)
  {
    if(m_pcName) LV_FREE(m_allocator, m_pcName);
    m_pcName = 0;
  }
}

Texture::Texture(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer) :
m_allocator(allocator),
m_userio(userio),
m_renderer(renderer)
{
  m_gGUID        = 0;
  m_pL           = 0;
  m_pR           = 0;
  m_pcName       = 0;
  m_iNumUsers    = 0;
  m_bAutoDestroy = LV_TRUE;
}

Texture::~Texture(void)
{
  Free();
}


