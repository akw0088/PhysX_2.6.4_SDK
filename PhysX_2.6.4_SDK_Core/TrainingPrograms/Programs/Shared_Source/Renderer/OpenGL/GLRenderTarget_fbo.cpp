/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Render Target implemented using PBuffers.
*/

#include "OpenGLRenderer.h"
#include "GLRenderTarget_fbo.h"

/*!
**  In:       buffers - what buffers to clear.
**
**  Notes:    Clear the render target's buffer.
**            See LvrClearBitsE.
**/
void OpenGLRenderTargetFBO::Clear(LvUInt32 buffers)
{
  GLbitfield glbuffers = 0;
  glbuffers |= buffers&LVR_CLEAR_COLOR   ? GL_COLOR_BUFFER_BIT   : 0;
  glbuffers |= buffers&LVR_CLEAR_DEPTH   ? GL_DEPTH_BUFFER_BIT   : 0;
  glbuffers |= buffers&LVR_CLEAR_STENCIL ? GL_STENCIL_BUFFER_BIT : 0;
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_framebuffer);
  glClear(glbuffers);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/*!
**  Notes:    Gets a renderable texture.
**/
LvrTexture *OpenGLRenderTargetFBO::GetTexture(void)
{
  return this;
}

/*!
**  Notes:    Free texture from memory.
**/
void OpenGLRenderTargetFBO::Free(void)
{
  if(m_depthbuffer)
  {
    glDeleteRenderbuffersEXT(1, &m_depthbuffer);
    m_depthbuffer = 0;
  }
  if(m_framebuffer)
  {
    glDeleteFramebuffersEXT(1, &m_framebuffer);
    m_framebuffer = 0;
  }
  if(m_texture)
  {
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
  }
}

/*!
**  In:       desc  - Render Target Descriptor.
**
**  Notes:    Create the Render Target from Descriptor.
**/
LvError OpenGLRenderTargetFBO::LoadFromDesc(const LvrRenderTargetDesc &desc)
{
  LvError error = LV_IGNORED;
  
  if(desc.targetWidth > desc.bufferWidth || desc.targetHeight > desc.bufferHeight)
    return LV_BAD_INPUT;
  
  m_targetWidth   = desc.targetWidth;
  m_targetHeight  = desc.targetHeight;
  
  Free();
  
  glGenFramebuffersEXT(1, &m_framebuffer);
  glGenTextures(1, &m_texture);
  glGenRenderbuffersEXT(1, &m_depthbuffer);
  
  if(m_framebuffer && m_depthbuffer)
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_framebuffer);
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
      
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, desc.bufferWidth, desc.bufferHeight,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D,
                              m_texture,
                              0);
    
    
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthbuffer);
    
    glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT,
                              GL_DEPTH_COMPONENT24,
                              desc.bufferWidth,
                              desc.bufferHeight);
                              
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT,
                                  GL_DEPTH_ATTACHMENT_EXT,
                                  GL_RENDERBUFFER_EXT,
                                  m_depthbuffer);
    
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            LV_ASSERT(0);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            LV_ASSERT(0);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            LV_ASSERT(0);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            LV_ASSERT(0);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            LV_ASSERT(0);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            LV_ASSERT(0);
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            LV_ASSERT(0);
            break;
        default:
            LV_ASSERT(0);
    }
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
      error = LV_OKAY;
    }
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
  }
  
  if(error != LV_OKAY)
  {
    Free();
  }
  
  return error;
}

/*!
**  Notes:    Signals OpenGL to write to this target.
**/
void OpenGLRenderTargetFBO::Begin(void)
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_framebuffer);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthbuffer);
  //glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glViewport(0, 0, m_targetWidth, m_targetHeight);
}

/*!
**  Notes:    Signals OpenGL to stop writing to this target.
**/
void OpenGLRenderTargetFBO::End(void)
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
  //glDrawBuffer(GL_FRONT);
}

OpenGLRenderTargetFBO::OpenGLRenderTargetFBO(LvAllocator &allocator, LvRenderer &renderer) :
m_allocator(allocator),
m_renderer(renderer),
OpenGLRenderTarget(allocator)
{
  m_texture       = 0;
  m_framebuffer   = 0;
  m_depthbuffer   = 0;
}

OpenGLRenderTargetFBO::~OpenGLRenderTargetFBO(void)
{

}

