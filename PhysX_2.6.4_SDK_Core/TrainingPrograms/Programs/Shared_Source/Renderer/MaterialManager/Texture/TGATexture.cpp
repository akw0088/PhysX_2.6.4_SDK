/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    Targa Texture Loader.
*/

// DEV NOTES:
//
// -  We should really support RLE compression. -jd

#include "Renderer/RendererImp.h"
#include "Renderer/MaterialManager/TextureManager.h"

#include "TGATexture.h"

#pragma pack(1)
class TGA_HEADER
{
  public:
	  LvUInt8   identsize;          // size of ID field that follows 18 byte header (0 usually)
	  LvUInt8   colourmaptype;      // type of colour map 0=none, 1=has palette
	  LvUInt8   imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

	  LvUInt16  colourmapstart;     // first colour map entry in palette
	  LvUInt16  colourmaplength;    // number of colours in palette
	  LvUInt8   colourmapbits;      // number of bits per palette entry 15,16,24,32

	  LvUInt16  xstart;             // image x origin
	  LvUInt16  ystart;             // image y origin
	  LvUInt16  width;              // image width in pixels
	  LvUInt16  height;             // image height in pixels
	  LvUInt8   bits;               // image bits per pixel 8,16,24,32
	  LvUInt8   descriptor;         // image descriptor bits (vh flip bits)
  
  public:
    LvBool FixEndian(void)
    {
    #if defined(LV_BIG_ENDIAN)
      colourmapstart  = LvSwapEndian16(colourmapstart);
      colourmaplength = LvSwapEndian16(colourmaplength);
      xstart          = LvSwapEndian16(xstart);
      ystart          = LvSwapEndian16(ystart);
      width           = LvSwapEndian16(width);
      height          = LvSwapEndian16(height);
      return LV_TRUE;
    #else
      return LV_FALSE;
    #endif
    }
};
#pragma pack()


/*!
**  In:       pcPath    - Path to texture.
**            desc      - texture descriptor.
**
**  Return:   Error code.
**
**  Notes:    Load texture from file.
**/
LvError TGATexture::LoadFromFile(const LvChar *pcPath, const LvrTextureDesc &desc)
{
  LvError       error    = LV_IGNORED;
  LvUInt8      *fileData = 0;
  LvUInt32      fileLen  = 0;
  
  LvFile *file = m_userio.LV_FileOpen(pcPath, LV_TRUE);
  if(file)
  {
    fileLen = (LvUInt32)file->GetSize();
    if(fileLen>sizeof(TGA_HEADER))
    {
      fileData = (LvUInt8*)LV_MALLOC(m_allocator,fileLen);
      if(fileData) file->ReadBuffer(fileData, 1, fileLen);
    }
    m_userio.LV_FileClose(file);
  }
  
  if(fileData)
  {
    TGA_HEADER &header = *(TGA_HEADER*)fileData;
    header.FixEndian();
    LvUInt8 *imageData = fileData+sizeof(TGA_HEADER);
    LV_ASSERT(header.imagetype == 2); // We need to add support for RLE TGA files!!!
    if(header.imagetype == 2)
    {
      imageData += header.identsize;
      LvUInt32 imageDepth = header.bits/8;
      LvUInt32 imageSize  = header.width * header.height * imageDepth;
      
      if(header.width >0 && header.height >0 && (imageDepth == 3 || imageDepth == 4))
      {
        // BGR to RGB.
        for(LvUInt32 i=0; i < imageSize; i += imageDepth)
        {
          LvUInt8 temp   = imageData[i];
          imageData[i]   = imageData[i+2];
          imageData[i+2] = temp;
        }
        LvrTextureData texturedata;
        texturedata.m_width   = header.width;
        texturedata.m_height  = header.height;
        texturedata.m_depth   = imageDepth;
        texturedata.m_data    = imageData;
        error = m_renderer.UploadTexture(m_gGUID, texturedata, desc);
      }
    }
    LV_FREE(m_allocator,fileData);
  }
  
  if(error == LV_OKAY)
  {
    m_pcName = (LvChar*)LV_MALLOC(m_allocator, strlen(pcPath)+1);
    strcpy(m_pcName, pcPath);
  }
  
  return error;
}

