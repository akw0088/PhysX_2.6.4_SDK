/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    JPEG Texture Loader.
*/

#ifndef LV__JPGTEXTURE_H
#define LV__JPGTEXTURE_H

#include "Texture.h"

/*!
**  Notes:    JPEG Texture Loader.
**/
class JPGTexture : public Texture
{
  private:
    /*!
    **  In:       pcPath    - Path to texture.
    **            desc      - texture descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    Load texture from file.
    **/
    virtual LvError LoadFromFile(const LvChar *pcPath, const LvrTextureDesc &desc);
  
  public:
    JPGTexture(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer) : Texture(allocator,userio,renderer) {}
};

#endif
