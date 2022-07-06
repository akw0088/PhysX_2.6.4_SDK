/*!
**  Author:   James Dolan (jdolan@gmail.com)
**            Taylor White (taylorcwhite@gmail.com)
**
**  Notes:    Cg Language implementation.
*/

#ifndef LV__CGGLSHADER_H
#define LV__CGGLSHADER_H

#include "Renderer/MaterialManager/Shader.h"

#if defined(LV_WINDOWS)
  #include "OpenGLExtensions/OpenGLExtensions.h"
  #include <Cg/cg.h>
  #include <Cg/cgGL.h>
  #define CGGL_OK 1

#elif defined(LV_APPLE)
  #include <AGL/agl.h>
  #include <OpenGL/glext.h>
  #include <Cg/cg.h>
  #include <Cg/cgGL.h>
  #define CGGL_OK 1

#elif defined(LV_PLAYSTATION3)
  #include <JS/js.h>
  #include <JS/jsu.h>
  #include <GLES/glext.h>
  #include <Cg/cgGL.h>
  #define CGGL_OK 1

#else
  #error static or dynamic linked?
#endif

#if CGGL_OK

/*!
**  Notes:    Cg Language implementation.
**/
class CGGLShader : public Shader
{
  public:
    CGGLShader(LvAllocator &allocator, LvUserIO &userio, CGcontext context);
    
    virtual ~CGGLShader(void);
    
    /*!
    **  In:       pcPath  - Path to shader.
    **            profile - Profile to use.
    **
    **  Return:   Error code.
    **
    **  Notes:    Load Cg Shader from File.
    **/
    LvError LoadFromFile(const LvChar *pcPath, CGprofile profile);
    
  public:
    /*!
    **  Notes:    Starts the use of this shader.
    **/
    virtual void Begin(void);
    
    /*!
    **  Notes:    End the use of this shader.
    **/
    virtual void End(void);
  
  public:
    virtual LvGUID GetVariable(const LvChar *pcName);
    virtual LvError SetReal(LvGUID gVar, LvReal fReal);
    virtual LvError SetVector2(LvGUID gVar, const LvVector2 &vec);
    virtual LvError SetVector3(LvGUID gVar, const LvVector3 &vec);
    virtual LvError SetVector4(LvGUID gVar, const LvVector4 &vec);
    virtual LvError SetStateMatrix(LvGUID gVar, const LvUInt8 state);
    
  private:
    
  
  private:
    CGcontext m_context;
    CGprofile m_profile;
    CGprogram m_program;
};

#endif

#endif
