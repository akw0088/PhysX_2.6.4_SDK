/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Implementation of the Renderer.
*/

// DEV NOTES
//
// -  We will phase out the use of GLU. It appears that
//    there are a number of crash causing bugs in its
//    various implementations. As far as I know we only
//    use it for mip map generation. We should probably
//    roll our own mipmap generator anyways because the
//    one in GLU sucks ass (there are methods for making
//    much nicer looking mipmaps). -jd
// -  Add support for Static meshes using vertex buffer
//    objects. -jd
// -  I think I am going to move from GLSL to just Cg
//    even though GLSL is the *standard*. GLSL just has
//    way too many bugs in the current implementations
//    and the Cg compiler offers some unique benefits
//    like working on Direct3D and OpenGL, being able to
//    compile multiple languages (like GLSL), and the fact
//    it actually fucking works. -jd

#include <stdio.h>

#include "OpenGLRenderer.h"
#include "GLStaticMesh.h"

#include "Renderer/MaterialManager/MaterialManagerImp.h"
#include "GLMaterialPass.h"
#include "GLRenderTarget.h"
#include "GLRenderTarget_pbuffer.h"
#include "GLRenderTarget_fbo.h"

#define NO_PUSH_POP_ATTR 1

static const GLuint g_vuiGLTextureFormats[] = 
{
  0,              /* 0 Channels - error */
  GL_LUMINANCE,   /* 1 Channel  - greyscale/alpha */
  0,              /* 2 Channels - error */
  GL_RGB,         /* 3 Channels - RGB */
  GL_RGBA         /* 4 Channels - RGB + Alpha */
};

GLuint g_vuiGLPrimitives[] =
{
  GL_POINTS,
  GL_LINES,
  GL_LINE_STRIP,
  GL_TRIANGLES,
  GL_TRIANGLE_STRIP,
  GL_TRIANGLE_FAN,
};

const LvUInt32 g_numGLPrimitives = sizeof(g_vuiGLPrimitives)/sizeof(g_vuiGLPrimitives[0]);


/**************
** Cg Voodoo **
**************/

static void handleCgError(void) 
{
    CGerror err = cgGetError();
    const char *pcError = cgGetErrorString(err);
    printf("%s\n", pcError); // USE USER STREAM INTERFACE!
    LV_ASSERT(0);
}

/*******************
** OpenGLRenderer **
*******************/

/*!
**  Notes:    Constructor.
**/
OpenGLRenderer::OpenGLRenderer(LvAllocator &allocator, LvUserIO &userio) : RendererImp(allocator, userio)
{
#ifdef LV_WINDOWS
  m_hWnd  = 0;
  m_hDc   = 0;
  m_hRc   = 0;

#elif defined(LV_APPLE)
  m_aglContext  = 0;
  m_winRef      = 0;

#elif defined(LV_PLAYSTATION3)
  m_device          = 0;
  m_jscontext       = 0;
  m_displayWidth    = 0;
  m_displayHeight   = 0;
  
#endif

#if CGGL_OK
  m_cgContext = 0;
#endif
  
  m_eMaterialPass = MAT_DEFAULT_PASS;

  memset(m_vtTextureUnits, 0, sizeof(m_vtTextureUnits));
  memset(m_extensions,     0, sizeof(m_extensions));
}

/*!
**  In:       lavaDesc - Initialization Data.
**
**  Return:   Error code.
**
**  Notes:    Initialize the Renderer.
**/
LvError OpenGLRenderer::Init(const LvLavaDesc &lavaDesc)
{
  LvError error = LV_OKAY;
  
  // Initialize our parent!
  error = RendererImp::Init(lavaDesc);
  if(error) return error;
  
  for(LvUInt32 i=0; i<LVR_NUM_TEXTURE_UNITS; i++)
  {
    m_vtTextureUnits[i].bEnabled  = LV_FALSE;
  #if USE_OPENGL_ES
    m_vtTextureUnits[i].uiUnitID  = GL_TEXTURE0 + i;
  #else
    m_vtTextureUnits[i].uiUnitID  = GL_TEXTURE0_ARB + i;
  #endif
  }
  
#if defined(LV_WINDOWS)
  LV_ASSERT(lavaDesc.hWnd);
  if(lavaDesc.hWnd)
    m_hWnd = lavaDesc.hWnd;
  else
    error = LV_BAD_INPUT;

#elif defined(LV_APPLE)
  LV_ASSERT(lavaDesc.winRef);
  if(lavaDesc.winRef)
    m_winRef = lavaDesc.winRef;
  else
    error = LV_BAD_INPUT;

#elif defined(LV_PLAYSTATION3)
  m_currentSurface    = 0;
  m_device            = lavaDesc.device;
  m_displayWidth      = lavaDesc.displayWidth;
  m_displayHeight     = lavaDesc.displayHeight;
  
  LV_ASSERT(!jsGetCurrentContext());
  
  m_jscontext = jsCreateContext();
  if(m_jscontext)
  {
    jsMakeCurrent(m_jscontext, m_device);

    jsResetCurrentContext();

    glGenSurfacesSCE(3, m_surfaces);
    GLuint aaBit = lavaDesc.antiAliasing ? GL_ANTIALIASED_BIT_SCE:0;
    glSetupSurfaceSCE(m_surfaces[0], GL_RGBA8,                GL_DRAWABLE_BIT_SCE|GL_ALLOW_SCAN_OUT_BIT_SCE|aaBit, m_displayWidth, m_displayHeight);
    glSetupSurfaceSCE(m_surfaces[1], GL_RGBA8,                GL_DRAWABLE_BIT_SCE|GL_ALLOW_SCAN_OUT_BIT_SCE|aaBit, m_displayWidth, m_displayHeight);
    glSetupSurfaceSCE(m_surfaces[2], GL_DEPTH24_STENCIL8_SCE, GL_DRAWABLE_BIT_SCE|aaBit,                           m_displayWidth, m_displayHeight);
    glBindSurfaceSCE(GL_DRAW_DEPTH_SCE, m_surfaces[2]);
    glBindSurfaceSCE(GL_DRAW_STENCIL_SCE, m_surfaces[2]);
    glBindSurfaceSCE(GL_DRAW_BUFFER0_ATI, m_surfaces[0]);

    glViewport(0, 0, m_displayWidth, m_displayHeight);
    glScissor( 0, 0, m_displayWidth, m_displayHeight);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glFlush();
  }

#endif
  
  if(error == LV_OKAY)
  {
    error = _InitOpenGL();
  }
  
  return error;
}

