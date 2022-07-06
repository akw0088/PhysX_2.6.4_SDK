/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Direct3D Implementation of the Renderer.
*/

#ifndef LV_DIRECT3DRENDERER_H
#define LV_DIRECT3DRENDERER_H

#include <Lava/LvLava.h>

// Don't Compile D3D Support if we are on the wrong platform.
#if defined(LV_WINDOWS) || defined(LV_XENON)

#include "Renderer/RendererImp.h"

#include <Lava/Utils/LvLibrary.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.h>

/*!
**  In:       mesh - Mesh to calculate the size of.
**
**  Notes:    Calculates the size of a single vertex.
**/
LvUInt32 LVD3D_CalcVertexSize(const LvrDynamicMesh &mesh);

/*!
**  Notes:    Direct3D Implementation of the Renderer.
**/
class Direct3DRenderer : public RendererImp
{
  public:
    /*!
    **  Notes:    Constructor.
    **/
    Direct3DRenderer(LvAllocator &allocator, LvUserIO &userio);
    
    /*!
    **  Notes:    Destructor.
    **/
    ~Direct3DRenderer(void);
    
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
  
  protected:
    /*!
    **  In:       pMeshNode - Current Mesh Node.
    **
    **  Notes:    Render a Mesh Node.
    **/
    void _RenderNode(LvrMeshNode *pMeshNode);
    
    /*!
    **  In:       mesh - Mesh to render.
    **
    **  Notes:    Render a Mesh.
    **/
    void _RenderMesh(const LvrMesh *mesh);
    
    /*!
    **  In:       size - size needed in the vertex cache.
    **
    **  Notes:    Assures the vertex cache is a minimal size.
    **/
    void _ResizeVertexCache(LvUInt32 size);
  
  private:
    HMODULE           m_library;
    IDirect3D9       *m_d3d;
    IDirect3DDevice9 *m_d3dDevice;
    
    LvUInt8          *m_vertexCache;
    LvUInt32          m_vertexCacheSize;
    
    MaterialPassE     m_materialPass;
};

#endif
#endif
