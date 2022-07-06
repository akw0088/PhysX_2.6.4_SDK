/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    PNG Texture Loader.
*/

#ifndef LV__PNGTEXTURE_H
#define LV__PNGTEXTURE_H

#include "Texture.h"

/*!
**  Notes:    PNG Texture Loader.
**/
class PNGTexture : public Texture
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
    PNGTexture(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer) : Texture(allocator,userio,renderer) {}
};

#endif
