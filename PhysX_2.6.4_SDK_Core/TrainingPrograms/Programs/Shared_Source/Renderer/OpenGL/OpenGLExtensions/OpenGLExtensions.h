/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/


#ifndef LV__OPENGLEXTENSIONS_H
#define LV__OPENGLEXTENSIONS_H


#include <Lava/LvLava.h>

#ifdef LV_WINDOWS
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#endif
#ifdef LV_APPLE
#include <AGL/agl.h>
#include "aglGetProcAddress.h"
#endif

#include "OpenGLExtShader.h"
#include "OpenGLMultText.h"
#include "OpenGLExtVBO.h"

#if defined(LV_WINDOWS)
#include "OpenGLExtwgl.h"
#endif

#ifndef GL_VERSION_1_4
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_POINT_DISTANCE_ATTENUATION     0x8129
#define GL_GENERATE_MIPMAP                0x8191
#define GL_GENERATE_MIPMAP_HINT           0x8192
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_MIRRORED_REPEAT                0x8370
#define GL_FOG_COORDINATE_SOURCE          0x8450
#define GL_FOG_COORDINATE                 0x8451
#define GL_FRAGMENT_DEPTH                 0x8452
#define GL_CURRENT_FOG_COORDINATE         0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE      0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE    0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER   0x8456
#define GL_FOG_COORDINATE_ARRAY           0x8457
#define GL_COLOR_SUM                      0x8458
#define GL_CURRENT_SECONDARY_COLOR        0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE     0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE     0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE   0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER  0x845D
#define GL_SECONDARY_COLOR_ARRAY          0x845E
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_TEXTURE_LOD_BIAS               0x8501
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_COMPARE_R_TO_TEXTURE           0x884E
#endif

// Point Sprites.
#define GL_POINT_DISTANCE_ATTENUATION_ARB 0x8129
#define GL_POINT_SIZE_MIN_ARB             0x8126
#define GL_POINT_SIZE_MAX_ARB             0x8127
#define GL_POINT_SPRITE_ARB               0x8861
#define GL_COORD_REPLACE_ARB              0x8862
typedef void (APIENTRY *PFNGLPOINTPARAMETERFVPROC)(GLenum, const GLfloat *);
extern PFNGLPOINTPARAMETERFVPROC glPointParameterfvARB;
typedef void (APIENTRY *PFNGLPOINTPARAMETERFPROC)(GLenum, GLfloat);
extern PFNGLPOINTPARAMETERFPROC glPointParameterfARB;


// DXTC. Texture Compression.
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3


// Two Sided Stencil support.
#define GL_STENCIL_TEST_TWO_SIDE_EXT      0x8910
#define GL_ACTIVE_STENCIL_FACE_EXT        0x8911
#define GL_INCR_WRAP_EXT                  0x8507
#define GL_DECR_WRAP_EXT                  0x8508
typedef void (LV_APIENTRY * PFNGLACTIVESTENCILFACEEXTPROC)(GLenum face);
extern PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT;

// ATI_separate_stencil
// glStencilOpSeparateATI(GLenum,GLenum,GLenum,GLenum)
typedef void (LV_APIENTRY * PFNGLSTENCILOPSEPARATEATIPROC)(GLenum,GLenum,GLenum,GLenum);
extern PFNGLSTENCILOPSEPARATEATIPROC glStencilOpSeparateATI;
// glStencilFuncSeparateATI(GLenum,GLenum,GLint,GLuint)
typedef void (LV_APIENTRY * PFNGLSTENCILFUNCSEPARATEATIPROC)(GLenum,GLenum,GLenum,GLenum);
extern PFNGLSTENCILFUNCSEPARATEATIPROC glStencilFuncSeparateATI;

// Frame Buffer Objects.
#define GL_FRAMEBUFFER_EXT                                    0x8D40
#define GL_RENDERBUFFER_EXT                                   0x8D41
#define GL_STENCIL_INDEX1_EXT                                 0x8D46
#define GL_STENCIL_INDEX4_EXT                                 0x8D47
#define GL_STENCIL_INDEX8_EXT                                 0x8D48
#define GL_STENCIL_INDEX16_EXT                                0x8D49
#define GL_RENDERBUFFER_WIDTH_EXT                             0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT                            0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                   0x8D44
#define GL_RENDERBUFFER_RED_SIZE_EXT                          0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT                        0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT                         0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT                        0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT                        0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT                      0x8D55
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT             0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT             0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT           0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT   0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT      0x8CD4
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT    0x8CD8
#define GL_COLOR_ATTACHMENT0_EXT                              0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                              0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                              0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                              0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                              0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                              0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                              0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                              0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT                              0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT                              0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT                             0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT                             0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT                             0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT                             0x8CED
#define GL_COLOR_ATTACHMENT14_EXT                             0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT                             0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                               0x8D00
#define GL_STENCIL_ATTACHMENT_EXT                             0x8D20
#define GL_FRAMEBUFFER_COMPLETE_EXT                           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT              0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT      0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT              0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                 0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT             0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT             0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                        0x8CDD
#define GL_FRAMEBUFFER_BINDING_EXT                            0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT                           0x8CA7
#define GL_MAX_COLOR_ATTACHMENTS_EXT                          0x8CDF
#define GL_MAX_RENDERBUFFER_SIZE_EXT                          0x84E8
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                  0x0506

typedef GLboolean (LV_APIENTRY * PFNGLISRENDERBUFFEREXTPROC)(GLuint);
extern PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
typedef void (LV_APIENTRY * PFNGLBINDRENDERBUFFEREXTPROC)(GLenum,GLuint);
extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
typedef void (LV_APIENTRY * PFNGLDELETERENDERBUFFERSEXTPROC)(GLsizei,const GLuint*);
extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
typedef void (LV_APIENTRY * PFNGLGENRENDERBUFFERSEXTPROC)(GLsizei,GLuint *);
extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
typedef void (LV_APIENTRY * PFNGLRENDERBUFFERSTORAGEEXTPROC)(GLenum,GLenum,GLsizei,GLsizei);
extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
typedef void (LV_APIENTRY * PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)(GLenum,GLenum,GLint *);
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
typedef GLboolean (LV_APIENTRY * PFNGLISFRAMEBUFFEREXTPROC)(GLuint);
extern PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
typedef void (LV_APIENTRY * PFNGLBINDFRAMEBUFFEREXTPROC)(GLenum,GLuint);
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
typedef void (LV_APIENTRY * PFNGLDELETEFRAMEBUFFERSEXTPROC)(GLsizei,const GLuint *);
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
typedef void (LV_APIENTRY * PFNGLGENFRAMEBUFFERSEXTPROC)(GLsizei,const GLuint *);
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
typedef GLenum (LV_APIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)(GLenum);
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
typedef void (LV_APIENTRY * PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)(GLenum,GLenum,GLenum,GLuint,GLint);
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
typedef void (LV_APIENTRY * PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)(GLenum,GLenum,GLenum,GLuint,GLint);
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
typedef void (LV_APIENTRY * PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)(GLenum,GLenum,GLenum,GLuint,GLint);
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
typedef void (LV_APIENTRY * PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)(GLenum,GLenum,GLenum,GLuint);
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
typedef void (LV_APIENTRY * PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)(GLenum,GLenum,GLenum,GLint *);
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
typedef void (LV_APIENTRY * PFNGLGENERATEMIPMAPEXTPROC)(GLenum);
extern PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;


/*!
**  Notes:    Load the extensions.
**/
LvError glextLoad(void);



#endif
