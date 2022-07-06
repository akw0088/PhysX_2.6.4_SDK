/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    
*/

// DEV NOTES:
//
// -  The D3D Renderer is so broke ass right now, someone
//    needs to "own" it. -jd

#include "Direct3DRenderer.h"
#include "D3DStaticMesh.h"
#include "HLSLShader.h"
#include "D3DMaterialPass.h"

// Don't Compile D3D Support if we are on the wrong platform.
#if defined(LV_WINDOWS) || defined(LV_XENON)

// Direct3D Function Pointer Types.
typedef IDirect3D9* (WINAPI* LPDIRECT3DCREATE9)(UINT SDKVersion);

static D3DMATRIX g_d3dIdentityMatrix =
{
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1
};

D3DPRIMITIVETYPE g_d3dPrimitives[] =
{
  D3DPT_POINTLIST,
  D3DPT_LINELIST,
  D3DPT_LINESTRIP,
  D3DPT_TRIANGLELIST,
  D3DPT_TRIANGLESTRIP,
  D3DPT_TRIANGLEFAN,
};

// Convert from a Lava matrix to a Direct3D Matrix.
// With clever projection matrix haxx0ring you actually don't need
// to convert from left-hand to right hand and swap columns/rows it seems.
static void LavaToD3DMatrix(D3DMATRIX &d3dMatrix, const LvMatrix44 &lavaMatrix)
{
  memcpy(&d3dMatrix, &lavaMatrix, sizeof(lavaMatrix));
}

/*!
**  In:       mesh - Mesh to calculate the size of.
**
**  Notes:    Calculates the size of a single vertex.
**/
LvUInt32 LVD3D_CalcVertexSize(const LvrDynamicMesh &mesh)
{
  LvUInt32 size = 0;
  LV_ASSERT(mesh.positions.numVerts);
  LV_ASSERT(mesh.positions.vertSize==3);
  if(mesh.positions.numVerts && mesh.positions.vertSize == 3)
  {
    size += 3 * sizeof(LvReal);
  }
  if(mesh.normals.numVerts && mesh.normals.vertSize == 3)
  {
    size += 3 * sizeof(LvReal);
  }
  if(mesh.colors.numVerts)
  {
    size += sizeof(DWORD);
  }
  for(LvUInt32 i=0; i<mesh.numTexCoords; i++)
  {
    if(mesh.texcoords[i].numVerts)
    {
      size += mesh.texcoords[i].vertSize * sizeof(LvReal);
    }
  }
  return size;
}


/*!
**  Notes:    Constructor.
**/
Direct3DRenderer::Direct3DRenderer(LvAllocator &allocator, LvUserIO &userio) : RendererImp(allocator, userio)
{
  m_library         = 0;
  m_d3d             = 0;
  m_d3dDevice       = 0;
  m_vertexCache     = 0;
  m_vertexCacheSize = 0;
}

/*!
**  Notes:    Destructor.
**/
Direct3DRenderer::~Direct3DRenderer(void)
{
  if(m_vertexCache) LV_FREE(m_allocator, m_vertexCache);
}

/*!
**  In:       lavaDesc - Initialization Data.
**
**  Return:   Error code.
**
**  Notes:    Initialize the Renderer.
**/
LvError Direct3DRenderer::Init(const LvLavaDesc &lavaDesc)
{
  LvError ret = LV_OKAY;
  
  // Initialize our parent!
  ret = RendererImp::Init(lavaDesc);

  LV_ASSERT(!m_library);
  
  m_library = 0;
  m_d3d     = 0;
  
  m_library = LoadLibraryA("d3d9.dll");
  LV_ASSERT(m_library);
  
  if(!m_library)
    ret = LV_BAD_OUTPUT;
  else
  {
    LPDIRECT3DCREATE9 pDirect3DCreate9 = (LPDIRECT3DCREATE9)GetProcAddress(m_library, "Direct3DCreate9");
    LV_ASSERT(pDirect3DCreate9);
    
    if(pDirect3DCreate9) m_d3d = pDirect3DCreate9(D3D_SDK_VERSION);
    
    if(!m_d3d)
      ret = LV_BAD_OUTPUT;
    else
    {
      D3DPRESENT_PARAMETERS params  = {0};
      params.Windowed               = TRUE;
      params.SwapEffect             = D3DSWAPEFFECT_DISCARD;
      params.BackBufferFormat       = D3DFMT_UNKNOWN;
      params.EnableAutoDepthStencil = TRUE;
      params.AutoDepthStencilFormat = D3DFMT_D16;
      
      HRESULT hResult = m_d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                             lavaDesc.hWnd,
                                             D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                             &params, &m_d3dDevice);
      LV_ASSERT(hResult == D3D_OK);
      if(hResult != D3D_OK)
        ret = LV_BAD_OUTPUT;
      else
      {
        // set out initial states...
        m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
      }
    }
  }
  
  LV_ASSERT(ret==LV_OKAY);
  return ret;
}

