/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Direct3D Material Pass.
*/

#include "D3DMaterialPass.h"

D3DMaterialPass::D3DMaterialPass(Direct3DRenderer &renderer, IDirect3DDevice9 *device) : m_renderer(renderer)
{
  m_d3dDevice = device;
}

D3DMaterialPass::~D3DMaterialPass(void)
{
  
}

/*!
**  Notes:    Starts the use of this material.
**/
void D3DMaterialPass::Begin(void)
{
  for(LvUInt32 i=0; i<m_uiNumTextures; i++)
  {
    Texture *texture = m_pTextures[i];
    LV_ASSERT(texture);
    IDirect3DTexture9 *d3dTexture = (IDirect3DTexture9*)texture->GetGUID();
    LV_ASSERT(d3dTexture);
    HRESULT hResult = m_d3dDevice->SetTexture(i, d3dTexture);
    LV_ASSERT(SUCCEEDED(hResult));
  }
}

/*!
**  Notes:    Ends the use of this material.
**/
void D3DMaterialPass::End(void)
{
  for(LvUInt32 i=0; i<m_uiNumTextures; i++)
  {
    m_d3dDevice->SetTexture(i, 0);
  }
}

