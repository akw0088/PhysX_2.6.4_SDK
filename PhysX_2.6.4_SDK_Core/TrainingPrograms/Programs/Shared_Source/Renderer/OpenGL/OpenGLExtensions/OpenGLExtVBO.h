/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/

#ifndef LV__OPENGLEXTVBO_H
#define LV__OPENGLEXTVBO_H

#include <Lava/LvLava.h>

#ifdef LV_WINDOWS
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#ifdef LV_APPLE
#define __glext_h_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

/*!
**  Notes:    Load the extensions.
**/
LvError glextLoadVBOExt(void);



#define GL_ARRAY_BUFFER_ARB                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB                     0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB                     0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB             0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB              0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB              0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB               0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB               0x8899
#define TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB          0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB           0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB     0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB      0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB              0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB       0x889F
#define GL_STREAM_DRAW_ARB                              0x88E0
#define GL_STREAM_READ_ARB                              0x88E1
#define GL_STREAM_COPY_ARB                              0x88E2
#define GL_STATIC_DRAW_ARB                              0x88E4
#define GL_STATIC_READ_ARB                              0x88E5
#define GL_STATIC_COPY_ARB                              0x88E6
#define GL_DYNAMIC_DRAW_ARB                             0x88E8
#define GL_DYNAMIC_READ_ARB                             0x88E9
#define GL_DYNAMIC_COPY_ARB                             0x88EA
#define GL_READ_ONLY_ARB                                0x88B8
#define GL_WRITE_ONLY_ARB                               0x88B9
#define GL_READ_WRITE_ARB                               0x88BA
#define GL_BUFFER_SIZE_ARB                              0x8764
#define GL_BUFFER_USAGE_ARB                             0x8765
#define GL_BUFFER_ACCESS_ARB                            0x88BB
#define GL_BUFFER_MAPPED_ARB                            0x88BC
#define GL_BUFFER_MAP_POINTER_ARB                       0x88BD

typedef void (LV_APIENTRY * PFNGLBINDBUFFERARBPROC)            (GLenum target, GLuint buffer);
typedef void (LV_APIENTRY * PFNGLDELETEBUFFERSARBPROC)         (GLsizei n, const GLuint *buffers);
typedef void (LV_APIENTRY * PFNGLGENBUFFERSARBPROC)            (GLsizei n, GLuint *buffers);
typedef void (LV_APIENTRY * PFNGLBUFFERDATAARBPROC)            (GLenum target, int size, const GLvoid *data, GLenum usage);
typedef void (LV_APIENTRY * PFNGLGETBUFFERPARAMETERIVARBPROC)  (GLenum target, GLenum pname, GLint *params);
extern PFNGLGENBUFFERSARBPROC                               glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC                               glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC                               glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC                            glDeleteBuffersARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC                     glGetBufferParameterivARB;


#endif
