/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Texture Manager.
*/

#include "Renderer/RendererImp.h"
#include "TextureManager.h"

#include "Texture/PNGTexture.h"
#include "Texture/TGATexture.h"
#include "Texture/JPGTexture.h"


TextureManager::TextureManager(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer) : 
m_allocator(allocator), m_userio(userio), m_renderer(renderer)
{
  m_pHeadTexture    = 0;
}

TextureManager::~TextureManager(void)
{

}

/*!
**  Return:   Error code.
**
**  Notes:    Initialize the Texture Manager.
**/
LvError TextureManager::Init(void)
{
  LvError ret = LV_OKAY;
  m_pHeadTexture    = 0;
  return ret;
}

/*!
**  Return:   Error code.
**
**  Notes:    Shutdown the Texture Manager.
**/
LvError TextureManager::Shutdown(void)
{
  return LV_OKAY;
}

/*!
**  In:       pcPath  - Name or Path of texture.
**            desc    - texture descriptor.
**
**  Return:   Texture instance.
**
**  Notes:    Get an instance of a texture.
**/
Texture *TextureManager::GetTexture(const LvChar *pcPath, const LvrTextureDesc &desc)
{
  Texture *pNode = _FindTextureByName(pcPath);
  
  if(!pNode)
  {
    pNode = _LoadTextureFromFile(pcPath, desc);
  }
  
  if(pNode)
  {
    pNode->m_iNumUsers++;
  }
  
  return pNode;
}

/*!
**  In:       files - paths to texture files.
**            desc  - texture descriptor.
**
**  Return:   Instance of a texture, or 0 for error.
**
**  Notes:    Gets an instance of a texture and loads from
**            file if needed.
**/
Texture *TextureManager::GetCubeMap(const LvrCubeMapFiles &files, const LvrTextureDesc &desc)
{
  Texture *cubemap = 0;
  LV_ASSERT(!"TODO!");
  return cubemap;
}

/*!
**  In:       pNode - The texture we are returning.
**
**  Notes:    Return a texture after finished being used.
**/
void TextureManager::ReturnTexture(Texture *pNode)
{
  if(pNode)
  {
    pNode->m_iNumUsers--;
    if(pNode->m_iNumUsers <= 0 && pNode->m_bAutoDestroy)
    {
      _FreeTexture(pNode);
    }
  }
}

/*!
**  In:       pcName  - Name or Path of texture.
**
**  Return:   Pointer to Texture handler or 0.
**
**  Notes:    Find a texture by name or path.
**/
Texture *TextureManager::_FindTextureByName(const LvChar *pcName)
{
  Texture *pNode = m_pHeadTexture;
  
  while(pNode)
  {
    LvSInt32 iCmp = strcmp(pcName, pNode->m_pcName);
    if(!iCmp)
      return pNode;
    else if(iCmp < 0)
      pNode = pNode->m_pL;
    else
      pNode = pNode->m_pR;
  }
  
  return 0;
}

/*!
**  In:       pcPath  - Path to texture.
**            desc    - texture descriptor.
**
**  Return:   Pointer to Texture handler or 0.
**
**  Notes:    Load a texture from File.
**/
Texture *TextureManager::_LoadTextureFromFile(const LvChar *pcPath, const LvrTextureDesc &desc)
{
  LvSInt32       iLen     = (LvSInt32)strlen(pcPath);
  const LvChar  *pExt     = 0;
  Texture       *pTexture = 0;
  
  if(iLen < 4)
  {
    return 0;
  }
  
  pExt = &pcPath[iLen]-3;
  
  if(!stricmp(pExt, "PNG"))
  {
    pTexture = LV_NEW(m_allocator, PNGTexture) PNGTexture(m_allocator, m_userio, m_renderer);
  }
  else if(!stricmp(pExt, "TGA"))
  {
    pTexture = LV_NEW(m_allocator, TGATexture) TGATexture(m_allocator, m_userio, m_renderer);
  }
  else if(!stricmp(pExt, "JPG") || !stricmp(pExt, "JPEG"))
  {
    pTexture = LV_NEW(m_allocator, JPGTexture) JPGTexture(m_allocator, m_userio, m_renderer);
  }
  
  if(pTexture)
  {
    LvError error = pTexture->LoadFromFile(pcPath, desc);
    if(error)
    {
      LV_DELETE(m_allocator, Texture, pTexture);
      pTexture = 0;
    }
  }
  
  if(pTexture)
  {
    _InsertTextureIntoList(pTexture);
  }
  
  return pTexture;
}

/*!
**  In:       pTexture  - Texture pointer.
**
**  Notes:    Free a texture.
**/
void TextureManager::_FreeTexture(Texture *pNode)
{
  if(pNode)
  {
    _RemoveTextureFromList(pNode);
    pNode->Free();
    LV_DELETE(m_allocator, Texture, pNode);
  }
}

/*!
**  In:       pTexture  - Texture pointer.
**
**  Notes:    Insert a texture into our linked list.
**/
void TextureManager::_InsertTextureIntoList(Texture *pNode)
{
  LV_ASSERT(pNode);
  
  Texture *pCurr = m_pHeadTexture;
  
  if(!m_pHeadTexture)
    m_pHeadTexture = pNode;
  
  while(pCurr)
  {
    LvSInt32 iCmp = strcmp(pNode->m_pcName, pCurr->m_pcName);
    
    // We should never find a texture we are inserting!
    LV_ASSERT(iCmp);
    
    if(iCmp < 0)
    {
      if(!pCurr->m_pL)
      {
        pCurr->m_pL = pNode;
        break;
      }
      else
        pCurr = pCurr->m_pL;
    }
    else
    {
      if(!pCurr->m_pR)
      {
        pCurr->m_pR = pNode;
        break;
      }
      else
        pCurr = pCurr->m_pR;
    }
  }
}

/*!
**  In:       pTexture  - Texture pointer.
**
**  Notes:    Remove a texture from out linked list.
**/
void TextureManager::_RemoveTextureFromList(Texture *pNode)
{
  // Find our self in the tree.
  Texture **pCurr  = &m_pHeadTexture;
  
  while(*pCurr)
  {
    LvSInt32 iCmp = strcmp(pNode->m_pcName, (*pCurr)->m_pcName);
    
    if(!iCmp)
    {
      // If we find a texture with the same name, but its
      // not us, we have a major problem.
      LV_ASSERT(pNode == *pCurr);
      break;
    }
    else if(iCmp < 0)
      pCurr = &(*pCurr)->m_pL;
    else
      pCurr = &(*pCurr)->m_pR;
  }
  
  // If we did't find the node, something is wrong.
  LV_ASSERT(*pCurr);
  
  // Nuke the pointer to us.
  *pCurr = 0;
  
  // Insert our children back into the tree.
  if(pNode->m_pL)
    _InsertTextureIntoList(pNode->m_pL);
  if(pNode->m_pR)
    _InsertTextureIntoList(pNode->m_pR);
}
