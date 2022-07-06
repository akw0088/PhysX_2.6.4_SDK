/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Material internal interface.
*/

#ifndef LV__MATERIAL_H
#define LV__MATERIAL_H

#include <Lava/LvLava.h>

#include "ShaderManager.h"
#include "TextureManager.h"

typedef enum MaterialPassE
{
  MAT_DEFAULT_PASS = 0,
  MAT_AMBIENT_PASS,
  MAT_DIRECTIONAL_PASS,
  MAT_POINT_PASS,
  MAT_SPOT_PASS,
};

/*!
**  Notes:    Material Pass Interface.
**/
class MaterialPass
{
  public:    
    /*!
    **  Notes:    Starts the use of this material.
    **/
    virtual void Begin(void)=0;
    
    /*!
    **  Notes:    Ends the use of this material.
    **/
    virtual void End(void)=0;

  public:
    MaterialPass(void);
    
    virtual ~MaterialPass(void);
    
    LvError LoadFromDesc(const LvrMaterialPassDesc &desc);
    
    void Free(void);
    
  protected:
    //! Texture units.
    Texture              *m_pTextures[LVR_NUM_TEXTURE_UNITS]; // Array of textures.
    LvUInt32              m_uiNumTextures;                    // Number of Textures in the array.

    //! Blending.
    LvBool                m_bBlending;
    LvrBlendFuncE         m_eBlendSrc;
    LvrBlendFuncE         m_eBlendDest;

    //! Lighting.
    LvBool                m_bLighting;
    LvVector4             m_vAmbientColor;
    LvVector4             m_vSpecularColor;
    LvVector4             m_vDiffuseColor;
    LvVector4             m_vEmissionColor;
    LvReal                m_fShininess;

    //! Shaders.
    Shader               *m_pVertexShader;
    Shader               *m_pFragmentShader;

    //! Shader matricies
    LvGUID                m_ModelViewProjectionMatrixParam;
    LvGUID                m_ModelViewMatrixParam;
    LvGUID                m_ProjectionMatrixParam;
};

/*!
**  Notes:    Node in Material cache List.
**/
class MaterialNode
{
  public:
    MaterialNode(LvAllocator &allocator) : m_allocator(allocator)
    {
      m_name            = 0;
      m_numUsers        = 0;
      m_left            = 0;
      m_right           = 0;
      m_parent          = 0;
      m_defaultPass     = 0;
      m_ambientPass     = 0;
      m_directionalPass = 0;
      m_pointPass       = 0;
      m_spotPass        = 0;
    }
    
    ~MaterialNode(void)
    {
      if(m_name) LV_FREE(m_allocator, m_name);
    }
    
  public:
    LvChar        *m_name;
    LvSInt32       m_numUsers;
    MaterialNode  *m_left;
    MaterialNode  *m_right;
    MaterialNode  *m_parent;
    MaterialPass  *m_defaultPass;
    MaterialPass  *m_ambientPass;
    MaterialPass  *m_directionalPass;
    MaterialPass  *m_pointPass;
    MaterialPass  *m_spotPass;
  
  protected:
    LvAllocator   &m_allocator;
};


/*!
**  Notes:    Internal material interface.
**/
class Material : public LvrMaterial
{
  public:    
    /*!
    **  Notes:    Starts the use of this material.
    **/
    void Begin(MaterialPassE pass);
    
    /*!
    **  Notes:    Ends the use of this material.
    **/
    void End(void);
  
  public:
    Material(MaterialNode *node);
    
  protected:
    MaterialPass  *m_defaultPass;
    MaterialPass  *m_ambientPass;
    MaterialPass  *m_directionalPass;
    MaterialPass  *m_pointPass;
    MaterialPass  *m_spotPass;
    
  private:
    friend class MaterialManagerImp;
    MaterialPass  *m_currentPass;
    MaterialNode  *m_node;
};


#endif
