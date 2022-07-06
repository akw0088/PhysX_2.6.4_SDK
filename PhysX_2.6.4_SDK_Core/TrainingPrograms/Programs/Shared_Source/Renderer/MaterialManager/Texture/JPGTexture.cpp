/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    JPEG Texture Loader.
*/

// DEV NOTES
//
// - Use Lava Memory manager! -jd

#include "Renderer/RendererImp.h"
#include "Renderer/MaterialManager/TextureManager.h"

#include "JPGTexture.h"

// Hacks because jpeglib is crap!
#define XMD_H
#if defined(FAR)
#undef FAR
#endif
#include <stdio.h>
#include <setjmp.h>
#ifdef  __cplusplus
extern "C" {
#endif
#include <jpeglib.h>
#ifdef  __cplusplus
}
#endif

struct lv_error_mgr
{
  struct jpeg_error_mgr pub;	          /* "public" fields */
  jmp_buf               setjmp_buffer;	/* for return to caller */
};

void init_source (j_decompress_ptr cinfo)
{
  
}

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
  return 1;
}

void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes <= 0)
    return;
  cinfo->src->bytes_in_buffer -= num_bytes;
  cinfo->src->next_input_byte += num_bytes;
}

void term_source (j_decompress_ptr cinfo)
{
}


/*!
**  In:       pcPath    - Path to texture.
**            desc      - texture descriptor.
**
**  Return:   Error code.
**
**  Notes:    Load texture from file.
**/
LvError JPGTexture::LoadFromFile(const LvChar *pcPath, const LvrTextureDesc &desc)
{
  LvError       ret      = LV_IGNORED;
  LvUInt8      *fileData = 0;
  LvUInt32      fileLen  = 0;
  
  LvFile *file = m_userio.LV_FileOpen(pcPath, LV_TRUE);
  if(file)
  {
    fileLen = (LvUInt32)file->GetSize();
    if(fileLen)
    {
      fileData = (LvUInt8*)LV_MALLOC(m_allocator,fileLen);
      if(fileData) file->ReadBuffer(fileData, 1, fileLen);
    }
    m_userio.LV_FileClose(file);
  }
  
  if(fileData)
  {
    LvrTextureData texturedata;
    struct jpeg_decompress_struct cinfo;
    struct lv_error_mgr           jerr;
    struct jpeg_source_mgr        srcmgr;
    int                           row_stride;
    JSAMPARRAY                    buffer;
    
    srcmgr.bytes_in_buffer   = fileLen;
    srcmgr.next_input_byte   = fileData;
    srcmgr.init_source       = init_source;
    srcmgr.fill_input_buffer = fill_input_buffer;
    srcmgr.skip_input_data   = skip_input_data;
    srcmgr.resync_to_restart = ::jpeg_resync_to_restart;
    srcmgr.term_source       = term_source;
    
    cinfo.err = jpeg_std_error(&jerr.pub);
    
    if(setjmp(jerr.setjmp_buffer))
    {
      /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
      jpeg_destroy_decompress(&cinfo);
    }
    else
    {
      /* If we get here, all is well... */
      
      /* Now we can initialize the JPEG decompression object. */
      jpeg_create_decompress(&cinfo);
      
      /* specify data source (eg, a file) */
      cinfo.src = &srcmgr;
      
      /* read file parameters with jpeg_read_header() */
      jpeg_read_header(&cinfo, TRUE);
      
      /* Start decompressor */
      jpeg_start_decompress(&cinfo);
      
      /* JSAMPLEs per row in output buffer */
      row_stride = cinfo.output_width * cinfo.output_components;
      
      /* Make a one-row-high sample array that will go away when done with image */
      buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
      
      // Initialize our Lava texture data descriptor.
      texturedata.m_width  = cinfo.output_width;
      texturedata.m_height = cinfo.output_height;
      texturedata.m_depth  = cinfo.output_components;
      LvUInt32 numbytes = texturedata.m_width * texturedata.m_height * texturedata.m_depth;
      LvUInt8 *currData = 0;
      LV_ASSERT(numbytes);
      if(numbytes)
      {
        texturedata.m_data = (LvUInt8*)LV_MALLOC(m_allocator, numbytes);
        currData = texturedata.m_data;
      }
      
      /* Here we use the library's state variable cinfo.output_scanline as the
      * loop counter, so that we don't have to keep track ourselves.
      */
      while (cinfo.output_scanline < cinfo.output_height)
      {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
        * Here the array is only one element long, but you could ask for
        * more than one scanline at a time if that's more convenient.
        */
        jpeg_read_scanlines(&cinfo, buffer, 1);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
        //put_scanline_someplace(buffer[0], row_stride);
        memcpy(currData, buffer[0], row_stride);
        currData += row_stride;
      }
      
      /* Finish decompression */
      jpeg_finish_decompress(&cinfo);
      
      /* This is an important step since it will release a good deal of memory. */
      jpeg_destroy_decompress(&cinfo);
    }
    LV_FREE(m_allocator, fileData);
    
    if(texturedata.m_data)
    {
      ret = m_renderer.UploadTexture(m_gGUID, texturedata, desc);
      LV_FREE(m_allocator, texturedata.m_data);
      if(ret == LV_OKAY)
      {
        m_pcName = (LvChar*)LV_MALLOC(m_allocator, strlen(pcPath)+1);
        strcpy(m_pcName, pcPath);
      }
    }
  }
  
  return ret;
}

