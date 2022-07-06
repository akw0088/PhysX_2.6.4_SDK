/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Texture Manager.
*/

// DEV NOTES
//
// -  Things are getting a little hacky in here due to
//    the new need to add Render Targets to the texture
//    manager. We need to revisit how all this works. -jd

#ifndef LV__TEXTUREMANAGER_H
#define LV__TEXTUREMANAGER_H

#include <Lava/LvLava.h>

#include "Texture/Texture.h"

/*!
**  Notes:    Texture Manager.
**/
class TextureManager
{
  public:
    TextureManager(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer);
    ~TextureManager(void);
    
  public:
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Initialize the Texture Manager.
    **/
    LvError Init(void);
    
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Shutdown the Texture Manager.
    **/
    LvError Shutdown(void);
    
    /*!
    **  In:       pcPath  - Name or Path of texture.
    **            desc    - texture descriptor.
    **
    **  Return:   Texture instance.
    **
    **  Notes:    Get an instance of a texture.
    **/
    Texture *GetTexture(const LvChar *pcPath, const LvrTextureDesc &desc);
    
    /*!
    **  In:       files - paths to texture files.
    **            desc  - texture descriptor.
    **
    **  Return:   Instance of a texture, or 0 for error.
    **
    **  Notes:    Gets an instance of a texture and loads from
    **            file if needed.
    **/
    Texture *GetCubeMap(const LvrCubeMapFiles &files, const LvrTextureDesc &desc);
    
    /*!
    **  In:       pNode - The texture we are returning.
    **
    **  Notes:    Return a texture after finished being used.
    **/
    void ReturnTexture(Texture *pNode);
  
  private:
    /*!
    **  In:       pcName  - Name or Path of texture.
    **
    **  Return:   Pointer to Texture handler or 0.
    **
    **  Notes:    Find a texture by name or path.
    **/
    Texture *_FindTextureByName(const LvChar *pcName);
    
    /*!
    **  In:       pcPath  - Path to texture.
    **            desc    - texture descriptor.
    **
    **  Return:   Pointer to Texture handler or 0.
    **
    **  Notes:    Load a texture from File.
    **/
    Texture *_LoadTextureFromFile(const LvChar *pcPath, const LvrTextureDesc &desc);
    
    /*!
    **  In:       pTexture  - Texture pointer.
    **
    **  Notes:    Free a texture.
    **/
    void _FreeTexture(Texture *pNode);
    
    /*!
    **  In:       pTexture  - Texture pointer.
    **
    **  Notes:    Insert a texture into our linked list.
    **/
    void _InsertTextureIntoList(Texture *pNode);
    
    /*!
    **  In:       pTexture  - Texture pointer.
    **
    **  Notes:    Remove a texture from out linked list.
    **/
    void _RemoveTextureFromList(Texture *pNode);
    
  
  private:
    LvAllocator    &m_allocator;
    LvUserIO       &m_userio;
    RendererImp    &m_renderer;
    Texture        *m_pHeadTexture;
    
};

#endif
