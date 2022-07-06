/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Direct3D Static mesh container.
*/

#ifndef D3DSTATICMESH_H
#define D3DSTATICMESH_H

#include "Direct3DRenderer.h"

class D3DStaticMesh : public LvrStaticMesh
{
  public:
    D3DStaticMesh(LvAllocator &allocator, IDirect3DDevice9 *device);
    ~D3DStaticMesh(void);
    
  public:
    /*!
    **  In:       dynmesh  - Dynamic Mesh (our descriptor).
    **
    **  Return:   Error code.
    **
    **  Notes:    Uploads the mesh to the static container.
    **/
    LvError Upload(const LvrDynamicMesh &dynmesh);
    
    /*!
    **  Notes:    Free static mesh from Memory.
    **/
    void Free(void);
    
    /*!
    **  Notes:    Render static mesh.
    **/
    void Render(void) const;
  
  private:
    LvAllocator            &m_allocator;
    IDirect3DDevice9       *m_d3dDevice;
    LvUInt32                m_vertSize;
    LvUInt32                m_numVerts;
    DWORD                   m_vertFormat;
    IDirect3DVertexBuffer9 *m_vertexBuffer;
    IDirect3DIndexBuffer9  *m_indexBuffer;
    D3DPRIMITIVETYPE        m_primitives;
    LvUInt32                m_primitiveCount;
};

#endif
