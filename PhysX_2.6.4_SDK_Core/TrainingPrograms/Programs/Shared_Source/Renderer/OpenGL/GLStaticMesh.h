/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Static Mesh Implementation.
*/

#ifndef LV_GLSTATICMESH_H
#define LV_GLSTATICMESH_H

#include <Lava/LvLava.h>
#include <Lava/Renderer/LvRenderer.h>

#if defined(LV_WINDOWS)
  #include "OpenGLExtensions/OpenGLExtensions.h"
  #define USE_DRAW_LISTS 1
  #define VBO_OK 1
#elif defined(LV_APPLE)
  #include <AGL/agl.h>
  #include <OpenGL/glext.h>
  #define USE_DRAW_LISTS 1
#elif defined(LV_PLAYSTATION3)
  #include <JS/js.h>
  #include <JS/jsu.h>
  #include <GLES/glext.h>
#else
#error static or dynamic linked?
#endif

/*!
**  Notes:    OpenGL Static Mesh Implementation.
**/
class GLStaticMesh : public LvrStaticMesh
{
  public:
    /*!
    **  Notes:    Constructor.
    **/
    GLStaticMesh(LvAllocator &allocator);
    
    /*!
    **  Notes:    Destructor.
    **/
    ~GLStaticMesh(void);
    
  public:
    /*!
    **  In:       dynmesh  - Dynamic Mesh (our descriptor.
    **
    **  Return:   Error code.
    **
    **  Notes:    OpenGL Static Mesh Implementation.
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
    LvAllocator    &m_allocator;
    
  #if USE_DRAW_LISTS
    GLuint          m_drawlist;
  #else
    GLuint          m_uiVerts;
    GLuint          m_uiNorms;
    GLuint          m_uiColors;
    GLuint         *m_uiTexCoords;
    LvUInt32        m_uiNumTexCoords;
    LvUInt32        m_uiNumVerts;
    
    LvUInt32       *m_pIndices32;
    LvUInt16       *m_pIndices16;
    LvUInt32        m_uiNumIndices;
    
    LvrPrimitivesE  m_ePrimitives;
  #endif
};

#endif