/*!
**  Return:   Error code.
**
**  Notes:    Shutdown the Renderer.
**/
LvError OpenGLRenderer::Shutdown(void)
{
  LvError error = LV_OKAY;
  
#if defined(LV_WINDOWS)
  if(m_hRc)
  {
    wglMakeCurrent(m_hDc, 0);
    wglDeleteContext(m_hRc);
    m_hRc = 0;
  }

#elif defined(LV_APPLE)
  if(m_aglContext)
	{
	  aglSetCurrentContext(0);
		aglDestroyContext(m_aglContext);
		m_aglContext = 0;
	}

#elif defined(LV_PLAYSTATION3)
  if(m_jscontext)
  {
    glDeleteSurfacesSCE(3, m_surfaces);
    jsDestroyContext(m_jscontext);
    m_jscontext = 0;
  }
  
#endif
  
  // Shutdown our parent!
  RendererImp::Shutdown();
  
  return error;
}

/*!
**  In:       caps - capabilities descriptor.
**
**  Return:   Error code.
**
**  Notes:    Get the renderer's capabilities.
**/
LvError OpenGLRenderer::GetCaps(LvrRendererCaps &caps)
{
  LvError ret = LV_OKAY;
  memcpy(&caps, &m_caps, sizeof(LvrRendererCaps));
  return ret;
}

/*!
**  In:       iWidth  - Width of Display region.
**            iHeight - Height of Display region.
**
**  Return:   Error code.
**
**  Notes:    Signal the Renderer that the display region resized.
**/
LvError OpenGLRenderer::Resized(LvUInt32 iWidth, LvUInt32 iHeight)
{
#ifdef LV_WINDOWS
  if(m_hWnd)
  {
    RECT tRect;
    GetWindowRect(m_hWnd, &tRect);

    iWidth  = tRect.right-tRect.left;
    iHeight = tRect.bottom-tRect.top;
  }
#endif
  
#ifdef LV_APPLE
  if(m_aglContext && m_winRef)
  {
    Rect rect;
    aglSetCurrentContext(m_aglContext);
    aglUpdateContext(m_aglContext);
    GetWindowPortBounds(m_winRef, &rect);
    iWidth  = rect.right  - rect.left;
    iHeight = rect.bottom - rect.top;
  }
#endif

  m_displayWidth  = iWidth;
  m_displayHeight = iHeight;
  
  glViewport(0, 0, iWidth, iHeight);
  
  return LV_OKAY;
}

