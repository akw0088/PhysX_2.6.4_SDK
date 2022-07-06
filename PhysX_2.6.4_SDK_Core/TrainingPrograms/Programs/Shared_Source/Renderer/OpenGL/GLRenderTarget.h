/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Render Target implemented using PBuffers.
*/

#ifndef GLRENDERTARGET_H
#define GLRENDERTARGET_H

#include "../RendererImp.h"

/*!
**  Notes:    OpenGL Implementation of a Rendering Target.
**/
class OpenGLRenderTarget : public LvrRenderTarget, public TextureBase
{ 
  public:
    /*!
    **  Return:   Internal Renderer GUID.
    **
    **  Notes:    Get the GUID for the texture's internal renderer.
    **/
    LvGUID GetGUID(void) { return *((LvGUID*)&m_texture); }
    
    /*!
    **  Notes:    Free texture from memory.
    **/
    virtual void Free(void) = 0;

    /*!
    **  In:       desc  - Render Target Descriptor.
    **
    **  Notes:    Create the Render Target from Descriptor.
    **/
    virtual LvError LoadFromDesc(const LvrRenderTargetDesc &desc) = 0;
    
    /*!
    **  Notes:    Signals OpenGL to write to this target.
    **/
    virtual void Begin(void) = 0;
    
    /*!
    **  Notes:    Signals OpenGL to stop writing to this target.
    **/
    virtual void End(void) = 0;
    
  public:
    OpenGLRenderTarget(LvAllocator &allocator)
    {
      m_texture = 0;
    }
  protected:
    GLuint        m_texture;
};

#endif
