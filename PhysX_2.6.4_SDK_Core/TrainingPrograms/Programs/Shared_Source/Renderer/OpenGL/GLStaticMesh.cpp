/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Static Mesh Implementation.
*/

#include "GLStaticMesh.h"

#define STATIC_MESH_MODE GL_STATIC_DRAW_ARB
//#define STATIC_MESH_MODE GL_DYNAMIC_DRAW_ARB

extern GLuint g_vuiGLPrimitives[];

/*!
**  Notes:    Constructor.
**/
GLStaticMesh::GLStaticMesh(LvAllocator &allocator) : m_allocator(allocator)
{
#if USE_DRAW_LISTS
  m_drawlist        = 0;
#else
  m_uiVerts         = 0;
  m_uiNorms         = 0;
  m_uiColors        = 0;
  m_uiTexCoords     = 0;
  m_uiNumTexCoords  = 0;
  m_uiNumVerts      = 0;
  m_pIndices32      = 0;
  m_pIndices16      = 0;
  m_uiNumIndices    = 0;
  m_ePrimitives     = LVR_TRIANGLES;
#endif
}

/*!
**  Notes:    Destructor.
**/
GLStaticMesh::~GLStaticMesh(void)
{
  Free();
}

/*!
**  In:       dynmesh  - Dynamic Mesh (our descriptor.
**
**  Return:   Error code.
**
**  Notes:    OpenGL Static Mesh Implementation.
**/
LvError GLStaticMesh::Upload(const LvrDynamicMesh &dynmesh)
{
  LvError error = LV_IGNORED;
  
  const LvrDynamicMesh *pDynMesh = &dynmesh;

#if USE_DRAW_LISTS
  Free();
  m_drawlist = glGenLists(1);
  if(m_drawlist)
  {
    error = LV_OKAY;
    glNewList(m_drawlist, GL_COMPILE);
    
    // We require vertex data.
    LV_ASSERT(pDynMesh->positions.buffer && pDynMesh->positions.numVerts);
    
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
        glActiveTextureARB(GL_TEXTURE0_ARB+i);
        glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(pDynMesh->texcoords[i].vertSize, GL_FLOAT, pDynMesh->texcoords[i].stride, pDynMesh->texcoords[i].buffer);
      }
    }
    
    if(pDynMesh->colors.buffer)
    {
      glEnableClientState(GL_VERTEX_ARRAY);
      glColorPointer(pDynMesh->colors.vertSize, GL_FLOAT, pDynMesh->colors.stride, pDynMesh->colors.buffer);
    }
    
    // We can't have 32 and 16 bit indices!
    LV_ASSERT(!pDynMesh->indices32 || !pDynMesh->indices16);
    
    if(pDynMesh->indices32)
    {
      glDrawElements( g_vuiGLPrimitives[pDynMesh->primitives], pDynMesh->numIndices,
                      GL_UNSIGNED_INT, pDynMesh->indices32);
    }
    else if(pDynMesh->indices16)
    {
      glDrawElements( g_vuiGLPrimitives[pDynMesh->primitives], pDynMesh->numIndices,
                      GL_UNSIGNED_SHORT, pDynMesh->indices16);
    }
    else
    {
      glDrawArrays(g_vuiGLPrimitives[pDynMesh->primitives], 0, pDynMesh->positions.numVerts);
    }
    
    if(pDynMesh->colors.buffer)
      glDisableClientState(GL_COLOR_ARRAY);
    
    for(LvUInt32 i=0; i<pDynMesh->numTexCoords; i++)
    {
      glActiveTextureARB(GL_TEXTURE0_ARB+i);
      glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    if(pDynMesh->normals.buffer)
      glDisableClientState(GL_NORMAL_ARRAY);
    
    if(pDynMesh->positions.buffer)
      glDisableClientState(GL_VERTEX_ARRAY);
    
    glEndList();
  }
  
