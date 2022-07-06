/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL MultiTexture extensions.
*/

#ifndef LV__OPENGLMULTTEXT_H
#define LV__OPENGLMULTTEXT_H

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
LvError glextLoadMultText(void);

#define GL_ACTIVE_TEXTURE_ARB               0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB        0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB            0x84E2
#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1
#define GL_TEXTURE2_ARB                     0x84C2
#define GL_TEXTURE3_ARB                     0x84C3
#define GL_TEXTURE4_ARB                     0x84C4
#define GL_TEXTURE5_ARB                     0x84C5
#define GL_TEXTURE6_ARB                     0x84C6
#define GL_TEXTURE7_ARB                     0x84C7
#define GL_TEXTURE8_ARB                     0x84C8
#define GL_TEXTURE9_ARB                     0x84C9
#define GL_TEXTURE10_ARB                    0x84CA
#define GL_TEXTURE11_ARB                    0x84CB
#define GL_TEXTURE12_ARB                    0x84CC
#define GL_TEXTURE13_ARB                    0x84CD
#define GL_TEXTURE14_ARB                    0x84CE
#define GL_TEXTURE15_ARB                    0x84CF
#define GL_TEXTURE16_ARB                    0x84D0
#define GL_TEXTURE17_ARB                    0x84D1
#define GL_TEXTURE18_ARB                    0x84D2
#define GL_TEXTURE19_ARB                    0x84D3
#define GL_TEXTURE20_ARB                    0x84D4
#define GL_TEXTURE21_ARB                    0x84D5
#define GL_TEXTURE22_ARB                    0x84D6
#define GL_TEXTURE23_ARB                    0x84D7
#define GL_TEXTURE24_ARB                    0x84D8
#define GL_TEXTURE25_ARB                    0x84D9
#define GL_TEXTURE26_ARB                    0x84DA
#define GL_TEXTURE27_ARB                    0x84DB
#define GL_TEXTURE28_ARB                    0x84DC
#define GL_TEXTURE29_ARB                    0x84DD
#define GL_TEXTURE30_ARB                    0x84DE
#define GL_TEXTURE31_ARB                    0x84DF

typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (LV_APIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
typedef void (LV_APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum target);
typedef void (LV_APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum target);

extern PFNGLMULTITEXCOORD1DARBPROC    glMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD1DVARBPROC   glMultiTexCoord1dvARB;  
extern PFNGLMULTITEXCOORD1FARBPROC    glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FVARBPROC   glMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1IARBPROC    glMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1IVARBPROC   glMultiTexCoord1ivARB;  
extern PFNGLMULTITEXCOORD1SARBPROC    glMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1SVARBPROC   glMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD2DARBPROC    glMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD2DVARBPROC   glMultiTexCoord2dvARB;  
extern PFNGLMULTITEXCOORD2FARBPROC    glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC   glMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2IARBPROC    glMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2IVARBPROC   glMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2SARBPROC    glMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2SVARBPROC   glMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD3DARBPROC    glMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD3DVARBPROC   glMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD3FARBPROC    glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3FVARBPROC   glMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3IARBPROC    glMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3IVARBPROC   glMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3SARBPROC    glMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3SVARBPROC   glMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD4DARBPROC    glMultiTexCoord4dARB;
extern PFNGLMULTITEXCOORD4DVARBPROC   glMultiTexCoord4dvARB;  
extern PFNGLMULTITEXCOORD4FARBPROC    glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC   glMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4IARBPROC    glMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4IVARBPROC   glMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4SARBPROC    glMultiTexCoord4sdARB;
extern PFNGLMULTITEXCOORD4SVARBPROC   glMultiTexCoord4svARB;
extern PFNGLACTIVETEXTUREARBPROC    glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC  glClientActiveTextureARB;

#endif
