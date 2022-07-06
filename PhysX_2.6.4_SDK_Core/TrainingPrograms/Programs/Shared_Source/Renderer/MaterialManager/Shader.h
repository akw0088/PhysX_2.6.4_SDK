/*!
**  Author:   James Dolan (jdolan@gmail.com)
**            Taylor White (taylorcwhite@gmail.com)
**
**  Notes:    Interface for a single shader.
*/

#ifndef LV__SHADER_H
#define LV__SHADER_H

#include <Lava/LvLava.h>

// TWHACK: May have to add transforms are well, depending on what you need to do
typedef enum LvrMatrixStateTypeE
{
  LVR_MODELVIEW_PROJECTION_MATRIX = 0,
  LVR_MODELVIEW_MATRIX,
  LVR_PROJECTION_MATRIX,
  LVR_NUM_MATRIX_STATE_TYPES
};

/*!
**  Notes:    Interface for a single shader.
**/
class Shader : public LvrShader
{
  friend class ShaderManager;
  public:
    virtual LvGUID GetVariable(const LvChar *pcName)                 { return 0; }
    virtual LvError SetReal(LvGUID gVar, LvReal fReal)               { return LV_IGNORED; }
    virtual LvError SetVector2(LvGUID gVar, const LvVector2 &vec)    { return LV_IGNORED; }
    virtual LvError SetVector3(LvGUID gVar, const LvVector3 &vec)    { return LV_IGNORED; }
    virtual LvError SetVector4(LvGUID gVar, const LvVector4 &vec)    { return LV_IGNORED; }
    virtual LvError SetStateMatrix(LvGUID gVar, const LvUInt8 state) { return LV_IGNORED; }

    
  public:
    /*!
    **  Notes:    Starts the use of this shader.
    **/
    virtual void Begin(void) = 0;
    
    /*!
    **  Notes:    End the use of this shader.
    **/
    virtual void End(void) = 0;
  
  public:
    Shader(LvAllocator &allocator, LvUserIO &userio) : m_allocator(allocator), m_userio(userio)
    {
      m_pcName    = 0;
      m_iNumUsers = 0;
      m_pNext     = 0;
      m_pPrev     = 0;
    }
    
    virtual ~Shader(void)
    {
      if(m_pcName) LV_FREE(m_allocator, m_pcName);
    }
    
  protected:
    LvAllocator    &m_allocator;
    LvUserIO       &m_userio;
    LvChar         *m_pcName;
    LvSInt32        m_iNumUsers;
    
  private:
    Shader         *m_pNext,
                   *m_pPrev;
    
};

#endif
