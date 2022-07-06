/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Render Target implemented using PBuffers.
*/

#include "OpenGLRenderer.h"
#include "GLRenderTarget_pbuffer.h"

/*!
**  In:       buffers - what buffers to clear.
**
**  Notes:    Clear the render target's buffer.
**            See LvrClearBitsE.
**/
void OpenGLRenderTargetPBuffer::Clear(LvUInt32 buffers)
{
  GLbitfield glbuffers = 0;
  glbuffers |= buffers&LVR_CLEAR_COLOR   ? GL_COLOR_BUFFER_BIT   : 0;
  glbuffers |= buffers&LVR_CLEAR_DEPTH   ? GL_DEPTH_BUFFER_BIT   : 0;
  glbuffers |= buffers&LVR_CLEAR_STENCIL ? GL_STENCIL_BUFFER_BIT : 0;
  #if defined(LV_WINDOWS)
    if(m_hPbuffer && m_hDc && m_hGlrc)
    {
      wglMakeCurrent(m_hDc, m_hGlrc);
      LvBool depthWrite = m_renderer.GetState(LVR_DEPTH_WRITE);
      glDepthMask(1);
      glClear(glbuffers);
      if(!depthWrite) glDepthMask(0);
      wglMakeCurrent(m_hDcParent, m_hGlrcParent);
    }
  #endif
}

/*!
**  Notes:    Gets a renderable texture.
**/
LvrTexture *OpenGLRenderTargetPBuffer::GetTexture(void)
{
  return this;
}

/*!
**  Notes:    Free texture from memory.
**/
void OpenGLRenderTargetPBuffer::Free(void)
{
  #if defined(LV_WINDOWS)
    BOOL makeCurrentRet = wglMakeCurrent(m_hDcParent,m_hGlrcParent);
    DWORD error = 0;
    if(!makeCurrentRet)
    {
      error = GetLastError();
    }
    LV_ASSERT(makeCurrentRet);
    if(makeCurrentRet)
    {
      if(m_texture)
      {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
      }
    
      if(m_hGlrc)
      {
        wglDeleteContext(m_hGlrc);
        m_hGlrc = 0;
      }
      
      if(m_hDc)
      {
        wglReleasePbufferDCARB(m_hPbuffer, m_hDc);
        m_hDc = 0;
      }
      
      if(m_hPbuffer)
      {
        wglDestroyPbufferARB(m_hPbuffer);
        m_hPbuffer = 0;
      }
    }
  #endif
}

/*!
**  In:       desc  - Render Target Descriptor.
**
**  Notes:    Create the Render Target from Descriptor.
**/
LvError OpenGLRenderTargetPBuffer::LoadFromDesc(const LvrRenderTargetDesc &desc)
{
  LvError error = LV_IGNORED;
  
  if(desc.targetWidth > desc.bufferWidth || desc.targetHeight > desc.bufferHeight)
    return LV_BAD_INPUT;
  
  m_targetWidth   = desc.targetWidth;
  m_targetHeight  = desc.targetHeight;
  
  #if defined(LV_WINDOWS)
    m_hDcParent   = wglGetCurrentDC();
    m_hGlrcParent = wglGetCurrentContext();
    
    BOOL makeCurrentRet = wglMakeCurrent(m_hDcParent,m_hGlrcParent);
    LV_ASSERT(makeCurrentRet);
  
    m_hPbuffer  = 0;
    m_hDc       = 0;
    m_hGlrc     = 0;
  
    int viPixelAttributes[] =
    {
      WGL_DRAW_TO_PBUFFER_ARB,        GL_TRUE,
      WGL_BIND_TO_TEXTURE_RGBA_ARB,   GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB,         GL_TRUE,
      WGL_ACCELERATION_ARB,           WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB,             24,
      WGL_ALPHA_BITS_ARB,             8,
      WGL_STENCIL_BITS_ARB,           8,
      0,                              0,
    };
    
    int viPbufferAttributes[] =
    {
      WGL_TEXTURE_FORMAT_ARB,         WGL_TEXTURE_RGBA_ARB,
      WGL_TEXTURE_TARGET_ARB,         WGL_TEXTURE_2D_ARB,
      WGL_MIPMAP_TEXTURE_ARB,         FALSE,
      WGL_PBUFFER_LARGEST_ARB,        FALSE,
      0,                              0,
    };
    
    
    LvReal vfAttributes[256] = {0};
    
    UINT iNumFormats = 0;
    int viFormats[512] = {0};
    BOOL blah = wglChoosePixelFormatARB(m_hDcParent, viPixelAttributes, vfAttributes, 512, viFormats, &iNumFormats);
    
    if(iNumFormats > 0)
    {
      m_hPbuffer = wglCreatePbufferARB( m_hDcParent, viFormats[0],
                                        desc.bufferWidth, desc.bufferHeight,
                                        viPbufferAttributes);
      
      if(m_hPbuffer)
      {
        m_hDc = wglGetPbufferDCARB(m_hPbuffer);
        if(m_hDc)
        {
          m_hGlrc = wglCreateContext(m_hDc);
          wglShareLists(m_hGlrcParent, m_hGlrc);
          
          // Setup the texture.
          wglMakeCurrent(m_hDc, m_hGlrc);
            glViewport(0, 0, m_targetWidth, m_targetHeight);
            glGenTextures(1, &m_texture);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          wglMakeCurrent(m_hDcParent, m_hGlrcParent);
          
          error = LV_OKAY;
        }
      }
    }
  #endif
  if(error != LV_OKAY)
  {
    Free();
  }
  return error;
}

/*!
**  Notes:    Signals OpenGL to write to this target.
**/
void OpenGLRenderTargetPBuffer::Begin(void)
{
  #if defined(LV_WINDOWS)
    if(m_hPbuffer && m_hDc && m_hGlrc)
    {
      // Are these needed? They slow me down a lot!
      // Documentation suggest this needs to be called
      // in order for the p-buffer to have write access
      // to the texture data again. But it seems to still
      // work without these calls.
      glBindTexture(GL_TEXTURE_2D, m_texture);
      wglReleaseTexImageARB(m_hPbuffer, WGL_FRONT_LEFT_ARB);
      
      wglMakeCurrent(m_hDc, m_hGlrc);
    }
  #endif
}

/*!
**  Notes:    Signals OpenGL to stop writing to this target.
**/
void OpenGLRenderTargetPBuffer::End(void)
{
  #if defined(LV_WINDOWS)
    if(m_hPbuffer && m_hDc && m_hGlrc)
    {
      glBindTexture(GL_TEXTURE_2D, m_texture);
      wglBindTexImageARB(m_hPbuffer, WGL_FRONT_LEFT_ARB);
      wglMakeCurrent(m_hDcParent, m_hGlrcParent);
    }
  #endif
}

OpenGLRenderTargetPBuffer::OpenGLRenderTargetPBuffer(LvAllocator &allocator, LvRenderer &renderer) :
m_renderer(renderer),
OpenGLRenderTarget(allocator)
{
#if defined(LV_WINDOWS)
  m_hPbuffer = 0;
  m_hDc = 0;
  m_hGlrc = 0;
  m_hDcParent = 0;
  m_hGlrcParent = 0;
#endif
  m_texture = 0;
}

OpenGLRenderTargetPBuffer::~OpenGLRenderTargetPBuffer(void)
{

}













