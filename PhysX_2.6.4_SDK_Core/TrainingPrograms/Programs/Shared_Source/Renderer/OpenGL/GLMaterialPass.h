/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Material Pass.
*/

#ifndef LV__GLMATERIALPASS_H
#define LV__GLMATERIALPASS_H

#include "Renderer/RendererImp.h"
#include "Renderer/OpenGL/OpenGLRenderer.h"
#include "Renderer/MaterialManager/MaterialManagerImp.h"

class GLMaterialPass : public MaterialPass
{
  public:
    /*!
    **  Notes:    Starts the use of this material.
    **/
    virtual void Begin(void);
    
    /*!
    **  Notes:    Ends the use of this material.
    **/
    virtual void End(void);
    
  public:
    GLMaterialPass(OpenGLRenderer &renderer) : m_renderer(renderer)
    {
      
    }
    
    virtual ~GLMaterialPass(void)
    {
      
    }
    
  private:
    OpenGLRenderer  &m_renderer;
};

#endif
