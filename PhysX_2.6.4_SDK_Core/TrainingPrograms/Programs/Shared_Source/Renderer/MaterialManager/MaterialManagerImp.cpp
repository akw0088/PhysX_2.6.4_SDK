/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Implementation of the Material Manager.
*/

#include "Renderer/RendererImp.h"

#include "MaterialManagerImp.h"
#include <stdio.h>

/*********************
** MATERIAL MANAGER **
*********************/

MaterialManagerImp::MaterialManagerImp(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer) :
m_allocator(allocator), m_userio(userio), m_renderer(renderer)
{
  m_pHeadMaterial    = 0;
  m_pCurrentMaterial = 0;
}

MaterialManagerImp::~MaterialManagerImp(void)
{
  
}

/*!
**  In:       tDesc - A material descriptor.
**
**  Return:   Instance of a material or 0 for error.
**
**  Notes:    Creates an instance of a Material
**/
LvrMaterial *MaterialManagerImp::CreateMaterial(const LvrMaterialDesc &tDesc)
{
  MaterialNode *pNode     = 0;
  Material     *pMaterial = 0;
  
  LV_ASSERT(tDesc.defaultPass);
  if(tDesc.defaultPass)
  {
    pNode = LV_NEW(m_allocator, MaterialNode) MaterialNode(m_allocator);
    
    // Default Pass.
    pNode->m_defaultPass = m_renderer.CreateMaterialPass();
    if(pNode->m_defaultPass)
      pNode->m_defaultPass->LoadFromDesc(*tDesc.defaultPass);
    
    // Ambient Lighting Pass.
    if(tDesc.ambientPass)
    {
      pNode->m_ambientPass = m_renderer.CreateMaterialPass();
      if(pNode->m_ambientPass)
        pNode->m_ambientPass->LoadFromDesc(*tDesc.ambientPass);
    }
    
    // Directional Light Pass.
    if(tDesc.directionalLightPass)
    {
      pNode->m_directionalPass = m_renderer.CreateMaterialPass();
      if(pNode->m_directionalPass)
        pNode->m_directionalPass->LoadFromDesc(*tDesc.directionalLightPass);
    }
    
    // Point Light Pass.
    if(tDesc.pointLightPass)
    {
      pNode->m_pointPass = m_renderer.CreateMaterialPass();
      if(pNode->m_pointPass)
        pNode->m_pointPass->LoadFromDesc(*tDesc.pointLightPass);
    }
    
    // Spot Light Pass.
    if(tDesc.spotLightPass)
    {
      pNode->m_spotPass = m_renderer.CreateMaterialPass();
      if(pNode->m_spotPass)
        pNode->m_spotPass->LoadFromDesc(*tDesc.spotLightPass);
    }
    
    pMaterial = LV_NEW(m_allocator, Material) Material(pNode);
  }
  
  return pMaterial;
}

/*!
**  In:       pMaterial - Instance of a material
**
**  Notes:    Destroys an instance of a material.
**/
void MaterialManagerImp::ReleaseMaterial(LvrMaterial *pMaterial)
{
  Material *pMat = (Material*)pMaterial;
  if(pMat)
  {
    MaterialNode *node = pMat->m_node;
    node->m_numUsers--;
    if(node->m_numUsers <= 0)
    {
      if(node->m_defaultPass)  node->m_defaultPass->Free();
      if(node->m_ambientPass)  node->m_ambientPass->Free();
      if(node->m_parent)
      {
        LV_ASSERT(0);
      }
      else if(node == m_pHeadMaterial)
      {
        LV_ASSERT(0);
      }
      m_renderer.ReleaseMaterialPass(node->m_defaultPass);
      m_renderer.ReleaseMaterialPass(node->m_ambientPass);
      LV_DELETE(m_allocator, MaterialNode, node);
    }
    LV_DELETE(m_allocator, Material, pMat);
  }
}

/*!
**  In:       pass      - what pass to render with.
**            pMaterial - The material to use.
**
**  Return:   Error code.
**
**  Notes:    Starts the use of a Material.
**/
LvError MaterialManagerImp::Begin(MaterialPassE pass, LvrMaterial *pMaterial)
{
  LvError ret = LV_IGNORED;
  Material *pMat = (Material*)pMaterial;
  if(m_pCurrentMaterial) End();
  if(pMat)
  {
    pMat->Begin(pass);
    m_pCurrentMaterial = pMat;
    ret = LV_OKAY;
  }
  return ret;
}

/*!
**  Notes:    Ends the current Material.
**/
void MaterialManagerImp::End(void)
{
  if(m_pCurrentMaterial)
  {
    m_pCurrentMaterial->End();
    m_pCurrentMaterial = 0;
  }
}

/*!
**  Return:   Error code.
**
**  Notes:    Initialize the Material manager.
**/
LvError MaterialManagerImp::Init(void)
{
  LvError ret        = LV_OKAY;
  m_pHeadMaterial    = 0;
  m_pCurrentMaterial = 0;
  return ret;
}

/*!
**  Return:   Error code.
**
**  Notes:    Shutdown the Material Manager.
**/
LvError MaterialManagerImp::Shutdown(void)
{
  LV_ASSERT(!m_pHeadMaterial);
  return LV_OKAY;
}
