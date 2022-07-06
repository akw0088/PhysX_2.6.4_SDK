/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Render Target implemented using PBuffers.
*/

#ifndef GLRENDERTARGET_PBUFFER_H
#define GLRENDERTARGET_PBUFFER_H

#include "../RendererImp.h"
#include "GLRenderTarget.h"

/*!
**  Notes:    OpenGL Implementation of a Rendering Target.
**/
class OpenGLRenderTargetPBuffer : public OpenGLRenderTarget
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
    OpenGLRenderTargetPBuffer(LvAllocator &allocator, LvRenderer &renderer);
    ~OpenGLRenderTargetPBuffer(void);
    
  private:
    LvRenderer   &m_renderer;
    
    #if defined(LV_WINDOWS)
      HPBUFFERARB m_hPbuffer;
      HDC         m_hDc;
      HGLRC       m_hGlrc;
      HDC         m_hDcParent;
      HGLRC       m_hGlrcParent;
    #endif
    
    LvUInt32      m_targetWidth;
    LvUInt32      m_targetHeight;
};

#endif
