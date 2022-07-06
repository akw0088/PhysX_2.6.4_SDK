/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Material internal interface.
*/

#include "../RendererImp.h"

/*****************
** MaterialPass **
*****************/

MaterialPass::MaterialPass(void)
{
  m_uiNumTextures   = 0;
  m_bBlending       = LV_FALSE;
  m_eBlendSrc       = LVR_SRC_ALPHA;
  m_eBlendDest      = LVR_ONE_MINUS_SRC_ALPHA;
  m_bLighting       = LV_TRUE;
  m_vAmbientColor.Set(0, 0, 0, 1);
  m_vDiffuseColor.Set(1, 1, 1, 1);
  m_vSpecularColor.Set(0, 0, 0, 1);
  m_vEmissionColor.Set(0, 0, 0, 1);
  m_fShininess      = 0;

  m_pVertexShader   = 0;
  m_pFragmentShader = 0;

  m_ModelViewProjectionMatrixParam = 0;
  m_ModelViewMatrixParam = 0;
  m_ProjectionMatrixParam = 0;
}

MaterialPass::~MaterialPass(void)
{
  LV_ASSERT(!m_uiNumTextures);
  LV_ASSERT(!m_pVertexShader);
  LV_ASSERT(!m_pFragmentShader);
}

LvError MaterialPass::LoadFromDesc(const LvrMaterialPassDesc &desc)
{
  LvError ret = LV_IGNORED;
  
  m_bBlending       = desc.blending;
  m_eBlendSrc       = desc.blendSrc;
  m_eBlendDest      = desc.blendDest;

  m_bLighting       = desc.lighting;
  m_vAmbientColor   = desc.ambientColor;
  m_vSpecularColor  = desc.specularColor;
  m_vDiffuseColor   = desc.diffuseColor;
  m_vEmissionColor  = desc.emissionColor;
  m_fShininess      = desc.shininess;
  
  m_uiNumTextures    = desc.numTextures;
  memset(m_pTextures, 0, sizeof(m_pTextures));

  if(desc.textures)
  {
    for(LvUInt32 i=0; i<m_uiNumTextures; i++)
    {
      m_pTextures[i] = (Texture*)desc.textures[i];
    }
  }
  
  if(desc.vertexShader)   m_pVertexShader   = (Shader*)desc.vertexShader;
  if(desc.fragmentShader) m_pFragmentShader = (Shader*)desc.fragmentShader;

  // TWHACK: Set up shader params if the shaders exist
  //
  // Also, we may need to change the name of the actual variables to something a bit
  // standardized later on
  if(m_pVertexShader)
  {
    m_ModelViewProjectionMatrixParam  = m_pVertexShader->GetVariable("modelViewProj");
    m_ModelViewMatrixParam            = m_pVertexShader->GetVariable("ModelViewMatrix");
    m_ProjectionMatrixParam           = m_pVertexShader->GetVariable("ProjectionMatrix");
  }

  return ret;
}

void MaterialPass::Free(void)
{
  m_uiNumTextures   = 0;
  m_pVertexShader   = 0;
  m_pFragmentShader = 0;
}


/*************
** Material **
*************/

/*!
**  Notes:    Starts the use of this material.
**/
void Material::Begin(MaterialPassE pass)
{
  MaterialPass *matpass = m_defaultPass;
  
  switch((LvSInt32)pass)
  {
    case MAT_AMBIENT_PASS:      if(m_ambientPass)      matpass = m_ambientPass;     break;
    case MAT_DIRECTIONAL_PASS:  if(m_directionalPass)  matpass = m_directionalPass; break;
    case MAT_POINT_PASS:        if(m_pointPass)        matpass = m_pointPass;       break;
    case MAT_SPOT_PASS:         if(m_spotPass)         matpass = m_spotPass;        break;
  }
  
  if(matpass)
  {
    matpass->Begin();
    m_currentPass = matpass;
  }
}

/*!
**  Notes:    Ends the use of this material.
**/
void Material::End(void)
{
  if(m_currentPass)
  {
    m_currentPass->End();
    m_currentPass = 0;
  }
}

Material::Material(MaterialNode *node)
{
  m_node            = node;
  m_defaultPass     = node->m_defaultPass;
  m_ambientPass     = node->m_ambientPass;
  m_directionalPass = node->m_directionalPass;
  m_pointPass       = node->m_pointPass;
  m_spotPass        = node->m_spotPass;
  m_currentPass     = 0;
}



