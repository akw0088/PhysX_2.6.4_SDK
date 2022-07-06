/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Texture loader definition.
*/

#ifndef LV__TEXTURE_H
#define LV__TEXTURE_H

#include <Lava/LvLava.h>
#include <Lava/Renderer/LvTexture.h>
#include "Renderer/MaterialManager/TextureManager.h"
#include "Renderer/RendererImp.h"

typedef enum TextureColorModesE
{
  LV_RGB      = 0,
  LV_RGBA,
  LV_GREYSCALE,
};

class TextureBase : public LvrTexture
{
  public:
    virtual LvGUID GetGUID(void)=0;
};

/*!
**  Notes:    Texture Loader definition.
**/
class Texture : public TextureBase
{
  friend class TextureManager;
  public:
    
  private:
    /*!
    **  In:       pcPath    - Path to texture.
    **            desc      - texture descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    Load texture from file.
    **/
    virtual LvError LoadFromFile(const LvChar *pcPath, const LvrTextureDesc &desc) = 0;

  public:
    /*!
    **  Notes:    Free texture from memory.
    **/
    virtual void Free(void);
    
    /*!
    **  Return:   Internal Renderer GUID.
    **
    **  Notes:    Get the GUID for the texture's internal renderer.
    **/
    LvGUID GetGUID(void) { return m_gGUID; }
    
  protected:
    Texture(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer);
    virtual ~Texture(void);
  
  protected:
    LvGUID           m_gGUID;

    LvAllocator     &m_allocator;
    LvUserIO        &m_userio;
    RendererImp     &m_renderer;
    
    Texture         *m_pL,
                    *m_pR;
    LvChar          *m_pcName;
    LvSInt32         m_iNumUsers;
    LvBool           m_bAutoDestroy;
};

#endif
