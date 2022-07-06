/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Renderer Implementor.
*/

#ifndef LV_RENDERERIMP_H
#define LV_RENDERERIMP_H

#define LVR_NUM_TEXTURE_UNITS   4
#define LVR_MAX_LIGHTS          8

#include <Lava/LvLava.h>
#include <Lava/Renderer/LvRenderer.h>

#include "MaterialManager/MaterialManagerImp.h"
#include "MaterialManager/TextureManager.h"
#include "MaterialManager/Shader.h"

/*!
**  Notes:    Renderer Implementor.
**/
class RendererImp : public LvRenderer
{
  protected:
    RendererImp(LvAllocator &allocator, LvUserIO &userio);
  
  public:
    ~RendererImp(void);
    
  public:
    /*!
    **  In:       allocator - memory allocator.
    **            userio    - User I/O stream.
    **            eAPI      - indicates which version to use.
    **
    **  Return:   Pointer to the instance of the Renderer.
    **
    **  Notes:    Get or create an instance of the Renderer.
    **/
    static RendererImp *CreateInstance(LvAllocator &allocator, LvUserIO &userio, LvrRendererAPI eAPI);
    
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
    **  In:       ptMesh  - Mesh descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    Add a mesh to the render queue.
    **            WARNING! Do not add the same MeshNode twice
    **            between renders.
    **/
    virtual LvError AddMesh(LvrMeshNode *ptMesh);
    
    /*!
    **  In:       pLight  - Light descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    Add a global light to the render queue.
    **            WARNING! Do not add the same Light twice
    **            between Render calls.
    **/
    virtual LvError AddLight(LvrLight *pLight);
    
    /*!
    **  In:       eState  - Global renderer state.
    **
    **  Notes:    Enable a global renderer state.
    **/
    virtual void EnableState(LvrRendererStateE eState);
    
    /*!
    **  In:       eState  - Global renderer state.
    **
    **  Notes:    Disable a global renderer state.
    **/
    virtual void DisableState(LvrRendererStateE eState);
    
    /*!
    **  In:       eState  - Global renderer state.
    **
    **  Return:   LV_TRUE if the state is enabled.
    **
    **  Notes:    Gets the current state of a renderer state.
    **/
    virtual LvBool GetState(LvrRendererStateE eState);
    
    /*!
    **  In:       desc - fog descriptor.
    **
    **  Notes:    Sets the fog descriptor.
    **/
    virtual void SetFogDesc(const LvrFogDesc &desc);
  
  public:
    /*!
    **  In:       path  - path to texture file.
    **            desc  - texture descriptor.
    **
    **  Return:   Instance of a texture, or 0 for error.
    **
    **  Notes:    Gets an instance of a texture and loads from
    **            file if needed.
    **/
    virtual LvrTexture *GetTextureFromFile(const LvChar *path, const LvrTextureDesc &desc);
    
    /*!
    **  In:       data - texture data.
    **            desc - texture descriptor.
    **
    **  Return:   Instance of a texture, or 0 for error.
    **
    **  Notes:    Creates a texture from data already in memory.
    **/
    virtual LvrTexture *GetTextureFromMemory(const LvrTextureData &data, const LvrTextureDesc &desc);
    
    /*!
    **  In:       files - paths to texture files.
    **            desc  - texture descriptor.
    **
    **  Return:   Instance of a texture, or 0 for error.
    **
    **  Notes:    Gets an instance of a texture and loads from
    **            file if needed.
    **/
    virtual LvrTexture *GetCubeMapFromFiles(const LvrCubeMapFiles &files, const LvrTextureDesc &desc);
    
    /*!
    **  In:       texture - the texture to return.
    **
    **  Notes:    Return a texture after its finished being used.
    **/
    virtual void ReturnTexture(LvrTexture *texture);
    
    /*!
    **  In:       desc - shader descriptor.
    **
    **  Return:   Instance of a shader, or 0 for error.
    **
    **  Notes:    Loads a shader.
    **/
    virtual LvrShader *GetShader(const LvrShaderDesc &desc);
    
    /*!
    **  In:       shader - the shader to return.
    **
    **  Notes:    Return a shader after its finished being used.
    **/
    virtual void ReturnShader(LvrShader *shader);
    
    /*!
    **  In:       tDesc - A material descriptor.
    **
    **  Return:   Instance of a material or 0 for error.
    **
    **  Notes:    Creates an instance of a material.
    **/
    virtual LvrMaterial *CreateMaterial(const LvrMaterialDesc &tDesc);
    
    /*!
    **  In:       pMaterial - Material Instance.
    **
    **  Notes:    Destroys a material.
    **/
    virtual void ReleaseMaterial(LvrMaterial *pMaterial);
  
  public:
    /*!
    **  Return:   Material pass instance.
    **
    **  Notes:    Creates a new material pass.
    **/
    virtual MaterialPass *CreateMaterialPass(void) = 0;
    
    /*!
    **  Notes:    Removes a meterial pass from memory.
    **/
    virtual void ReleaseMaterialPass(MaterialPass *pass) = 0;
    
    /*!
    **  In:       LvrShaderProfileE - path to shader.
    **            profile           - profile to use with shader.
    **
    **  Return:   Pointer to a new Shader.
    **
    **  Notes:    Creates a new Shader.
    **/
    virtual Shader *CreateShader(const LvChar *pcPath, LvrShaderProfileE profile) = 0;
    
    /*!
    **  Notes:    Removes a shader from memory.
    **/
    virtual void ReleaseShader(Shader *shader) = 0;
    
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
    virtual LvError UploadTexture(LvGUID &guid, const LvrTextureData &data, const LvrTextureDesc &desc) = 0;
    
    /*!
    **  In:       gGUID     - Texture identifier.
    **
    **  Return:   Error code.
    **
    **  Notes:    Deletes a texture from the renderer (likely video card).
    **/
    virtual LvError DeleteTexture(LvGUID &gGUID) = 0;
  
  protected:
    /*!
    **  In:       pMeshNode - Current Mesh Node.
    **
    **  Notes:    Render a Mesh Node.
    **/
    virtual void _RenderNode(LvrMeshNode *pMeshNode) = 0;
    
    /*!
    **  Notes:    Traverses the render queue.
    **/
    LV_INLINE void _Traverse(void)
    {
      if(m_headMeshMode)
        _Traverse(m_headMeshMode);
    }
    
    /*!
    **  Notes:    Clears the render queue.
    **/
    LV_INLINE void _ClearRenderQueue(void)
    {
      m_headMeshMode    = 0;
      m_numLights       = 0;
    }
    
    /*!
    **  Notes:    Returns LV_TRUE if the render queue is empty.
    **/
    LV_INLINE LvBool _RenderQueueIsEmpty(void)
    {
      return m_headMeshMode ? LV_FALSE : LV_TRUE;
    }
  
  private:
    /*!
    **  In:       pMeshNode - Current Mesh Node.
    **
    **  Notes:    Traverses the render queue.
    **/
    void _Traverse(LvrMeshNode *pMeshNode);
    
  protected:
    LvAllocator          &m_allocator;
    LvUserIO             &m_userio;
    
    LvrLight             *m_lights[LVR_MAX_LIGHTS];
    LvUInt32              m_numLights;
    
    TextureManager        m_textureManager;
    ShaderManager         m_shaderManager;
    MaterialManagerImp    m_materialManager;
    
    LvrFogDesc            m_fog;
    
    LvBool                m_states[LVR_NUM_RENDERER_STATES];
  private:
    LvrMeshNode          *m_headMeshMode;
};

#endif
