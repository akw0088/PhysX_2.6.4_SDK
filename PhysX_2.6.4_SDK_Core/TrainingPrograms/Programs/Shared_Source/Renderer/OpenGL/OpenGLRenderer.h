/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Implementation of the Renderer.
*/

// DEV NOTES
//
// -  How do we mix the existance of standard texture files
//    and Render Target Textures? They are both named and 
//    should have the same interface as far as materials go. -jd

#ifndef LV_OPENGLRENDERER_H
#define LV_OPENGLRENDERER_H

#include "../RendererImp.h"

// Include OpenGL and extensions.
#if defined(LV_WINDOWS)
  #include "OpenGLExtensions/OpenGLExtensions.h"
  
#elif defined(LV_APPLE)
  #include <AGL/agl.h>
  #include <OpenGL/glu.h>
  #include <OpenGL/glext.h>
  
#elif defined(LV_PLAYSTATION3)
  #include <JS/js.h>
  #include <JS/jsu.h>
  #include <GLES/glext.h>
  #define USE_OPENGL_ES 1
  
#else
  #error static or dynamic linked?
#endif

#include "CGGLShader.h"

typedef enum OGLExtensions
{
  LV_EXT_stencil_two_side = 0,
  LV_ATI_separate_stencil,
  LV_ARB_multitexture,
  LV_ARB_point_sprite,
  LV_EXT_texture_compression_dxt1,
  LV_EXT_texture_compression_dxt2,
  LV_EXT_texture_compression_dxt3,
  LV_EXT_texture_compression_dxt4,
  LV_EXT_texture_compression_dxt5,
  LV_EXT_texture_compression_s3tc,
  LV_EXT_framebuffer_object,
  LV_NUM_OGL_EXTENSIONS,
};

typedef struct _GLTextureUnitT
{
  LvBool    bEnabled;
  GLuint    uiUnitID;
} GLTextureUnitT;

/*!
**  Notes:    OpenGL Implementation of the Renderer.
**/
class OpenGLRenderer : public RendererImp
{
  public:
    /*!
    **  Notes:    Constructor.
    **/
    OpenGLRenderer(LvAllocator &allocator, LvUserIO &userio);
    
  public:
    /*!
    **  In:       lavaDesc - Initialization Data.
    **
    **  Return:   Error code.
    **
    **  Notes:    Initialize the Renderer.
    **/
    virtual LvError Init(const LvLavaDesc &lavaDesc);
    
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Shutdown the Renderer.
    **/
    virtual LvError Shutdown(void);
    
    /*!
    **  In:       caps - capabilities descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    Get the renderer's capabilities.
    **/
    virtual LvError GetCaps(LvrRendererCaps &caps);
    
    /*!
    **  In:       iWidth  - Width of Display region.
    **            iHeight - Height of Display region.
    **
    **  Return:   Error code.
    **
    **  Notes:    Signal the Renderer that the display region resized.
    **/
    virtual LvError Resized(LvUInt32 iWidth, LvUInt32 iHeight);
    
    /*!
    **  In:       mProjection - Projection matrix (3D -> 2D).
    **            mView       - The View matrix (ie, Camera).
    **            pTarget     - The Render Target. 0 for screen buffer.
    **
    **  Return:   Error code.
    **
    **  Notes:    Render the scene.
    **/
    virtual LvError Render(const LvMatrix44 &mProjection, const LvMatrix44 &mView, LvrRenderTarget *pTarget);
    
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
    virtual LvrStaticMesh *CreateStaticMesh(const LvrDynamicMesh &desc);
    
    /*!
    **  In:       pMesh - Static mesh to destroy
    **
    **  Return:   Error code.
    **
    **  Notes:    Destroy a Static mesh freeing it from any memory
    **            it may be present in.
    **/
    virtual LvError ReleaseStaticMesh(LvrStaticMesh *pMesh);
    
    /*!
    **  In:       desc - Render Target Descriptor.
    **
    **  Return:   Pointer to Render Target or 0 for error.
    **
    **  Notes:    Create a new Render Target.
    **/
    virtual LvrRenderTarget *CreateRenderTarget(const LvrRenderTargetDesc &desc);
    
    /*!
    **  In:       pTarget - Render Target to destroy.
    **
    **  Return:   Error code.
    **
    **  Notes:    Destroy a Render Target.
    **/
    virtual LvError ReleaseRenderTarget(LvrRenderTarget *pTarget);
    
