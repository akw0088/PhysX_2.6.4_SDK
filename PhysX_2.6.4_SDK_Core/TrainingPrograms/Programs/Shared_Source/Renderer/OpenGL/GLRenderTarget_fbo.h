/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Render Target implemented using Frame Buffer Objects.
*/

#ifndef GLRENDERTARGET_FBO_H
#define GLRENDERTARGET_FBO_H

#include "../RendererImp.h"
#include "GLRenderTarget.h"

/*!
**  Notes:    OpenGL Implementation of a Rendering Target.
**/
class OpenGLRenderTargetFBO : public OpenGLRenderTarget
{
  public:
    /*!
    **  In:       buffers - what buffers to clear.
    **
    **  Notes:    Clear the render target's buffer.
    **            See LvrClearBitsE.
    **/
    void Clear(LvUInt32 buffers);
    
    /*!
    **  Notes:    Gets a renderable texture.
    **/
    virtual LvrTexture *GetTexture(void);
    
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
    void Free(void);
    
  public:
    /*!
    **  In:       desc  - Render Target Descriptor.
    **
    **  Notes:    Create the Render Target from Descriptor.
    **/
    LvError LoadFromDesc(const LvrRenderTargetDesc &desc);
    
    /*!
    **  Notes:    Signals OpenGL to write to this target.
    **/
    void Begin(void);
    
    /*!
    **  Notes:    Signals OpenGL to stop writing to this target.
    **/
    void End(void);
    
  public:
    OpenGLRenderTargetFBO(LvAllocator &allocator, LvRenderer &renderer);
    ~OpenGLRenderTargetFBO(void);
    
  private:
    LvAllocator  &m_allocator;
    LvRenderer   &m_renderer;
    
    GLuint        m_framebuffer;
    GLuint        m_depthbuffer;
    
    LvUInt32      m_targetWidth;
    LvUInt32      m_targetHeight;
};

#endif
