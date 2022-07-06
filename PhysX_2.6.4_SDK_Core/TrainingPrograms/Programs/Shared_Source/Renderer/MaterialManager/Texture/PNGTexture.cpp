/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    PNG Texture Loader.
*/

#include "Renderer/RendererImp.h"
#include "Renderer/MaterialManager/TextureManager.h"

#include "PNGTexture.h"

#include <png.h>

/* Settings */
#define LV_PNG_HEADER_SIZE    8
#define LV_PNG_READ_SIZE    128


static void LvPNGRead(png_structp png_ptr, png_bytep data, png_size_t length)
{
  LvFile *file = (LvFile*)png_get_io_ptr(png_ptr);
  LV_ASSERT(file);
  file->ReadBuffer(data, 1, length);
}

static png_voidp LvPNGMalloc(png_structp png_ptr, png_size_t size)
{
  LvAllocator *allocator = (LvAllocator*)png_get_mem_ptr(png_ptr);
  LV_ASSERT(allocator);
  return LV_MALLOC((*allocator), size);
}

static void LvPNGFree(png_structp png_ptr, png_voidp ptr)
{
  LvAllocator *allocator = (LvAllocator*)png_get_mem_ptr(png_ptr);
  LV_ASSERT(allocator);
  LV_FREE((*allocator), ptr);
}


/*!
**  In:       pcPath    - Path to texture.
**            desc      - texture descriptor.
**
**  Return:   Error code.
**
**  Notes:    Load texture from file.
**/
LvError PNGTexture::LoadFromFile(const LvChar *pcPath, const LvrTextureDesc &desc)
{
  LvFile     *file                          = m_userio.LV_FileOpen(pcPath, LV_TRUE);
  LvUInt32    uiBytesRead                   = 0;
  LvUInt8     vHeader[LV_PNG_HEADER_SIZE]   = {0},
             *pData                         = 0;
  LvSInt32    iTemp;
  LvSInt32    iChannelDepth,
              iColorType,
              iInterlaceType,
              iCompressionType,
              iFilterMethod;
  LvError     error                         = LV_OKAY;
  png_structp pPNGStruct;
  png_infop   pPNGInfo;
  
  LvUInt32    width, height, numChannels, size;
  
  // Nuke our Internal data.
  m_pL        = 0;
  m_pR        = 0;
  m_pcName    = 0;
  m_iNumUsers = 0;
  m_gGUID     = 0;
  
  if(!file)
  {
    // Failed to open file.
    return LV_FILE_ERROR;
  }
  
  uiBytesRead = (LvUInt32)file->ReadBuffer(vHeader, 1, LV_PNG_HEADER_SIZE);
  if(uiBytesRead != LV_PNG_HEADER_SIZE)
  {
    // Read failure.
    m_userio.LV_FileClose(file);
    return LV_FILE_ERROR;
  }
  
  iTemp = png_sig_cmp(vHeader, 0, LV_PNG_HEADER_SIZE);
  if(iTemp)
  {
    // Bad Header Data.
    m_userio.LV_FileClose(file);
    return LV_BAD_OUTPUT;
  }
  
  pPNGStruct = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, 0, 0, 0, &m_allocator, LvPNGMalloc, LvPNGFree);
  if(!pPNGStruct)
  {
    // Failed to create PNG Structure.
    m_userio.LV_FileClose(file);
    return LV_BAD_OUTPUT;
  }
  
  pPNGInfo = png_create_info_struct(pPNGStruct);
  if(!pPNGInfo)
  {
    // Failed to create PNG Info structure.
    png_destroy_read_struct(&pPNGStruct, 0, 0);
    m_userio.LV_FileClose(file);
    return LV_BAD_OUTPUT;
  }
  
  /* This is mega leet error handling, uses jump
   * statements to return to this point incase of
   * error.
   */
  if(setjmp(pPNGStruct->jmpbuf))
  {
    // We jump back to here in the event of an error.
    png_destroy_read_struct(&pPNGStruct, &pPNGInfo, 0);
    m_userio.LV_FileClose(file);
    return LV_BAD_OUTPUT;
  }
  
  // Init PNG IO.
  png_set_read_fn(pPNGStruct, file, LvPNGRead);
  png_set_sig_bytes(pPNGStruct, uiBytesRead);
  
  // Read in PNG info.
  png_read_info(pPNGStruct, pPNGInfo);
  
  // Get info about the texture.
  png_get_IHDR( pPNGStruct, pPNGInfo, (png_uint_32 *)&width, (png_uint_32 *)&height, &iChannelDepth,
          &iColorType, &iInterlaceType, &iCompressionType, &iFilterMethod);
  numChannels = png_get_channels(pPNGStruct, pPNGInfo);
  
  // check for incompatibilities.
  if(iInterlaceType != PNG_INTERLACE_NONE)
  {
    png_destroy_read_struct(&pPNGStruct, &pPNGInfo, 0);
    m_userio.LV_FileClose(file);
    return LV_BAD_OUTPUT;
  }
  
  // Make the png file friendly to our needs.
  if(iColorType == PNG_COLOR_TYPE_PALETTE)
  {
    png_set_palette_to_rgb(pPNGStruct);
    numChannels  = 3;
    iChannelDepth = 8;
  }
  if(iColorType == PNG_COLOR_TYPE_GRAY && iChannelDepth < 8)
  {
    png_set_gray_1_2_4_to_8(pPNGStruct);
    iChannelDepth = 8;
  }
  if(png_get_valid(pPNGStruct, pPNGInfo, PNG_INFO_tRNS))
  {
    png_set_tRNS_to_alpha(pPNGStruct);
    numChannels = 4;
  }
  if(iChannelDepth == 16)
  {
    png_set_strip_16(pPNGStruct);
    iChannelDepth = 8;
  }
  
  // Calculate the size of texture data.
  size = width * height * numChannels;
  pData = (LvUInt8*)LV_MALLOC(m_allocator, size);
  
  /* Read the image */
  LvSInt32 iCurrentRow = 0;
  for(iTemp = height; iTemp > 0; iTemp -= LV_PNG_READ_SIZE)
  {
    LvSInt32  iRows = iTemp > LV_PNG_READ_SIZE? LV_PNG_READ_SIZE: iTemp,
          i;
    LvUInt8   *pvRowPointers[LV_PNG_READ_SIZE];
    for(i=0; i<iRows; i++)
    {
      pvRowPointers[i] = &pData[iCurrentRow*width*numChannels];
      iCurrentRow++;
    }
    png_read_rows(pPNGStruct, pvRowPointers, 0, iRows);
  }
  
  LvrTextureData  texturedata;
  texturedata.m_width       = width;
  texturedata.m_height      = height;
  texturedata.m_depth       = numChannels;
  texturedata.m_data        = pData;
  
  error = m_renderer.UploadTexture(m_gGUID, texturedata, desc);
  
  // Free Texture memory (it should be in VRAM by now).
  if(pData) LV_FREE(m_allocator, pData);
  
  // Free PNG memory.
  png_destroy_read_struct(&pPNGStruct, &pPNGInfo, 0);
  
  m_userio.LV_FileClose(file);
  
  if(error == LV_OKAY)
  {
    m_pcName = (LvChar*)LV_MALLOC(m_allocator, strlen(pcPath)+1);
    strcpy(m_pcName, pcPath);
  }
  
  return error;
}
