/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Targa Texture Loader.
*/

#ifndef LV__TGATEXTURE_H
#define LV__TGATEXTURE_H

#include "Texture.h"

/*!
**  Notes:    Targa Texture Loader.
**/
class TGATexture : public Texture
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
    TGATexture(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer) : Texture(allocator,userio,renderer) {}
};

#endif
