/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    
*/

#include "RendererImp.h"

#include "OpenGL/OpenGLRenderer.h"
#include "Direct3D/Direct3DRenderer.h"

#if defined(LV_WINDOWS)
#pragma warning(disable : 4355)
#endif

/*!
**  In:       pVol  - Shadow Volume Descriptor.
**
**  Return:   Error code.
**
**  Notes:    Add a Shadow Volume to the light.
**            WARNING! Do not add the same Shadow Volume twice.
**/
LvError LvrLight::AddShadowVolume(LvrShadowVolumeNode *shadow)
{
  LvError ret = LV_IGNORED;
  if(shadow)
  {
    shadow->_pN = m_headShadowVolume;
    m_headShadowVolume = shadow;
    ret = LV_OKAY;
  }
  return ret;
}

/*!
**  Notes:    Clear the list of shadow volumes.
**/
void LvrLight::ClearShadowVolumeList(void)
{
  m_headShadowVolume = 0;
}

LvrFogDesc::LvrFogDesc(void)
{
  density = 1;
  start   = 0;
  end     = 1;
  fogcolor.Set(0,0,0,0);
  clearcolor.Set(0,0,0,0);
}


RendererImp::RendererImp(LvAllocator &allocator, LvUserIO &userio) :
m_allocator(allocator),
m_userio(userio),
m_textureManager(allocator, userio, *this),
m_shaderManager(allocator, *this),
m_materialManager(allocator, userio, *this)
{
  memset(m_states, 0, sizeof(m_states));
}

RendererImp::~RendererImp(void)
{

}

/*!
**  In:       allocator - memory allocator.
**            userio    - User I/O stream.
**            eAPI      - indicates which version to use.
**
**  Return:   Pointer to the instance of the Renderer.
**
**  Notes:    Get or create an instance of the Renderer.
**/
RendererImp *RendererImp::CreateInstance(LvAllocator &allocator, LvUserIO &userio, LvrRendererAPI eAPI)
{
  RendererImp *pInstance = 0;
  
  switch((LvSInt32)eAPI)
  {
    case LVR_DEFAULT_RENDERER:
    case LVR_OPENGL:
    {
      pInstance = LV_NEW(allocator, OpenGLRenderer) OpenGLRenderer(allocator, userio);
      break;
    }
    #ifdef LV_WINDOWS
    case LVR_DIRECT3D:
    {
      pInstance = LV_NEW(allocator, Direct3DRenderer) Direct3DRenderer(allocator, userio);
      break;
    }
    #endif
  }
  
  LV_ASSERT(pInstance);
  
  return pInstance;
}

/*!
**  In:       lavaDesc - Initialization Data.
**
**  Return:   Error code.
**
**  Notes:    Initialize the Renderer.
**/
LvError RendererImp::Init(const LvLavaDesc &lavaDesc)
{
  m_headMeshMode    = 0;
  m_numLights       = 0;
  
  m_textureManager.Init();
  m_shaderManager.Init();
  m_materialManager.Init();
  
  memset(m_states, 0, sizeof(m_states));
  
  // Set default states.
  DisableState(LVR_WIREFRAME);
  EnableState(LVR_BACKFACE_CULLING);
  EnableState(LVR_DEPTH_TEST);
  EnableState(LVR_DEPTH_WRITE);
  EnableState(LVR_MATERIALS);
  EnableState(LVR_SHADERS);
  
  return LV_OKAY;
}

/*!
**  Return:   Error code.
**
**  Notes:    Shutdown the Renderer.
**/
LvError RendererImp::Shutdown(void)
{
  m_materialManager.Shutdown();
  m_shaderManager.Shutdown();
  m_textureManager.Shutdown();
  
  return LV_OKAY;
}

/*!
**  In:       ptMesh  - Mesh descriptor.
**
**  Return:   Error code.
**
**  Notes:    Add a mesh to the render queue.
**            WARNING! Do not add the same MeshNode twice
**            between renders.
**/
LvError RendererImp::AddMesh(LvrMeshNode *ptMesh)
{
  if(!ptMesh)
    return LV_BAD_INPUT;
  
  LvrMeshNode **ppNode  = &m_headMeshMode;
  LvrMeshNode  *pN      = 0;
  while(*ppNode)
  {
    if(ptMesh->material < (*ppNode)->material)
      ppNode  = &(*ppNode)->_pL;
    else if(ptMesh->material > (*ppNode)->material)
      ppNode  = &(*ppNode)->_pR;
    else //(ptMesh->material == (*ppNode)->material)
    {
      pN = (*ppNode)->_pN;
      ppNode  = &(*ppNode)->_pN;
      break;
    }
  }
  
  *ppNode = ptMesh;
  ptMesh->_pL = 0;
  ptMesh->_pR = 0;
  ptMesh->_pN = pN;
  
  return LV_OKAY;
}