#elif VBO_OK
  
  LV_ASSERT(glGenBuffersARB && glBindBufferARB && glBufferDataARB);
  
  m_uiVerts         = 0;
  m_uiNorms         = 0;
  m_uiColors        = 0;
  m_uiTexCoords     = 0;
  m_uiNumVerts      = 0;
  m_pIndices32      = 0;
  m_pIndices16      = 0;
  m_uiNumIndices    = 0;
  m_ePrimitives     = LVR_TRIANGLES;
  
  // Upload Verts.
  if(pDynMesh->positions.buffer)
  {
    glGenBuffersARB(1, &m_uiVerts);
    m_uiNumVerts = pDynMesh->positions.numVerts;
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiVerts);
    
    LvVector3 *pVerts;
    LV_NEW_ARRAY(m_allocator, pVerts, LvVector3, m_uiNumVerts);
    for(LvUInt32 i=0; i<m_uiNumVerts; i++)
    {
      pDynMesh->positions.GetVector(&pVerts[i].x, i);
    }
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(LvVector3) * m_uiNumVerts, pVerts, STATIC_MESH_MODE);
    LV_DELETE_ARRAY(m_allocator, LvVector3, pVerts);
  
    // Upload Normals.
    if(pDynMesh->normals.buffer)
    {
      glGenBuffersARB(1, &m_uiNorms);
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiNorms);
      
      LvVector3 *pNorms;
      LV_NEW_ARRAY(m_allocator, pNorms, LvVector3, m_uiNumVerts);
      for(LvUInt32 i=0; i<m_uiNumVerts; i++)
      {
        pDynMesh->normals.GetVector(&pNorms[i].x, i);
      }
      glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(LvVector3) * m_uiNumVerts, pNorms, STATIC_MESH_MODE);
      LV_DELETE_ARRAY(m_allocator, LvVector3, pNorms);
    }
    
    // Upload Colors.
    if(pDynMesh->colors.buffer)
    {
      glGenBuffersARB(1, &m_uiColors);
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiColors);
      
      LvVector4 *pColor;
      LV_NEW_ARRAY(m_allocator, pColor, LvVector4, m_uiNumVerts);
      for(LvUInt32 i=0; i<m_uiNumVerts; i++)
      {
        pDynMesh->colors.GetVector(&pColor[i].x, i);
      }
      glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(LvVector4) * m_uiNumVerts, pColor, STATIC_MESH_MODE);
      LV_DELETE_ARRAY(m_allocator, LvVector4, pColor);
    }
    
    // Upload Texture Coords.
    if(pDynMesh->numTexCoords)
    {
      m_uiNumTexCoords = pDynMesh->numTexCoords;
      m_uiTexCoords    = (LvUInt32*)LV_MALLOC(m_allocator, sizeof(GLuint)*pDynMesh->numTexCoords);
      for(LvUInt32 i=0; i<m_uiNumTexCoords; i++)
      {
        LV_ASSERT(pDynMesh->texcoords[i].buffer);
        
        glGenBuffersARB(1, &m_uiTexCoords[i]);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiTexCoords[i]);
        
        LvVector2 *pTexCoords;
        LV_NEW_ARRAY(m_allocator, pTexCoords, LvVector2, m_uiNumVerts);
        for(LvUInt32 j=0; j<m_uiNumVerts; j++)
        {
          pDynMesh->texcoords[i].GetVector(&pTexCoords[j].x, j);
        }
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(LvVector2) * m_uiNumVerts, pTexCoords, STATIC_MESH_MODE);
        LV_DELETE_ARRAY(m_allocator, LvVector2, pTexCoords);
      }
    }
    
    // Copy Indices.
    if(pDynMesh->indices32)
    {
      m_uiNumIndices = pDynMesh->numIndices;
      if(m_uiNumIndices)
      {
        m_pIndices32 = (LvUInt32*)LV_MALLOC(m_allocator, sizeof(LvUInt32)*m_uiNumIndices);
        memcpy(m_pIndices32, pDynMesh->indices32, sizeof(LvUInt32)*m_uiNumIndices);
      }
    }
    else if(pDynMesh->indices16)
    {
      m_uiNumIndices = pDynMesh->numIndices;
      if(m_uiNumIndices)
      {
        m_pIndices16 = (LvUInt16*)LV_MALLOC(m_allocator, sizeof(LvUInt16)*m_uiNumIndices);
        memcpy(m_pIndices16, pDynMesh->indices16, sizeof(LvUInt16)*m_uiNumIndices);
      }
    }
    
    m_ePrimitives = pDynMesh->primitives;
    error = LV_OKAY;
  }
  
