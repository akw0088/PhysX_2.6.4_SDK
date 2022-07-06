/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Direct3D Material Pass.
*/

#ifndef D3DMATERIALPASS_H
#define D3DMATERIALPASS_H

#include "Renderer/RendererImp.h"
#include "Renderer/Direct3D/Direct3DRenderer.h"
#include "Renderer/MaterialManager/MaterialManagerImp.h"

class D3DMaterialPass : public MaterialPass
{
  public:
    D3DMaterialPass(Direct3DRenderer &renderer, IDirect3DDevice9 *device);
    virtual ~D3DMaterialPass(void);
    
  public:
    /*!
    **  Notes:    Starts the use of this material.
    **/
    virtual void Begin(void);
    
    /*!
    **  Notes:    Ends the use of this material.
    **/
    virtual void End(void);
    
  private:
    Direct3DRenderer  &m_renderer;
    IDirect3DDevice9  *m_d3dDevice;
};

#endif