/*!
**  In:       pLight  - Light descriptor.
**
**  Return:   Error code.
**
**  Notes:    Add a global light to the render queue.
**            WARNING! Do not add the same Light twice
**            between Render calls.
**/
LvError RendererImp::AddLight(LvrLight *pLight)
{
  LvError error = LV_IGNORED;
  
  LV_ASSERT(m_numLights <= LVR_MAX_LIGHTS);
  
  if(pLight && m_numLights < LVR_MAX_LIGHTS)
  {
    m_lights[m_numLights++] = pLight;
    error = LV_OKAY;
  }
  
  return error;
}

/*!
**  In:       eState  - Global renderer state.
**
**  Notes:    Enable a global renderer state.
**/
void RendererImp::EnableState(LvrRendererStateE eState)
{
  if(eState < 0 || eState >= LVR_NUM_RENDERER_STATES)
    return;
  m_states[eState] = LV_TRUE;
}

/*!
**  In:       eState  - Global renderer state.
**
**  Notes:    Disable a global renderer state.
**/
void RendererImp::DisableState(LvrRendererStateE eState)
{
  if(eState < 0 || eState >= LVR_NUM_RENDERER_STATES)
    return;
  m_states[eState] = LV_FALSE;
}

/*!
**  In:       eState  - Global renderer state.
**
**  Return:   LV_TRUE if the state is enabled.
**
**  Notes:    Gets the current state of a renderer state.
**/
LvBool RendererImp::GetState(LvrRendererStateE eState)
{
  if(eState < 0 || eState >= LVR_NUM_RENDERER_STATES)
    return LV_FALSE;
  return m_states[eState];
}

/*!
**  In:       desc - fog descriptor.
**
**  Notes:    Sets the fog descriptor.
**/
void RendererImp::SetFogDesc(const LvrFogDesc &desc)
{
  m_fog = desc;
}

/*!
**  In:       path  - path to texture file.
**            desc  - texture descriptor.
**
**  Return:   Instance of a texture, or 0 for error.
**
**  Notes:    Gets an instance of a texture and loads from
**            file if needed.
**/
LvrTexture *RendererImp::GetTextureFromFile(const LvChar *path, const LvrTextureDesc &desc)
{
  Texture *node = m_textureManager.GetTexture(path, desc);
  return node;
}

/*!
**  In:       data - texture data.
**            desc - texture descriptor.
**
**  Return:   Instance of a texture, or 0 for error.
**
**  Notes:    Creates a texture from data already in memory.
**/
LvrTexture *RendererImp::GetTextureFromMemory(const LvrTextureData &data, const LvrTextureDesc &desc)
{
  LV_ASSERT(!"TODO!");
  return 0;
}

/*!
**  In:       files - paths to texture files.
**            desc  - texture descriptor.
**
**  Return:   Instance of a texture, or 0 for error.
**
**  Notes:    Gets an instance of a texture and loads from
**            file if needed.
**/
LvrTexture *RendererImp::GetCubeMapFromFiles(const LvrCubeMapFiles &files, const LvrTextureDesc &desc)
{
  Texture *node = m_textureManager.GetCubeMap(files, desc);
  return node;
}

/*!
**  In:       texture - the texture to return.
**
**  Notes:    Return a texture after its finished being used.
**/
void RendererImp::ReturnTexture(LvrTexture *texture)
{
  m_textureManager.ReturnTexture((Texture*)texture);
}

/*!
**  In:       desc - shader descriptor.
**
**  Return:   Instance of a shader, or 0 for error.
**
**  Notes:    Loads a shader.
**/
LvrShader *RendererImp::GetShader(const LvrShaderDesc &desc)
{
  return m_shaderManager.GetShaderFromFile(desc.path, desc.profile);
}

/*!
**  In:       shader - the shader to return.
**
**  Notes:    Return a shader after its finished being used.
**/
void RendererImp::ReturnShader(LvrShader *shader)
{
  Shader *internalShader = (Shader*)shader;
  m_shaderManager.ReturnShader(internalShader);
}

/*!
**  In:       tDesc - A material descriptor.
**
**  Return:   Instance of a material or 0 for error.
**
**  Notes:    Creates an instance of a material.
**/
LvrMaterial *RendererImp::CreateMaterial(const LvrMaterialDesc &tDesc)
{
  return m_materialManager.CreateMaterial(tDesc);
}

/*!
**  In:       pMaterial - Material Instance.
**
**  Notes:    Destroys a material.
**/
void RendererImp::ReleaseMaterial(LvrMaterial *pMaterial)
{
  m_materialManager.ReleaseMaterial(pMaterial);
}

/*!
**  In:       pMeshNode - Current Mesh Node.
**
**  Notes:    Traverses the render queue.
**/
void RendererImp::_Traverse(LvrMeshNode *pMeshNode)
{
  LV_ASSERT(pMeshNode);
  
  if(pMeshNode->_pL)
    _Traverse(pMeshNode->_pL);
    
  for(LvrMeshNode *pCurrNode=pMeshNode; pCurrNode; pCurrNode=pCurrNode->_pN)
  {
    _RenderNode(pCurrNode);
  }
    
  if(pMeshNode->_pR)
    _Traverse(pMeshNode->_pR);
}