#endif
  
  return error;
}

/*!
**  Notes:    Free static mesh from Memory.
**/
void GLStaticMesh::Free(void)
{
#if USE_DRAW_LISTS
  
  if(m_drawlist)
  {
    glDeleteLists(m_drawlist, 1);
    m_drawlist = 0;
  }
  
#elif VBO_OK

  if(m_uiVerts)     glDeleteBuffersARB(1, &m_uiVerts);
  if(m_uiNorms)     glDeleteBuffersARB(1, &m_uiNorms);
  if(m_uiColors)    glDeleteBuffersARB(1, &m_uiColors);
  
  if(m_uiTexCoords)
  {
    for(LvUInt32 i=0; i<m_uiNumTexCoords; i++)
    {
      if(m_uiTexCoords[i]) glDeleteBuffersARB(1, &m_uiTexCoords[i]);
    }
    LV_FREE(m_allocator, m_uiTexCoords);
  }
  
  if(m_pIndices32)  LV_FREE(m_allocator, m_pIndices32);
  if(m_pIndices16)  LV_FREE(m_allocator, m_pIndices16);

  m_uiVerts         = 0;
  m_uiNorms         = 0;
  m_uiColors        = 0;
  m_uiTexCoords     = 0;
  m_uiNumTexCoords  = 0;
  m_uiNumVerts      = 0;
  m_pIndices32      = 0;
  m_pIndices16      = 0;
  m_uiNumIndices    = 0;
  m_ePrimitives     = LVR_TRIANGLES;
#endif
}

/*!
**  Notes:    Render static mesh.
**/
void GLStaticMesh::Render(void) const
{
#if USE_DRAW_LISTS
  
   glCallLists(1, GL_UNSIGNED_INT, &m_drawlist);
  
#elif VBO_OK
	if(m_uiVerts)
  {
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiVerts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
  }
  
  if(m_uiNorms)
  {
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiNorms);
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);
  }
  
  for(LvUInt32 i=0; i<m_uiNumTexCoords; i++)
  {
    glActiveTextureARB(GL_TEXTURE0_ARB+i);
    glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiTexCoords[i]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
  }
  
  if(m_uiColors)
  {
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiColors);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, 0);
  }
  
  // We can't have 32 and 16 bit indices!
  LV_ASSERT(!m_pIndices32 || !m_pIndices16);
  
  if(m_pIndices32)
  {
    glDrawElements( g_vuiGLPrimitives[m_ePrimitives], m_uiNumIndices,
                    GL_UNSIGNED_INT, m_pIndices32);
  }
  else if(m_pIndices16)
  {
    glDrawElements( g_vuiGLPrimitives[m_ePrimitives], m_uiNumIndices,
                    GL_UNSIGNED_SHORT, m_pIndices16);
  }
  else
  {
    glDrawArrays(g_vuiGLPrimitives[m_ePrimitives], 0, m_uiNumVerts);
  }
  
  if(m_uiColors)
    glDisableClientState(GL_COLOR_ARRAY);
  
  for(LvUInt32 i=0; i<m_uiNumTexCoords; i++)
  {
    glActiveTextureARB(GL_TEXTURE0_ARB+i);
    glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  
  if(m_uiNorms)
    glDisableClientState(GL_NORMAL_ARRAY);
  
  if(m_uiVerts)
    glDisableClientState(GL_VERTEX_ARRAY);
  
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
#endif
}
