/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Implementation of the Material Manager.
*/

// DEV NOTES
//
// -  Need to remove the texture manager from the MaterialManager
//    as its going to be a public interface soon... -jd

#ifndef LV__MATERIALMANAGERIMP_H
#define LV__MATERIALMANAGERIMP_H


#include <Lava/LvLava.h>

#include "ShaderManager.h"
#include "TextureManager.h"

#include "Renderer/RendererImp.h"

#include "Material.h"

/*!
**  Notes:    Implementation of the Material Manager.
**/
class MaterialManagerImp
{
  friend class Material;
  public:
    MaterialManagerImp(LvAllocator &allocator, LvUserIO &userio, RendererImp &renderer);
    ~MaterialManagerImp(void);
    
  public:
    /*!
    **  In:       tDesc - A material descriptor.
    **
    **  Return:   Instance of a material or 0 for error.
    **
    **  Notes:    Creates an instance of a Material
    **/
    LvrMaterial *CreateMaterial(const LvrMaterialDesc &tDesc);
    
    /*!
    **  In:       pMaterial - Instance of a material
    **
    **  Notes:    Destroys an instance of a material.
    **/
    void ReleaseMaterial(LvrMaterial *pMaterial);
    
    /*!
    **  In:       pass      - what pass to render with.
    **            pMaterial - The material to use.
    **
    **  Return:   Error code.
    **
    **  Notes:    Starts the use of a Material.
    **/
    LvError Begin(MaterialPassE pass, LvrMaterial *pMaterial);
    
    /*!
    **  Notes:    Ends the current Material.
    **/
    void End(void);
  
  public:
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Initialize the Material manager.
    **/
    LvError Init(void);
    
    /*!
    **  Return:   Error code.
    **
    **  Notes:    Shutdown the Material Manager.
    **/
    LvError Shutdown(void);
    
  private:
    MaterialNode     *m_pHeadMaterial;
    Material         *m_pCurrentMaterial;
    
  private:
    LvAllocator      &m_allocator;
    LvUserIO         &m_userio;
    RendererImp      &m_renderer;
};


#endif
