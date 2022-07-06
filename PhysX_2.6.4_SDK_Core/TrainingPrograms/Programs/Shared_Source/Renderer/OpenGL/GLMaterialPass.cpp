/*!
**  Author:   James Dolan (jdolan@gmail.com)
**            Taylor White (taylorcwhite@gmail.com)
**
**  Notes:    OpenGL Material Pass.
*/

#include "GLMaterialPass.h"

#include "OpenGLRenderer.h"

static GLuint g_uiGLBlendFuncs[] =
{
  GL_ZERO,
  GL_ONE,
  GL_SRC_COLOR,
  GL_ONE_MINUS_SRC_COLOR,
  GL_SRC_ALPHA,
  GL_ONE_MINUS_SRC_ALPHA,
  GL_DST_ALPHA,
  GL_ONE_MINUS_DST_ALPHA,
  GL_DST_COLOR,
  GL_ONE_MINUS_DST_COLOR,
  GL_SRC_ALPHA_SATURATE,
};

/*!
**  Notes:    Starts the use of this material.
**/
void GLMaterialPass::Begin(void)
{
  if(m_bLighting) glEnable(GL_LIGHTING);
  
  if(m_bBlending)
  {
    glEnable(GL_BLEND);
    glBlendFunc(g_uiGLBlendFuncs[m_eBlendSrc], g_uiGLBlendFuncs[m_eBlendDest]);
  }
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   (GLfloat*)&m_vAmbientColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   (GLfloat*)&m_vDiffuseColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  (GLfloat*)&m_vSpecularColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  (GLfloat*)&m_vEmissionColor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_fShininess);
  
  m_renderer.EnableTextureUnit(m_uiNumTextures);
  for(LvUInt32 i=0; i<m_uiNumTextures; i++)
  {
    if(m_pTextures[i]) m_renderer.BindTexture(m_pTextures[i]->GetGUID(), i);
  }   

  if(m_pVertexShader)
  {
  // TWHACK: There is no problem that we check to see if the param is valid inside of
  //  SetStateMatrix(), but should we check here??

  // Set matricies
  m_pVertexShader->SetStateMatrix(m_ModelViewProjectionMatrixParam, LVR_MODELVIEW_PROJECTION_MATRIX);
  m_pVertexShader->SetStateMatrix(m_ModelViewMatrixParam, LVR_MODELVIEW_MATRIX);
  m_pVertexShader->SetStateMatrix(m_ProjectionMatrixParam, LVR_PROJECTION_MATRIX);

    m_pVertexShader->Begin();
  }
  if(m_pFragmentShader)
    m_pFragmentShader->Begin();
}

/*!
**  Notes:    Ends the use of this material.
**/
void GLMaterialPass::End(void)
{
  if(m_bLighting) glDisable(GL_LIGHTING);
  if(m_bBlending) glDisable(GL_BLEND);
  m_renderer.EnableTextureUnit(0);
  
  if(m_pVertexShader)
    m_pVertexShader->End();
  if(m_pFragmentShader)
    m_pFragmentShader->End();
}