    /*!
    **  Notes:    Swap the rendering buffers
    **/
    virtual void SwapBuffers(void);
    
  public:
    /*!
    **  Return:   Material pass instance.
    **
    **  Notes:    Creates a new material pass.
    **/
    virtual MaterialPass *CreateMaterialPass(void);
    
    /*!
    **  Notes:    Removes a meterial pass from memory.
    **/
    virtual void ReleaseMaterialPass(MaterialPass *pass);
    
    /*!
    **  In:       LvrShaderProfileE - path to shader.
    **            profile           - profile to use with shader.
    **
    **  Return:   Pointer to a new Shader.
    **
    **  Notes:    Creates a new Shader.
    **/
    virtual Shader *CreateShader(const LvChar *pcPath, LvrShaderProfileE profile);
    
    /*!
    **  Notes:    Removes a shader from memory.
    **/
    virtual void ReleaseShader(Shader *shader);
    
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
    virtual LvError UploadTexture(LvGUID &guid, const LvrTextureData &data, const LvrTextureDesc &desc);
    
    /*!
    **  In:       gGUID     - Texture identifier.
    **
    **  Return:   Error code.
    **
    **  Notes:    Deletes a texture from the renderer (likely video card).
    **/
    virtual LvError DeleteTexture(LvGUID &gGUID);
    
    /*!
    **  In:       gGUID     - Texture identifier.
    **            iTextureUnit  - Texture Unit.
    **
    **  Notes:    Bind a texture to a specific texture unit.
    **/
    virtual void BindTexture(LvGUID gGUID, LvSInt32 iTextureUnit);
    
    /*!
    **  In:       iNumTextureUnits  - Number of texture units to enable.
    **
    **  Notes:    Sets which texture units to enable (disabling all others).
    **            For instance, iNumTextureUnits = 3 would enable units [0,1,2].
    **/
    virtual void EnableTextureUnit(LvSInt32 iNumTextureUnits);
  
  private:
    /*!
    **  In:       caps - capabilities descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    Get the renderer's capabilities.
    **/
    LvError _GetCaps(LvrRendererCaps &caps);
    
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Initialize OpenGL for the current Window.
    **/
    LvError _InitOpenGL(void);
    
    /*!
    **  Notes:    Binds the current renderer states to OpenGL.
    **/
    void _BindStates(void);
    
    /*!
    **  Return:   On success, returns the light;
    **
    **  Notes:    Enable a Light.
    **/
    LvrLight *_EnableLight(LvUInt32 index);
    
    /*!
    **  Notes:    Disable a Light.
    **/
    void _DisableLight(LvUInt32 index);
    
    /*!
    **  In:       pMesh - Mesh to render.
    **
    **  Notes:    Render a Mesh.
    **/
    void _RenderMesh(const LvrMesh *pMesh);
    
  protected:
    /*!
    **  In:       pMeshNode - Current Mesh Node.
    **
    **  Notes:    Render a Mesh Node.
    **/
    void _RenderNode(LvrMeshNode *pMeshNode);
    
    /*!
    **  In:       shadow - shadow volume node.
    **
    **  Notes:    Render a shadow volume.
    **/
    void _RenderShadowVolume(LvrShadowVolumeNode *shadow);
  
  private:
  #if defined(LV_PLAYSTATION3)
    void _BindCurrentSurface(void);
  #endif
  
  public:
    // Windows Specifics.
  #if   defined(LV_WINDOWS)
    HWND        m_hWnd;
    HDC         m_hDc;
    HGLRC       m_hRc;
  #elif defined(LV_APPLE)
    WindowRef   m_winRef;
    AGLContext  m_aglContext;
  #elif defined(LV_PLAYSTATION3)
    JsDevice   *m_device;
    JsContext  *m_jscontext;
    GLuint      m_surfaces[3]; // rgb8, rgb8, depth24/stencil8
    GLuint      m_currentSurface;
  #endif
    LvUInt32    m_displayWidth;
    LvUInt32    m_displayHeight;
  
  #if CGGL_OK
    CGcontext   m_cgContext;
  #endif
    
  private:
    LvBool              m_extensions[LV_NUM_OGL_EXTENSIONS];
    GLfloat             m_maxPointSpriteSize;
    
    GLTextureUnitT      m_vtTextureUnits[LVR_NUM_TEXTURE_UNITS];
    
    MaterialPassE       m_eMaterialPass;
    
    LvrRendererCaps     m_caps;
};

#endif
