/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Texture Instance
*/

#ifndef LV_TEXTUREINSTANCE_H
#define LV_TEXTUREINSTANCE_H

#include <Lava/LvLava.h>
#include <Lava/Renderer/LvTexture.h>
#include "Texture.h"

typedef enum TextureInstanceType
{
  LVR_TEXTURE_2D = 0,         // 2D Texture.   
  LVR_TEXTURE_CUBEMAP,        // Cube-Map.
  LVR_TEXTURE_RENDERTARGET,   // Render Target.
};

//! Texture Instance Base Class.
class TextureInstance : public LvrTexture
{
  public:
    TextureInstance(TextureInstanceType type);
    ~TextureInstance(void);
    
    LV_INLINE TextureInstanceType GetType(void) { return m_type; }
    
  private:
    const TextureInstanceType m_type;
};

//! 2D Texture Instance.
class TextureInstance2D : public TextureInstance
{
  public:
    TextureInstance2D(void);
    ~TextureInstance2D(void);
    
  public:
    Texture *m_texture;
};

//! CubeMap texture instance.
class TextureInstanceCube : public TextureInstance
{
  public:
    TextureInstanceCube(void);
    ~TextureInstanceCube(void);
    
  public:
    Texture *m_texture;
};

#endif