/*!
**  Return:   Error code.
**
**  Notes:    Shutdown the Renderer.
**/
LvError Direct3DRenderer::Shutdown(void)
{
  LvError ret = LV_OKAY;
  
  if(m_d3dDevice)
  {
    m_d3dDevice->Release();
    m_d3dDevice = 0;
  }
  
  if(m_d3d)
  {
    m_d3d->Release();
    m_d3d = 0;
  }
  
  if(m_library)
  {
    FreeLibrary(m_library);
    m_library = 0;
  }
  
  return ret;
}

/*!
**  In:       caps - capabilities descriptor.
**
**  Return:   Error code.
**
**  Notes:    Get the renderer's capabilities.
**/
LvError Direct3DRenderer::GetCaps(LvrRendererCaps &caps)
{
  LV_ASSERT(!"TODO");
  return LV_IGNORED;
}

/*!
**  In:       iWidth  - Width of Display region.
**            iHeight - Height of Display region.
**
**  Return:   Error code.
**
**  Notes:    Signal the Renderer that the display region resized.
**/
LvError Direct3DRenderer::Resized(LvUInt32 iWidth, LvUInt32 iHeight)
{
  LvError error = LV_IGNORED;
  
  if(m_d3dDevice && 0)
  {
    D3DVIEWPORT9 tViewport = {0};
    m_d3dDevice->GetViewport(&tViewport);
    tViewport.Width  = iWidth;
    tViewport.Height = iHeight;

    HRESULT hResult = m_d3dDevice->SetViewport(&tViewport);
    LV_ASSERT(hResult == D3D_OK);
    
    if(hResult == D3D_OK)
      error = LV_OKAY;
    error = LV_BAD_OUTPUT;
  }
  
  return error;
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
LvError Direct3DRenderer::Render(const LvMatrix44 &mProjection, const LvMatrix44 &mView, LvrRenderTarget *pTarget)
{
  D3DMATRIX d3dMatrix;
  HRESULT hResult = D3D_OK;
  
  // Set Projection Matrix.
  LvMatrix44 proj(mProjection);
  //proj.z.w *= -1;
  //proj.w.z *= -0.5f;
  LavaToD3DMatrix(d3dMatrix, proj);
  m_d3dDevice->SetTransform(D3DTS_PROJECTION, &d3dMatrix);
  
  // Set View Matrix.
  LavaToD3DMatrix(d3dMatrix, mView);
  m_d3dDevice->SetTransform(D3DTS_VIEW, &d3dMatrix);
  
  m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  //m_d3dDevice->SetRenderState(D3DRS_ZENABLE,  D3DZB_TRUE);
  
  // Start the D3D "Scene".
  hResult = m_d3dDevice->BeginScene();
  LV_ASSERT(hResult == D3D_OK);
  
  if(hResult == D3D_OK)
  {
    m_materialPass = MAT_DEFAULT_PASS;
    
    // Traverse the render queue and render its contents.
    _Traverse();
    
    // Stop the D3D "Scene".
    hResult = m_d3dDevice->EndScene();
    LV_ASSERT(hResult == D3D_OK);
  }
  
  // Clear the render queue.
  _ClearRenderQueue();
  
  return LV_IGNORED;
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
LvrStaticMesh *Direct3DRenderer::CreateStaticMesh(const LvrDynamicMesh &desc)
{
  D3DStaticMesh *staticmesh = LV_NEW(m_allocator, D3DStaticMesh) D3DStaticMesh(m_allocator, m_d3dDevice);
  LvError error = staticmesh->Upload(desc);
  if(error != LV_OKAY)
  {
    LV_DELETE(m_allocator, D3DStaticMesh, staticmesh);
    staticmesh = 0;
  }
  return staticmesh;
}

/*!
**  In:       pMesh - Static mesh to destroy
**
**  Return:   Error code.
**
**  Notes:    Destroy a Static mesh freeing it from any memory
**            it may be present in.
**/
LvError Direct3DRenderer::ReleaseStaticMesh(LvrStaticMesh *pMesh)
{
  LvError error = LV_IGNORED;
  D3DStaticMesh *staticmesh = (D3DStaticMesh*)pMesh;
  if(staticmesh)
  {
    staticmesh->Free();
    LV_DELETE(m_allocator, D3DStaticMesh, staticmesh);
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
LvrRenderTarget *Direct3DRenderer::CreateRenderTarget(const LvrRenderTargetDesc &desc)
{
  //LV_ASSERT(0);
  return 0;
}

/*!
**  In:       pTarget - Render Target to destroy.
**
**  Return:   Error code.
**
**  Notes:    Destroy a Render Target.
**/
LvError Direct3DRenderer::ReleaseRenderTarget(LvrRenderTarget *pTarget)
{
  LV_ASSERT(0);
  return LV_IGNORED;
}

/*!
**  Notes:    Swap the rendering buffers
**/
void Direct3DRenderer::SwapBuffers(void)
{
  LV_ASSERT(m_d3dDevice);
  if(m_d3dDevice)
  {
    HRESULT hResult = D3D_OK;
    
    hResult = m_d3dDevice->Present(NULL, NULL, NULL, NULL);
    LV_ASSERT(hResult == D3D_OK);
    
    // Clear to black.
    hResult = m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 0.0f, 0);
    LV_ASSERT(hResult == D3D_OK);
  }
}

/*!
**  Return:   Material pass instance.
**
**  Notes:    Creates a new material pass.
**/
MaterialPass *Direct3DRenderer::CreateMaterialPass(void)
{
  D3DMaterialPass *pass = LV_NEW(m_allocator, D3DMaterialPass) D3DMaterialPass(*this, m_d3dDevice);
  return pass;
}

/*!
**  Notes:    Removes a meterial pass from memory.
**/
void Direct3DRenderer::ReleaseMaterialPass(MaterialPass *pass)
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
Shader *Direct3DRenderer::CreateShader(const LvChar *pcPath, LvrShaderProfileE profile)
{
  return 0;
}

/*!
**  Notes:    Removes a shader from memory.
**/
void Direct3DRenderer::ReleaseShader(Shader *shader)
{

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
LvError Direct3DRenderer::UploadTexture(LvGUID &guid, const LvrTextureData &data, const LvrTextureDesc &desc)
{
  LvError ret = LV_OKAY;
  IDirect3DTexture9 *texture;
  D3DFORMAT format;
  switch(data.m_depth)
  {
    case 1: format = D3DFMT_L8;       break;
    case 3: format = D3DFMT_X8R8G8B8; break;
    case 4: format = D3DFMT_A8R8G8B8; break;
    default: ret = LV_BAD_INPUT;
  }
  if(ret == LV_OKAY)
  {
    UINT levels = desc.m_mipmap ? 0 : 1;
    DWORD usage = desc.m_mipmap ? D3DUSAGE_AUTOGENMIPMAP : 0;
  
    HRESULT hResult = m_d3dDevice->CreateTexture(data.m_width,  data.m_height,
                                                 levels,        usage,
                                                 format,        D3DPOOL_MANAGED,
                                                 &texture,      0);
    if(SUCCEEDED(hResult))
    {
      D3DLOCKED_RECT lockedrect = {0};
      texture->LockRect(0, &lockedrect, 0, D3DLOCK_DISCARD);
      LvUInt8 *bits = (LvUInt8*)lockedrect.pBits;
      if(bits)
      {
        LvUInt32 numPixels = data.m_width * data.m_height;
        if(format == D3DFMT_L8)
        {
          memcpy(bits, data.m_data, numPixels);
        }
        else
        {
          for(LvUInt32 p=0; p<numPixels; p++)
          {
            LvUInt8 *dstpixel = &bits[p*4];
            LvUInt8 *srcpixel = &data.m_data[p*data.m_depth];
            for(LvUInt32 c=0; c<data.m_depth; c++)
              dstpixel[c] = srcpixel[data.m_depth-1-c];
          }
        }
      }
      texture->UnlockRect(0);
    }
    
    if(SUCCEEDED(hResult)) guid = texture;
    else                   ret  = LV_BAD_OUTPUT;
  }
  return ret;
}

/*!
**  In:       gGUID     - Texture identifier.
**
**  Return:   Error code.
**
**  Notes:    Deletes a texture from the renderer (likely video card).
**/
LvError Direct3DRenderer::DeleteTexture(LvGUID &gGUID)
{
  LvError ret = LV_IGNORED;
  IDirect3DTexture9 *texture = (IDirect3DTexture9*)gGUID;
  if(texture)
  {
    texture->Release();
    ret = LV_OKAY;
  }
  return ret;
}

/*!
**  In:       pMeshNode - Current Mesh Node.
**
**  Notes:    Render a Mesh Node.
**/
void Direct3DRenderer::_RenderNode(LvrMeshNode *pMeshNode)
{
  LV_ASSERT(pMeshNode);

  // Are we going to apply a transform to this Mesh?
  LvBool bTransform = pMeshNode->flags & LVR_NO_TRANSFORM || !pMeshNode->transforms ? LV_FALSE : LV_TRUE;
  
  if(bTransform)
  {
    D3DMATRIX d3dTransform;
    LavaToD3DMatrix(d3dTransform, pMeshNode->transforms[0]);
    m_d3dDevice->SetTransform(D3DTS_WORLD, &d3dTransform);
  }
  
  // Are we going to apply a material to this Mesh?
  LvBool bMaterial = pMeshNode->flags & LVR_NO_MATERIAL || !GetState(LVR_MATERIALS) || !pMeshNode->material ? LV_FALSE : LV_TRUE;

  // Set the material.
  if(bMaterial)
    m_materialManager.Begin(m_materialPass, pMeshNode->material);
  else
    m_materialManager.End();
  
  LV_ASSERT(pMeshNode->mesh);
  
  if(pMeshNode->mesh)
    _RenderMesh(pMeshNode->mesh);
  
  if(bTransform)
  {
    m_d3dDevice->SetTransform(D3DTS_WORLD, &g_d3dIdentityMatrix);
  }
}

/*!
**  In:       pMesh - Mesh to render.
**
**  Notes:    Render a Mesh.
**/
void Direct3DRenderer::_RenderMesh(const LvrMesh *mesh)
{
  if(mesh->eType == LVR_MESH_STATIC)
  {
    const D3DStaticMesh *staticmesh = (const D3DStaticMesh*)mesh;
    staticmesh->Render();
  }
  else if(mesh->eType == LVR_MESH_DYNAMIC)
  {
    const LvrDynamicMesh &dynmesh = *(LvrDynamicMesh*)mesh;
    LV_ASSERT(dynmesh.positions.numVerts);
    LvUInt32 vertSize  = LVD3D_CalcVertexSize(dynmesh);
    LvUInt32 numVerts  = dynmesh.positions.numVerts;
    LvUInt32 cacheSize = vertSize * numVerts;
    _ResizeVertexCache(cacheSize);
    LV_ASSERT(m_vertexCache);
    if(m_vertexCache && cacheSize)
    {
      LvUInt8 *cacheStart   = m_vertexCache;
      LvUInt32 vertexFormat = 0;
      
      // Load Positions into Vertex Cache.
      if(dynmesh.positions.numVerts && dynmesh.positions.vertSize == 3)
      {
        vertexFormat       |= D3DFVF_XYZ;
        LvUInt32 components = dynmesh.positions.vertSize;
        for(LvUInt32 i=0; i<numVerts; i++)
        {
          LvReal *vstart = (LvReal*)(cacheStart+i*vertSize);
          dynmesh.positions.GetVector(vstart, i);
        }
        cacheStart += sizeof(LvReal)*components;
      }
      
      // Load Normals into Vertex Cache.
      if(dynmesh.normals.numVerts && dynmesh.normals.vertSize == 3)
      {
        vertexFormat       |= D3DFVF_NORMAL;
        LvUInt32 components = dynmesh.normals.vertSize;
        for(LvUInt32 i=0; i<numVerts; i++)
        {
          LvReal *vstart = (LvReal*)(cacheStart+i*vertSize);
          dynmesh.normals.GetVector(vstart, i);
        }
        cacheStart += sizeof(LvReal)*components;
      }
      
      // Load Diffuse Color into Vertex Cache.
      if(dynmesh.colors.numVerts)
      {
        vertexFormat       |= D3DFVF_DIFFUSE;
        LvUInt32 components = dynmesh.colors.vertSize;
        for(LvUInt32 i=0; i<numVerts; i++)
        {
          LvUInt8 *vstart = (LvUInt8*)(cacheStart+i*vertSize);
          LvReal vec[4] = {0};
          dynmesh.colors.GetVector(vec, i);
          for(LvUInt32 j=0; j<4; j++)
          {
            vstart[j] = (LvUInt8)(vec[j]*255);
          }
        }
        cacheStart += sizeof(DWORD);
      }
      
      // Load Texture Coords into Vertex Cache.
      for(LvUInt32 tc=0; tc<dynmesh.numTexCoords; tc++)
      {
        vertexFormat       |= D3DFVF_TEX1+tc*D3DFVF_TEX1;
        LvUInt32 components = dynmesh.texcoords[tc].vertSize;
        LV_ASSERT(components <= 4);
        for(LvUInt32 i=0; i<numVerts; i++)
        {
          LvReal *vstart = (LvReal*)(cacheStart+i*vertSize);
          dynmesh.texcoords[tc].GetVector(vstart, i);
        }
        cacheStart += sizeof(LvReal)*components;
      }

      LvUInt32 numPrimitives = numVerts;
      if(dynmesh.numIndices) numPrimitives = dynmesh.numIndices;

      switch(dynmesh.primitives)
      {
        case LVR_POINTS:                              break;
        case LVR_LINES:           numPrimitives /= 2; break;
        case LVR_LINE_STRIP:      numPrimitives -= 1; break;
        case LVR_TRIANGLES:       numPrimitives /= 3; break;
        case LVR_TRIANGLE_STRIP:  numPrimitives -= 2; break;
        case LVR_TRIANGLE_FAN:    numPrimitives -= 2; break;
        case LVR_POINT_SPRITES:                       break;
      }
      
      m_d3dDevice->SetFVF(vertexFormat);
      if(dynmesh.indices16)
      {
        m_d3dDevice->DrawIndexedPrimitiveUP(g_d3dPrimitives[dynmesh.primitives],
                                            0, numVerts, numPrimitives,
                                            dynmesh.indices16, D3DFMT_INDEX16,
                                            m_vertexCache,     vertSize );
      }
      else if(dynmesh.indices32)
      {
        m_d3dDevice->DrawIndexedPrimitiveUP(g_d3dPrimitives[dynmesh.primitives],
                                            0, numVerts, numPrimitives,
                                            dynmesh.indices32, D3DFMT_INDEX32,
                                            m_vertexCache,     vertSize );
      }
      else
      {
        m_d3dDevice->DrawPrimitiveUP(g_d3dPrimitives[dynmesh.primitives],
                                     numPrimitives, m_vertexCache, vertSize);
      }
    }
  }
}

/*!
**  In:       size - size needed in the vertex cache.
**
**  Notes:    Assures the vertex cache is a minimal size.
**/
void Direct3DRenderer::_ResizeVertexCache(LvUInt32 size)
{
  if(m_vertexCacheSize < size)
  {
    LvUInt8 *vertexCache = (LvUInt8*)LV_MALLOC(m_allocator, size);
    if(vertexCache)
    {
      if(m_vertexCacheSize)
      {
        memcpy(vertexCache, m_vertexCache, m_vertexCacheSize);
      }
      m_vertexCacheSize = size;
      if(m_vertexCache)
      {
        LV_FREE(m_allocator, m_vertexCache);
      }
      m_vertexCache = vertexCache;
    }
  }
}

#endif