/*!
**  In:       mProjection - Projection matrix (3D -> 2D).
**            mView       - The View matrix (ie, Camera).
**            pTarget     - The Render Target. 0 for screen buffer.
**
**  Return:   Error code.
**
**  Notes:    Render the scene.
**/
LvError OpenGLRenderer::Render(const LvMatrix44 &mProjection, const LvMatrix44 &mView, LvrRenderTarget *pTarget)
{
  if(_RenderQueueIsEmpty()) return LV_IGNORED;
  
#if defined(LV_WINDOWS)
  BOOL makeCurrentRet = wglMakeCurrent(m_hDc, m_hRc);
  LV_ASSERT(makeCurrentRet);
#elif defined(LV_APPLE)
  aglSetCurrentContext(m_aglContext);
#elif defined(LV_PLAYSTATION3)
  _BindCurrentSurface();
#endif
    
  OpenGLRenderTarget *pGLTarget = (OpenGLRenderTarget *)pTarget;
  if(pGLTarget)
  {
    pGLTarget->Begin();
  }
  else
  {
    glViewport(0, 0, m_displayWidth, m_displayHeight);
  }
  
  // Load the projection matrix.
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf((GLfloat *)&mProjection);
  glMatrixMode(GL_MODELVIEW);
  
  
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  
  LvVector4 vLightDir(-0.7f,0.5f,0.5f,0);
  vLightDir.Normalize();

  
  // Bind our Global Rendering states to OpenGL.
  _BindStates();
  
  // Load the view matrix.
  glPushMatrix();
  glLoadMatrixf((GLfloat *)&mView);
  
  glColor4f(1, 1, 1, 1);
  
  LvBool haveShadowVolumes = LV_FALSE;
  
  for(LvUInt32 i=0; i<m_numLights; i++)
  {
    LvrLight *pLight = m_lights[i];
    LV_ASSERT(pLight);
    if(pLight && pLight->GetShadowVolumes())
    {
      haveShadowVolumes = LV_TRUE;
      break;
    }
  }
  
  // Render with shadow volumes.
  if(haveShadowVolumes)
  {    
    for(LvUInt32 i=0; i<LVR_MAX_LIGHTS; i++)
    {
      _DisableLight(i);
    }
    
    // Render Ambient light only.
    glDepthFunc(GL_LESS);
    m_eMaterialPass = MAT_AMBIENT_PASS;
    _Traverse();
    
    // Render each light.
    for(LvUInt32 i=0; i<m_numLights; i++)
    {
      glClear(GL_STENCIL_BUFFER_BIT);
      LvrLight *light = _EnableLight(i);
      if(light)
      {
        LvrShadowVolumeNode *shadows = light->GetShadowVolumes();
        if(shadows)
        {
          // Render Shadow Volumes.
          m_materialManager.End();

        #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
          
        #else
          glPushAttrib(GL_ALL_ATTRIB_BITS);
        #endif
          glColorMask(0, 0, 0, 0);
          glShadeModel(GL_FLAT);
          glDepthMask(0);
          glDepthFunc(GL_LESS);
          glEnable(GL_STENCIL_TEST);
        
          if(m_extensions[LV_ATI_separate_stencil])  // ATI_separate_stencil
          {
          #if !defined(LV_PLAYSTATION3)
            glDisable(GL_CULL_FACE);

            glStencilFuncSeparateATI(GL_ALWAYS, GL_ALWAYS, 0, ~0);
            glStencilOpSeparateATI(GL_BACK,   GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
            glStencilOpSeparateATI(GL_FRONT,  GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
            
            for(LvrShadowVolumeNode *pCurr=shadows; pCurr; pCurr=pCurr->GetNext())
            {
              _RenderShadowVolume(pCurr);
            }
          #endif
          }
          else if(m_extensions[LV_EXT_stencil_two_side]) // EXT_stencil_two_side
          {
          #if !defined(LV_PLAYSTATION3)
            glDisable(GL_CULL_FACE);
            glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
            
            glActiveStencilFaceEXT(GL_BACK);
            glStencilFunc(GL_ALWAYS, 0, ~0);
            glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);

            glActiveStencilFaceEXT(GL_FRONT);
            glStencilFunc(GL_ALWAYS, 0, ~0);
            glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
            
            for(LvrShadowVolumeNode *pCurr=shadows; pCurr; pCurr=pCurr->GetNext())
            {
              _RenderShadowVolume(pCurr);
            }
          #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
            glStencilFunc(GL_NEVER, 0, ~0);
            glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
          #endif
          #endif
          }
          else  // This method renders the shadow volumes twice!
          {
            glStencilFunc(GL_ALWAYS, 0, ~0);
            
            glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
            glCullFace(GL_BACK);
            for(LvrShadowVolumeNode *pCurr=shadows; pCurr; pCurr=pCurr->GetNext())
            {
              _RenderShadowVolume(pCurr);
            }
            
            glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
            glCullFace(GL_FRONT);
            for(LvrShadowVolumeNode *pCurr=shadows; pCurr; pCurr=pCurr->GetNext())
            {
              _RenderShadowVolume(pCurr);
            }
            
            
          #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
            glStencilFunc(GL_NEVER, 0, ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glCullFace(GL_BACK);
          #endif
          }
        #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
          glDisable(GL_STENCIL_TEST);
          glDepthMask(1);
          glShadeModel(GL_SMOOTH);
          glColorMask(1,1,1,1);
          if(m_states[LVR_BACKFACE_CULLING]) glEnable(GL_CULL_FACE);
        #else
          glPopAttrib();
        #endif
          
          // Render Diffuse with shadows.
        #if USE_OPENGL_ES || NO_PUSH_POP_ATTR

        #else
          glPushAttrib(GL_ENABLE_BIT);
        #endif
            glStencilFunc(GL_EQUAL, 0, ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glEnable(GL_STENCIL_TEST);
            glDepthFunc(GL_EQUAL);
            m_eMaterialPass = MAT_DEFAULT_PASS;
            glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_BLEND);
            _Traverse();
            glDisable(GL_BLEND);
        #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
          glDisable(GL_STENCIL_TEST);
        #else
          glPopAttrib();
        #endif
        }
        else
        {
          // Render Diffuse without shadows.
        #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
          
        #else
          glPushAttrib(GL_ENABLE_BIT);
        #endif
            glDepthFunc(GL_EQUAL);
            m_eMaterialPass = MAT_DEFAULT_PASS;
            glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_BLEND);
            _Traverse();
            glDisable(GL_BLEND);
        #if USE_OPENGL_ES || NO_PUSH_POP_ATTR
          
        #else
          glPopAttrib();
        #endif
        }
        
        _DisableLight(i);
      }
    }
  }
  else // No Shadow Volumes.
  {
    // Initialize the lights.
    for(LvUInt32 i=0; i<m_numLights; i++)
    {
      _EnableLight(i);
    }
    
    // Disable extra Lights.
    for(LvUInt32 i=m_numLights; i<LVR_MAX_LIGHTS; i++)
    {
      _DisableLight(i);
    }
  
    m_eMaterialPass = MAT_DEFAULT_PASS;
    _Traverse();
  }
  
  
    
  glPopMatrix();
  
  // Reset the material once a frame.
  m_materialManager.End();
  
  // Clear the render queue.
  _ClearRenderQueue();
  
  // Stop writing to target.
  if(pGLTarget)
  {
    pGLTarget->End();
  }
  
#if defined(LV_WINDOWS)
  wglMakeCurrent(m_hDc, m_hRc);
#endif
  
  return LV_OKAY;
}

/*!
**  In:       desc - Dynamic mesh that we want to make Static.
**
**  Return:   Static mesh pointer. 0 for error.
**
**  Notes:    Generate a Static mesh based off a Dynamic mesh.
**            Static meshes are good because Lava has a chance to
**            upload them to the graphics card or do other data 
**            optimizations. CreateStaticMesh will make a copy
**            of any data inside the Dynamic mesh so it IS safe
**            to free the Dynamic meshes memory after creating
**            a Static mesh.
**/
LvrStaticMesh *OpenGLRenderer::CreateStaticMesh(const LvrDynamicMesh &desc)
{
  GLStaticMesh *pStaMesh = LV_NEW(m_allocator, GLStaticMesh) GLStaticMesh(m_allocator);
  LvError error = pStaMesh->Upload(desc);
  if(error != LV_OKAY)
  {
    LV_DELETE(m_allocator, GLStaticMesh, pStaMesh);
    pStaMesh = 0;
  }
  return pStaMesh;
}

/*!
**  In:       pMesh - Static mesh to destroy
**
**  Return:   Error code.
**
**  Notes:    Destroy a Static mesh freeing it from any memory
**            it may be present in.
**/
LvError OpenGLRenderer::ReleaseStaticMesh(LvrStaticMesh *pMesh)
{
  LvError error = LV_IGNORED;
  GLStaticMesh *pStaMesh = (GLStaticMesh*)pMesh;
  if(pStaMesh)
  {
    pStaMesh->Free();
    LV_DELETE(m_allocator, GLStaticMesh, pStaMesh);
    error = LV_OKAY;
  }
  return error;
}

/*!
**  In:       desc - Render Target Descriptor.
**
**  Return:   Pointer to Render Target or 0 for error.
**
**  Notes:    Create a new Render Target.
**/
LvrRenderTarget *OpenGLRenderer::CreateRenderTarget(const LvrRenderTargetDesc &desc)
{
  OpenGLRenderTarget *target = 0;
  
  if(m_extensions[LV_EXT_framebuffer_object])
  {
    target = LV_NEW(m_allocator, OpenGLRenderTargetFBO) OpenGLRenderTargetFBO(m_allocator, *this);
  }
  
  if(!target)
  {
    target = LV_NEW(m_allocator, OpenGLRenderTargetPBuffer) OpenGLRenderTargetPBuffer(m_allocator, *this);
  }
  
#if defined(LV_WINDOWS)
  wglMakeCurrent(m_hDc, m_hRc);
#endif
  
  if(target->LoadFromDesc(desc) != LV_OKAY)
  {
    LV_DELETE(m_allocator, OpenGLRenderTarget, target);
    target = 0;
  }
  
  return target;
}

/*!
**  In:       pTarget - Render Target to destroy.
**
**  Return:   Error code.
**
**  Notes:    Destroy a Render Target.
**/
LvError OpenGLRenderer::ReleaseRenderTarget(LvrRenderTarget *pTarget)
{
  OpenGLRenderTarget *pGLTarget = (OpenGLRenderTarget *)pTarget;
  if(pGLTarget)
  {
    pGLTarget->Free();
    LV_DELETE(m_allocator, OpenGLRenderTarget, pGLTarget);
    return LV_OKAY;
  }
  return LV_IGNORED;
}

#if defined(LV_PLAYSTATION3)
void OpenGLRenderer::_BindCurrentSurface(void)
{
  glBindSurfaceSCE(GL_DRAW_BUFFER0_ATI, m_surfaces[m_currentSurface]);
  glBindSurfaceSCE(GL_DRAW_DEPTH_SCE,   m_surfaces[2]);
  glBindSurfaceSCE(GL_DRAW_STENCIL_SCE, m_surfaces[2]);
}
#endif

/*!
**  Notes:    Swap the rendering buffers
**/
void OpenGLRenderer::SwapBuffers(void)
{
#if defined(LV_WINDOWS)
  wglMakeCurrent(m_hDc, m_hRc);
  ::SwapBuffers(m_hDc);
#elif defined(LV_APPLE)
  aglSetCurrentContext(m_aglContext);
  aglSwapBuffers(m_aglContext);
#elif defined(LV_PLAYSTATION3)
  glSwapScanToSurfaceSCE(m_surfaces[m_currentSurface]);
	m_currentSurface=1-m_currentSurface;
	_BindCurrentSurface();
#endif

  // Because we need updated states...
  // for instance LVR_DEPTH_WRITE.
  _BindStates();
  
  glClearColor(m_fog.clearcolor.x, m_fog.clearcolor.y, m_fog.clearcolor.z, m_fog.clearcolor.w);
  if(!m_states[LVR_DEPTH_WRITE]) glDepthMask(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if(!m_states[LVR_DEPTH_WRITE]) glDepthMask(0);
}

/*!
**  Return:   Material pass instance.
**
**  Notes:    Creates a new material pass.
**/
MaterialPass *OpenGLRenderer::CreateMaterialPass(void)
{
  GLMaterialPass *pass = LV_NEW(m_allocator, GLMaterialPass) GLMaterialPass(*this);
  return pass;
}

/*!
**  Notes:    Removes a meterial pass from memory.
**/
void OpenGLRenderer::ReleaseMaterialPass(MaterialPass *pass)
{
  LV_DELETE(m_allocator, MaterialPass, pass);
}

/*!
**  In:       LvrShaderProfileE - path to shader.
**            profile           - profile to use with shader.
**
**  Return:   Pointer to a new Shader.
**
**  Notes:    Creates a new Shader.
**/
Shader *OpenGLRenderer::CreateShader(const LvChar *pcPath, LvrShaderProfileE profile)
{
  Shader     *pShader   = 0;
  
  #define LOAD_CG_SHADER(_profile)                                                                        \
  {                                                                                                       \
    CGGLShader *pCGShader = LV_NEW(m_allocator,CGGLShader) CGGLShader(m_allocator,m_userio,m_cgContext);  \
    if(pCGShader->LoadFromFile(pcPath, _profile) != LV_OKAY)                                              \
      ReleaseShader(pCGShader);                                                                           \
    else                                                                                                  \
      pShader = pCGShader;                                                                                \
  }
  
  switch(profile)
  {
    case LVR_CG_ARBVP1:   LOAD_CG_SHADER(CG_PROFILE_ARBVP1);    break;
    case LVR_CG_ARBFP1:   LOAD_CG_SHADER(CG_PROFILE_ARBFP1);    break;
    case LVR_CG_VP20:     LOAD_CG_SHADER(CG_PROFILE_VP20);      break;
    case LVR_CG_FP20:     LOAD_CG_SHADER(CG_PROFILE_FP20);      break;
    case LVR_CG_VP30:     LOAD_CG_SHADER(CG_PROFILE_VP30);      break;
    case LVR_CG_FP30:     LOAD_CG_SHADER(CG_PROFILE_FP30);      break;
    case LVR_CG_VP40:     LOAD_CG_SHADER(CG_PROFILE_VP40);      break;
    case LVR_CG_FP40:     LOAD_CG_SHADER(CG_PROFILE_FP40);      break;
  #if defined(LV_PLAYSTATION3)
    case LVR_CG_SCEVP10:  LOAD_CG_SHADER(CG_PROFILE_SCE_VP10);  break;
    case LVR_CG_SCEFP10:  LOAD_CG_SHADER(CG_PROFILE_SCE_FP10);  break;
  #endif
  }
  
  #undef LOAD_CG_SHADER
  return pShader;
}

/*!
**  Notes:    Removes a shader from memory.
**/
void OpenGLRenderer::ReleaseShader(Shader *shader)
{
  LV_DELETE(m_allocator, Shader, shader);
}

/*!
**  In:       data    - Texture data.
**            desc    - texture descriptor.
**
**  Out:      guid    - Texture identifier.
**
**  Return:   Error code.
**
**  Notes:    Uploads a texture to the renderer (likely video card).
**/
LvError OpenGLRenderer::UploadTexture(LvGUID &guid, const LvrTextureData &data, const LvrTextureDesc &desc)
{
  GLuint  uiTextureID = 0;
  glGenTextures(1, &uiTextureID);
  LV_ASSERT(uiTextureID);
  if(!uiTextureID)
  {
    return LV_BAD_OUTPUT;
  }
  
  glBindTexture(GL_TEXTURE_2D, uiTextureID);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  LvBool mipmap = desc.m_mipmap;
#if defined(LV_PLAYSTATION3)
  // todo: why are mipmaps not working?
  //       is the texture size more limited?
  mipmap = LV_FALSE;
#endif
  
  // Assert on unsupported texture compression scheme.
  LV_ASSERT(m_caps.textureCompression[desc.m_compression]);
  
  GLuint internalFormat = data.m_depth;
  
  if( desc.m_compression != LVR_TEXTURE_RAW &&
      m_extensions[LV_EXT_texture_compression_s3tc] &&
      m_caps.textureCompression[desc.m_compression])
  {
    switch(desc.m_compression)
    {
      case LVR_TEXTURE_DXT1:
      {
        if(data.m_depth == 3)      internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        else if(data.m_depth == 4) internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
      }
      case LVR_TEXTURE_DXT3:
      {
        internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
      }
      case LVR_TEXTURE_DXT5:
      {
        internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
      }
    }
  }
  
  if(mipmap)
  {
    gluBuild2DMipmaps(GL_TEXTURE_2D, data.m_depth, data.m_width, data.m_height,
                      g_vuiGLTextureFormats[data.m_depth], GL_UNSIGNED_BYTE, data.m_data);
  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else
  {
    glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, data.m_width, data.m_height, 
                  0, g_vuiGLTextureFormats[data.m_depth], GL_UNSIGNED_BYTE, data.m_data);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  
  guid = *((LvGUID*)&uiTextureID);
  
  return LV_OKAY;
}

/*!
**  In:       gGUID     - Texture identifier.
**
**  Return:   Error code.
**
**  Notes:    Deletes a texture from the renderer (likely video card).
**/
LvError OpenGLRenderer::DeleteTexture(LvGUID &gGUID)
{
  GLuint  uiTextureID = *((GLuint*)&gGUID);
  gGUID = 0;
  
  if(uiTextureID)
  {
    glDeleteTextures(1, &uiTextureID);
  }
  
  return LV_OKAY;
}

/*!
**  In:       gGUID     - Texture identifier.
**            iTextureUnit  - Texture Unit.
**
**  Notes:    Bind a texture to a specific texture unit.
**/
void OpenGLRenderer::BindTexture(LvGUID gGUID, LvSInt32 iTextureUnit)
{
  if(iTextureUnit < 0 || iTextureUnit >= LVR_NUM_TEXTURE_UNITS)
    return;
  
  GLuint  uiUnitEnum  = m_vtTextureUnits[iTextureUnit].uiUnitID;
  GLuint  uiTextureID = *((GLuint *)&gGUID);
  
#if defined(LV_PLAYSTATION3)
  glActiveTexture(uiUnitEnum);
  glClientActiveTexture(uiUnitEnum);
#else
  glActiveTextureARB(uiUnitEnum);
  glClientActiveTextureARB(uiUnitEnum);
#endif
  glBindTexture(GL_TEXTURE_2D, uiTextureID);
}

/*!
**  In:       iNumTextureUnits  - Number of texture units to enable.
**
**  Notes:    Sets which texture units to enable (disabling all others).
**            For instance, iNumTextureUnits = 3 would enable units [0,1,2].
**/
void OpenGLRenderer::EnableTextureUnit(LvSInt32 iNumTextureUnits)
{
  if(iNumTextureUnits > LVR_NUM_TEXTURE_UNITS)
    iNumTextureUnits = LVR_NUM_TEXTURE_UNITS;
  
  // Enable Texture Units.
  for(LvSInt32 i=0; i<iNumTextureUnits; i++)
  {
    if(!m_vtTextureUnits[i].bEnabled)
    {
    #if defined(LV_PLAYSTATION3)
      glActiveTexture(m_vtTextureUnits[i].uiUnitID);
      glClientActiveTexture(m_vtTextureUnits[i].uiUnitID);
    #else
      glActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
      glClientActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
    #endif
      glEnable(GL_TEXTURE_2D);
      m_vtTextureUnits[i].bEnabled = LV_TRUE;
    }
  }
  
  // Disable Texture Units.
  for(LvSInt32 i=iNumTextureUnits; i<LVR_NUM_TEXTURE_UNITS; i++)
  {
    if(m_vtTextureUnits[i].bEnabled)
    {
    #if defined(LV_PLAYSTATION3)
      glActiveTexture(m_vtTextureUnits[i].uiUnitID);
      glClientActiveTexture(m_vtTextureUnits[i].uiUnitID);
    #else
      glActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
      glClientActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
    #endif
      glDisable(GL_TEXTURE_2D);
      m_vtTextureUnits[i].bEnabled = LV_FALSE;
    }
  }
}



/*!
**  In:       caps - capabilities descriptor.
**
**  Return:   Error code.
**
**  Notes:    Get the renderer's capabilities.
**/
LvError OpenGLRenderer::_GetCaps(LvrRendererCaps &caps)
{
  LvError ret = LV_OKAY;
  memset(&caps, 0, sizeof(LvrRendererCaps));
  
  GLint temp[4];
  #define GL_GET_INT(_name, _index) (LvUInt32)(glGetIntegerv(_name, temp),temp[_index])
  
  caps.maxTextureWidth              = GL_GET_INT(GL_MAX_TEXTURE_SIZE, 0);
  caps.maxTextureHeight             = GL_GET_INT(GL_MAX_TEXTURE_SIZE, 0);
  caps.maxTextureExtent             = GL_GET_INT(GL_MAX_TEXTURE_SIZE, 0);
  caps.maxTextureRepeat             = 0; // TODO.
  caps.maxTextureAspectRatio        = 0; // TODO.
  caps.maxAnisotropy                = 0; // TODO.
  caps.maxVertexW                   = 0; // TODO.
  caps.maxTextureBlendStages        = 0; // TODO.
  caps.maxSimultaneousTextures      = 0; // TODO.
  caps.maxActiveLights              = GL_GET_INT(GL_MAX_LIGHTS, 0);
  caps.maxUserClipPlanes            = GL_GET_INT(GL_MAX_CLIP_PLANES, 0);
  caps.maxVertexBlendMatrices       = 0; // TODO.
  caps.maxVertexBlendMatricesIndex  = 0; // TODO.
  caps.minPointSize                 = GL_GET_INT(GL_POINT_SIZE_RANGE, 0);
  caps.maxPointSize                 = GL_GET_INT(GL_POINT_SIZE_RANGE, 1);
  caps.maxPrimitiveCount            = 0; // TODO.
  caps.maxVertexIndex               = 0; // TODO.
  caps.maxStreams                   = 0; // TODO.
  caps.maxStreamStride              = 0; // TODO.
  
  #undef GL_GET_INT
  
  #define IS_PROFILE_SUPPORTED(_profile) \
  if(cgGLIsProfileSupported(CG_PROFILE_##_profile)) \
    caps.shaderProfiles[LVR_CG_##_profile] = LV_TRUE;
  
  IS_PROFILE_SUPPORTED(ARBVP1);
  IS_PROFILE_SUPPORTED(ARBFP1);
  IS_PROFILE_SUPPORTED(VP20);
  IS_PROFILE_SUPPORTED(FP20);
  IS_PROFILE_SUPPORTED(VP30);
  IS_PROFILE_SUPPORTED(FP30);
  IS_PROFILE_SUPPORTED(VP40);
  IS_PROFILE_SUPPORTED(FP40);
#if defined(LV_PLAYSTATION3)
  IS_PROFILE_SUPPORTED(SCEVP10);
  IS_PROFILE_SUPPORTED(SCEFP10);
#endif
  
  #undef IS_PROFILE_SUPPORTED
  
  caps.textureCompression[LVR_TEXTURE_RAW]  = LV_TRUE;
  caps.textureCompression[LVR_TEXTURE_DXT1] = m_extensions[LV_EXT_texture_compression_dxt1];
  caps.textureCompression[LVR_TEXTURE_DXT2] = m_extensions[LV_EXT_texture_compression_dxt2];
  caps.textureCompression[LVR_TEXTURE_DXT3] = m_extensions[LV_EXT_texture_compression_dxt3];
  caps.textureCompression[LVR_TEXTURE_DXT4] = m_extensions[LV_EXT_texture_compression_dxt4];
  caps.textureCompression[LVR_TEXTURE_DXT5] = m_extensions[LV_EXT_texture_compression_dxt5];
  
  if(m_extensions[LV_EXT_texture_compression_s3tc])
  {
    caps.textureCompression[LVR_TEXTURE_DXT1] = LV_TRUE;
    caps.textureCompression[LVR_TEXTURE_DXT3] = LV_TRUE;
    caps.textureCompression[LVR_TEXTURE_DXT5] = LV_TRUE;
  }
  
  return ret;
}

/*!
**  Return:   Error code.
**
**  Notes:    Initialize OpenGL for the current Window.
**/
LvError OpenGLRenderer::_InitOpenGL(void)
{
#if defined(LV_WINDOWS)
  LvSInt32  iPFIndex;
  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW |
    PFD_SUPPORT_OPENGL |
    PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0, 0, 0, 0,
    32,
    8,
    0,
    PFD_MAIN_PLANE,
    0,
    0, 0, 0
  };
  
  m_hDc = GetDC(m_hWnd);
  iPFIndex = ChoosePixelFormat(m_hDc, &pfd);
  SetPixelFormat(m_hDc, iPFIndex, &pfd);
  m_hRc = wglCreateContext(m_hDc);
  wglMakeCurrent(m_hDc, m_hRc);
  //wglSwapIntervalEXT(m_hDc, 0); // Disables V-sync.
  
  // Initialize OpenGL Extensions
  glextLoad();
  
  RECT tRect;
  GetWindowRect(m_hWnd, &tRect);
  LvSInt32  iWidth  = tRect.right-tRect.left,
            iHeight = tRect.bottom-tRect.top;
  glViewport(0, 0, iWidth, iHeight);

#elif defined(LV_APPLE)
  GLint attrib[] =
	{
		AGL_RGBA, 
		AGL_DOUBLEBUFFER, 
		AGL_ACCELERATED,
		AGL_DEPTH_SIZE, 32,
    AGL_STENCIL_SIZE,8,
		AGL_NONE,
	};
  
  SetPortWindowPort(m_winRef);
  AGLDevice gdev = GetDeviceList();
  AGLPixelFormat fmt = aglChoosePixelFormat(&gdev, 1, attrib);
  LV_ASSERT(fmt);
  
  if(fmt)
  {
    m_aglContext = aglCreateContext(fmt, 0);
    LV_ASSERT(m_aglContext);
    aglDestroyPixelFormat(fmt);
    aglSetDrawable(m_aglContext, GetWindowPort(m_winRef));
    aglSetCurrentContext(m_aglContext);
  }
  
#endif
  
  // Check available Extensions.
  const LvChar *extensions = (const LvChar*)glGetString(GL_EXTENSIONS);
  if(extensions)
  {
    #define CHECK_GL_EXTENSION(_ext) m_extensions[LV_##_ext] = strstr(extensions, "GL_" #_ext) ? LV_TRUE : LV_FALSE
    CHECK_GL_EXTENSION(EXT_stencil_two_side);
    CHECK_GL_EXTENSION(ATI_separate_stencil);
    CHECK_GL_EXTENSION(ARB_multitexture);
    CHECK_GL_EXTENSION(ARB_point_sprite);
    CHECK_GL_EXTENSION(EXT_texture_compression_dxt1);
    CHECK_GL_EXTENSION(EXT_texture_compression_dxt2);
    CHECK_GL_EXTENSION(EXT_texture_compression_dxt3);
    CHECK_GL_EXTENSION(EXT_texture_compression_dxt4);
    CHECK_GL_EXTENSION(EXT_texture_compression_dxt5);
    CHECK_GL_EXTENSION(EXT_texture_compression_s3tc);
    CHECK_GL_EXTENSION(EXT_framebuffer_object);
    #undef CHECK_GL_EXTENSION
  }
  
  if(m_extensions[LV_ARB_point_sprite])
  {
    glGetFloatv(GL_POINT_SIZE_MAX_ARB, &m_maxPointSpriteSize);
  }
  
  
  LvVector4 vFogColor(0,0,0,0);
  glClearColor(vFogColor.x, vFogColor.y, vFogColor.z, vFogColor.w);
  
  glEnable(GL_COLOR_MATERIAL);
  
#if CGGL_OK
  cgSetErrorCallback(handleCgError); 
  m_cgContext = cgCreateContext();
#endif
  
  _GetCaps(m_caps);
  
  Resized(0,0);

  return LV_OKAY;
}

/*!
**  Notes:    Binds the current renderer states to OpenGL.
**/
void OpenGLRenderer::_BindStates(void)
{
  // LVR_WIREFRAME
  if(m_states[LVR_WIREFRAME])
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  // LVR_BACKFACE_CULLING
  if(m_states[LVR_BACKFACE_CULLING])
  {
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
  }
  else
    glDisable(GL_CULL_FACE);
  
  // LVR_DEPTH_TEST
  if(m_states[LVR_DEPTH_TEST])
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  
  // LVR_DEPTH_WRITE
  if(m_states[LVR_DEPTH_WRITE])
    glDepthMask(1);
  else
    glDepthMask(0);
  
  // LVR_FOG
  if(m_states[LVR_FOG])
  {
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE,    GL_LINEAR);
    glFogf(GL_FOG_DENSITY, m_fog.density);
    glFogf(GL_FOG_START,   m_fog.start);
    glFogf(GL_FOG_END,     m_fog.end);
    glFogfv(GL_FOG_COLOR, &m_fog.fogcolor.x);
  }
  else
    glDisable(GL_FOG);
}

/*!
**  Return:   On success, returns the light;
**
**  Notes:    Enable a Light.
**/
LvrLight *OpenGLRenderer::_EnableLight(LvUInt32 index)
{
  LvrLight *pLight = m_lights[index];
  LV_ASSERT(pLight);
  if(pLight)
  {
    LvUInt32 uiGLLight = GL_LIGHT0+index;
    glEnable(uiGLLight);
    
    glLightfv(uiGLLight, GL_AMBIENT,  (GLfloat *)&pLight->ambient);
    glLightfv(uiGLLight, GL_DIFFUSE,  (GLfloat *)&pLight->diffuse);
    glLightfv(uiGLLight, GL_SPECULAR, (GLfloat *)&pLight->specular);
    
    glLightf(uiGLLight, GL_CONSTANT_ATTENUATION,  (GLfloat)pLight->constantAttenuation);
    glLightf(uiGLLight, GL_LINEAR_ATTENUATION,    (GLfloat)pLight->linearAttenuation);
    glLightf(uiGLLight, GL_QUADRATIC_ATTENUATION, (GLfloat)pLight->quadraticAttenuation);
    
    switch(pLight->m_eType)
    {
      // Directional Light.
      case LVR_LIGHT_DIRECTIONAL:
      {
        LvrDirectionalLight *pDirLight = (LvrDirectionalLight *)pLight;
        LvVector4 vPosition(pDirLight->direction); vPosition.w = 0;
        glLightfv(uiGLLight, GL_POSITION, (GLfloat *)&vPosition);
        break;
      }
      // Point Light.
      case LVR_LIGHT_POINT:
      {
        LvrPointLight *pPntLight = (LvrPointLight *)pLight;
        LvVector4 vPosition(pPntLight->position); vPosition.w = 1;
        glLightfv(uiGLLight, GL_POSITION, (GLfloat *)&vPosition);
        break;
      }
      // Spot Light.
      case LVR_LIGHT_SPOT:
      {
        LvrSpotLight *pSptLight = (LvrSpotLight *)pLight;
        LvVector4 vPosition(pSptLight->position); vPosition.w = 1;
        glLightfv(uiGLLight, GL_POSITION, (GLfloat *)&vPosition);
        LvVector3 vDirection(pSptLight->direction);
        glLightfv(uiGLLight, GL_SPOT_DIRECTION, (GLfloat *)&vDirection);
        glLightf(uiGLLight, GL_SPOT_EXPONENT, pSptLight->exponent);
        glLightf(uiGLLight, GL_SPOT_CUTOFF, pSptLight->cutoff);
        break;
      }
    }
  }
  return pLight;
}

/*!
**  Notes:    Disable a Light.
**/
void OpenGLRenderer::_DisableLight(LvUInt32 index)
{
  glDisable(GL_LIGHT0+index);
}

/*!
**  In:       pMesh - Mesh to render.
**
**  Notes:    Render a Mesh.
**/
void OpenGLRenderer::_RenderMesh(const LvrMesh *pMesh)
{
  LV_ASSERT(pMesh);
  
  if(pMesh->eType == LVR_MESH_STATIC)
  {
    const GLStaticMesh *pStaMesh = (const GLStaticMesh*)pMesh;
    pStaMesh->Render();
  }
  else if(pMesh->eType == LVR_MESH_DYNAMIC)
  {
    const LvrDynamicMesh *pDynMesh = (const LvrDynamicMesh *)pMesh;
    LV_ASSERT(pDynMesh->positions.buffer && pDynMesh->positions.numVerts); // We require vertex data.
    
    if(pDynMesh->positions.buffer)
    {
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(pDynMesh->positions.vertSize, GL_FLOAT, pDynMesh->positions.stride, pDynMesh->positions.buffer);
    }
    
    if(pDynMesh->normals.buffer)
    {
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_FLOAT, pDynMesh->normals.stride, pDynMesh->normals.buffer);
    }
    
    for(LvUInt32 i=0; i<pDynMesh->numTexCoords; i++)
    {
      LV_ASSERT(pDynMesh->texcoords[i].buffer);
      if(pDynMesh->texcoords[i].buffer)
      {
      #if defined(LV_PLAYSTATION3)
        glActiveTexture(m_vtTextureUnits[i].uiUnitID);
        glClientActiveTexture(m_vtTextureUnits[i].uiUnitID);
      #else
        glActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
        glClientActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
      #endif
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(pDynMesh->texcoords[i].vertSize, GL_FLOAT, pDynMesh->texcoords[i].stride, pDynMesh->texcoords[i].buffer);
      }
    }
    
    if(pDynMesh->colors.buffer)
    {
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(pDynMesh->colors.vertSize, GL_FLOAT, pDynMesh->colors.stride, pDynMesh->colors.buffer);
    }
    
    // We can't have 32 and 16 bit indices!
    LV_ASSERT(!pDynMesh->indices32 || !pDynMesh->indices16);
    

    
    LvrPrimitivesE primitives = pDynMesh->primitives;
    
    // Point sprite support.
    if(pDynMesh->primitives == LVR_POINT_SPRITES)
    {
      primitives = LVR_POINTS;
    #if 1 // TODO
      if(m_extensions[LV_ARB_point_sprite])
      {
        //GLfloat quadratic[] =  { 1.0f, 0.0f, 0.01f };
        GLfloat quadratic[] =  { 0.0f, 0.0f, 1.0f };
        glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
        glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);
        glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, m_maxPointSpriteSize);
        glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
        glEnable(GL_POINT_SPRITE_ARB);
        glPointSize(64.0f);
      }
    #endif
    }
    
    if(pDynMesh->indices32)
    {
      glDrawElements( g_vuiGLPrimitives[primitives], pDynMesh->numIndices,
                      GL_UNSIGNED_INT, pDynMesh->indices32);
    }
    else if(pDynMesh->indices16)
    {
      glDrawElements( g_vuiGLPrimitives[primitives], pDynMesh->numIndices,
                      GL_UNSIGNED_SHORT, pDynMesh->indices16);
    }
    else
    {
      glDrawArrays(g_vuiGLPrimitives[primitives], 0, pDynMesh->positions.numVerts);
    }
    
    // End point sprite support.
    if(pDynMesh->primitives == LVR_POINT_SPRITES)
    {
    #if 1 // TODO
      if(m_extensions[LV_ARB_point_sprite])
      {
        glDisable(GL_POINT_SPRITE_ARB);
        glPointSize(1.0f);
      }
    #endif
    }
    
    if(pDynMesh->colors.buffer)
      glDisableClientState(GL_COLOR_ARRAY);
    
    for(LvUInt32 i=0; i<pDynMesh->numTexCoords; i++)
    {
    #if defined(LV_PLAYSTATION3)
      glActiveTexture(m_vtTextureUnits[i].uiUnitID);
      glClientActiveTexture(m_vtTextureUnits[i].uiUnitID);
    #else
      glActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
      glClientActiveTextureARB(m_vtTextureUnits[i].uiUnitID);
    #endif
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    if(pDynMesh->normals.buffer)
      glDisableClientState(GL_NORMAL_ARRAY);
    
    if(pDynMesh->positions.buffer)
      glDisableClientState(GL_VERTEX_ARRAY);
  }
}

/*!
**  In:       pMeshNode - Current Mesh Node.
**
**  Notes:    Render a Mesh Node.
**/
void OpenGLRenderer::_RenderNode(LvrMeshNode *pMeshNode)
{
  LV_ASSERT(pMeshNode);

  // Are we going to apply a transform to this Mesh?
  LvBool bTransform = pMeshNode->flags & LVR_NO_TRANSFORM || !pMeshNode->transforms ? LV_FALSE : LV_TRUE;
  
  if(bTransform)
  {
    glPushMatrix();
    glMultMatrixf((const GLfloat*)pMeshNode->transforms);
  }
  
  // Are we going to apply a material to this Mesh?
  LvBool bMaterial = pMeshNode->flags & LVR_NO_MATERIAL || !GetState(LVR_MATERIALS) || !pMeshNode->material ? LV_FALSE : LV_TRUE;

  // Set the material.
  if(bMaterial)
    m_materialManager.Begin(m_eMaterialPass, pMeshNode->material);
  else
    m_materialManager.End();
  
  LV_ASSERT(pMeshNode->mesh);
  
  if(pMeshNode->mesh)
    _RenderMesh(pMeshNode->mesh);
  
  if(bTransform)
  {
    glPopMatrix();
  }
}


/*!
**  In:       shadow - shadow volume node.
**
**  Notes:    Render a shadow volume.
**/
void OpenGLRenderer::_RenderShadowVolume(LvrShadowVolumeNode *shadow)
{
  const GLfloat *transform = (const GLfloat*)shadow->transform;
  if(transform)
  {
    glPushMatrix();
    glMultMatrixf(transform);
  }
  _RenderMesh(shadow->mesh);
  if(transform)
  {
    glPopMatrix();
  }
}















