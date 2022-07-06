/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Direct3D Static mesh container.
*/

#include "D3DStaticMesh.h"

extern D3DPRIMITIVETYPE g_d3dPrimitives[];

D3DStaticMesh::D3DStaticMesh(LvAllocator &allocator, IDirect3DDevice9 *device) :
m_allocator(allocator)
{
  m_d3dDevice     = device;
  m_vertSize      = 0;
  m_numVerts      = 0;
  m_vertFormat    = 0;
  m_vertexBuffer  = 0;
  m_indexBuffer   = 0;
}

D3DStaticMesh::~D3DStaticMesh(void)
{
  Free();
}

/*!
**  In:       dynmesh  - Dynamic Mesh (our descriptor).
**
**  Return:   Error code.
**
**  Notes:    Uploads the mesh to the static container.
**/
LvError D3DStaticMesh::Upload(const LvrDynamicMesh &dynmesh)
{
  LvError ret = LV_IGNORED;
  Free();
  
  m_primitives = g_d3dPrimitives[dynmesh.primitives];
  
  LvUInt32 vertSize     = LVD3D_CalcVertexSize(dynmesh);
  LvUInt32 meshFlags    = D3DXMESH_MANAGED;
  LvUInt32 numVerts     = dynmesh.positions.numVerts;
  LvUInt32 byteSize     = vertSize * numVerts;
  m_vertFormat = 0;
  m_vertSize   = vertSize;
  if(dynmesh.positions.numVerts && dynmesh.positions.vertSize == 3)
  {
    m_vertFormat       |= D3DFVF_XYZ;
  }
  if(dynmesh.normals.numVerts && dynmesh.normals.vertSize == 3)
  {
    m_vertFormat       |= D3DFVF_NORMAL;
  }
  if(dynmesh.colors.numVerts)
  {
    m_vertFormat       |= D3DFVF_DIFFUSE;
  }
  for(LvUInt32 tc=0; tc<dynmesh.numTexCoords; tc++)
  {
    m_vertFormat       |= D3DFVF_TEX1+tc*D3DFVF_TEX1;
  }
  
  if(dynmesh.indices32)
  {
    meshFlags |= D3DXMESH_32BIT;
  }
  
  HRESULT hResult = m_d3dDevice->CreateVertexBuffer(byteSize,
                                                    0,
                                                    m_vertFormat,
                                                    D3DPOOL_MANAGED,
                                                    &m_vertexBuffer,
                                                    0);
  if(SUCCEEDED(hResult))
  {
    void *vdata = 0;
    hResult = m_vertexBuffer->Lock(0, 0, &vdata, D3DLOCK_DISCARD);
    if(SUCCEEDED(hResult))
    {
      LvUInt8 *cacheStart = (LvUInt8*)vdata;
      // Load Positions into Vertex Cache.
      if(dynmesh.positions.numVerts && dynmesh.positions.vertSize == 3)
      {
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
        LvUInt32 components = dynmesh.texcoords[tc].vertSize;
        LV_ASSERT(components <= 4);
        for(LvUInt32 i=0; i<numVerts; i++)
        {
          LvReal *vstart = (LvReal*)(cacheStart+i*vertSize);
          dynmesh.texcoords[tc].GetVector(vstart, i);
        }
        cacheStart += sizeof(LvReal)*components;
      }
      m_vertexBuffer->Unlock();
      ret = LV_OKAY;
      m_numVerts = numVerts;
      m_primitiveCount = numVerts;
    }
    
    D3DFORMAT indexFormat = D3DFMT_UNKNOWN;
    if(dynmesh.indices16)
    {
      indexFormat = D3DFMT_INDEX16;
    }
    else if(dynmesh.indices32)
    {
      indexFormat = D3DFMT_INDEX32;
    }
    if(indexFormat != D3DFMT_UNKNOWN)
    {
      LvUInt32 indexByteSize = dynmesh.numIndices * (dynmesh.indices16?2:4);
      hResult = m_d3dDevice->CreateIndexBuffer(indexByteSize,
                                               0,
                                               indexFormat,
                                               D3DPOOL_MANAGED,
                                               &m_indexBuffer,
                                               0);
      if(SUCCEEDED(hResult))
      {
        void *indexdata = 0;
        hResult = m_indexBuffer->Lock(0, 0, &indexdata, D3DLOCK_DISCARD);
        if(SUCCEEDED(hResult))
        {
          if(dynmesh.indices16)
          {
            memcpy(indexdata, dynmesh.indices16, indexByteSize);
          }
          else if(dynmesh.indices32)
          {
            memcpy(indexdata, dynmesh.indices32, indexByteSize);
          }
          m_indexBuffer->Unlock();
          m_primitiveCount = dynmesh.numIndices;
        }
      }
      else
      {
        ret = LV_UNKNOWN_ERROR;
      }
    }
  }
  
  switch(dynmesh.primitives)
  {
    case LVR_POINTS:                                 break;
    case LVR_LINES:           m_primitiveCount /= 2; break;
    case LVR_LINE_STRIP:      m_primitiveCount -= 1; break;
    case LVR_TRIANGLES:       m_primitiveCount /= 3; break;
    case LVR_TRIANGLE_STRIP:  m_primitiveCount -= 2; break;
    case LVR_TRIANGLE_FAN:    m_primitiveCount -= 2; break;
    case LVR_POINT_SPRITES:                          break;
  }
  
  if(ret != LV_OKAY) Free();
  
  return ret;
}

/*!
**  Notes:    Free static mesh from Memory.
**/
void D3DStaticMesh::Free(void)
{
  if(m_vertexBuffer)
  {
    m_vertexBuffer->Release();
    m_vertexBuffer = 0;
  }
  if(m_indexBuffer)
  {
    m_indexBuffer->Release();
    m_indexBuffer = 0;
  }
}

/*!
**  Notes:    Render static mesh.
**/
void D3DStaticMesh::Render(void) const
{
  HRESULT hResult = S_OK;
  hResult = m_d3dDevice->SetStreamSource(0, m_vertexBuffer, 0, m_vertSize);
  LV_ASSERT(SUCCEEDED(hResult));
  
  hResult = m_d3dDevice->SetFVF(m_vertFormat);
  LV_ASSERT(SUCCEEDED(hResult));
  
  if(m_indexBuffer)
  {
    hResult = m_d3dDevice->SetIndices(m_indexBuffer);
    LV_ASSERT(SUCCEEDED(hResult));
    hResult = m_d3dDevice->DrawIndexedPrimitive(m_primitives, 0, 0, m_numVerts, 0, m_primitiveCount);
    LV_ASSERT(SUCCEEDED(hResult));
  }
  else
  {
    hResult = m_d3dDevice->DrawPrimitive(m_primitives, 0, m_primitiveCount);
    LV_ASSERT(SUCCEEDED(hResult));
  }
  
  hResult = m_d3dDevice->SetStreamSource(0, 0, 0, 0);
  LV_ASSERT(SUCCEEDED(hResult));
}

