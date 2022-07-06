/*!
**  Author:   James Dolan (jdolan@gmail.com)
**
**  Notes:    OpenGL Extensions.
*/

#ifndef LV__OPENGLEXTSHADER_H
#define LV__OPENGLEXTSHADER_H

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
#define LV_APIENTRY 
#define CALLBACK 
#endif


/*!
**  Notes:    Load the extensions.
**/
LvError glextLoadShaderObject(void);



/**************************
** SHADER OBJECT SUPPORT **
**************************/

#define GL_PROGRAM_OBJECT_ARB                       0x8B40

#define GL_OBJECT_TYPE_ARB                          0x8B4E
#define GL_OBJECT_SUBTYPE_ARB                       0x8B4F
#define GL_OBJECT_DELETE_STATUS_ARB                 0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB                0x8B81
#define GL_OBJECT_LINK_STATUS_ARB                   0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB               0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB               0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB              0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB               0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB     0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB          0x8B88

#define GL_SHADER_OBJECT_ARB                        0x8B48

#define GL_FLOAT_VEC2_ARB                           0x8B50
#define GL_FLOAT_VEC3_ARB                           0x8B51
#define GL_FLOAT_VEC4_ARB                           0x8B52
#define GL_INT_VEC2_ARB                             0x8B53
#define GL_INT_VEC3_ARB                             0x8B54
#define GL_INT_VEC4_ARB                             0x8B55
#define GL_BOOL_ARB                                 0x8B56
#define GL_BOOL_VEC2_ARB                            0x8B57
#define GL_BOOL_VEC3_ARB                            0x8B58
#define GL_BOOL_VEC4_ARB                            0x8B59
#define GL_FLOAT_MAT2_ARB                           0x8B5A
#define GL_FLOAT_MAT3_ARB                           0x8B5B
#define GL_FLOAT_MAT4_ARB                           0x8B5C

typedef char         GLcharARB;
typedef unsigned int GLhandleARB;

typedef GLvoid              (LV_APIENTRY *PFNGLDELETEOBJECTARBPROC)(GLhandleARB obj);
typedef GLhandleARB         (LV_APIENTRY *PFNGLGETHANDLEARBPROC)(GLenum pname);
typedef GLvoid              (LV_APIENTRY *PFNGLDETACHOBJECTARBPROC)(GLhandleARB containerObj, GLhandleARB attachedObj);
typedef GLhandleARB         (LV_APIENTRY *PFNGLCREATESHADEROBJECTARBPROC)(GLenum shaderType);
typedef GLvoid              (LV_APIENTRY *PFNGLSHADERSOURCEARBPROC)(GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
typedef GLvoid              (LV_APIENTRY *PFNGLCOMPILESHADERARBPROC)(GLhandleARB shaderObj);
typedef GLhandleARB         (LV_APIENTRY *PFNGLCREATEPROGRAMOBJECTARBPROC)(GLvoid);
typedef GLvoid              (LV_APIENTRY *PFNGLATTACHOBJECTARBPROC)(GLhandleARB containerObj, GLhandleARB obj);
typedef GLvoid              (LV_APIENTRY *PFNGLLINKPROGRAMARBPROC)(GLhandleARB programObj);
typedef GLvoid              (LV_APIENTRY *PFNGLUSEPROGRAMOBJECTARBPROC)(GLhandleARB programObj);
typedef GLvoid              (LV_APIENTRY *PFNGLVALIDATEPROGRAMARBPROC)(GLhandleARB programObj);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM1FARBPROC)(GLint location, GLfloat v0);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM2FARBPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM3FARBPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM4FARBPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM1IARBPROC)(GLint location, GLint v0);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM2IARBPROC)(GLint location, GLint v0, GLint v1);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM3IARBPROC)(GLint location, GLint v0, GLint v1, GLint v2);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM4IARBPROC)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM1FVARBPROC)(GLint location, GLsizei count, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM2FVARBPROC)(GLint location, GLsizei count, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM3FVARBPROC)(GLint location, GLsizei count, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM4FVARBPROC)(GLint location, GLsizei count, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM1IVARBPROC)(GLint location, GLsizei count, GLint *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM2IVARBPROC)(GLint location, GLsizei count, GLint *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM3IVARBPROC)(GLint location, GLsizei count, GLint *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORM4IVARBPROC)(GLint location, GLsizei count, GLint *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORMMATRIX2FVARBPROC)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORMMATRIX3FVARBPROC)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLUNIFORMMATRIX4FVARBPROC)(GLint location, GLsizei count, GLboolean transpose, GLfloat *value);
typedef GLvoid              (LV_APIENTRY *PFNGLGETOBJECTPARAMETERFVARBPROC)(GLhandleARB obj, GLenum pname, GLfloat *params);
typedef GLvoid              (LV_APIENTRY *PFNGLGETOBJECTPARAMETERIVARBPROC)(GLhandleARB obj, GLenum pname, GLint *params);
typedef GLvoid              (LV_APIENTRY *PFNGLGETINFOLOGARBPROC)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef GLvoid              (LV_APIENTRY *PFNGLGETATTACHEDOBJECTSARBPROC)(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
typedef GLint               (LV_APIENTRY *PFNGLGETUNIFORMLOCATIONARBPROC)(GLhandleARB programObj, const GLcharARB *name);
typedef GLvoid              (LV_APIENTRY *PFNGLGETACTIVEUNIFORMARBPROC)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLvoid              (LV_APIENTRY *PFNGLGETUNIFORMFVARBPROC)(GLhandleARB programObj, GLint location, GLfloat *params);
typedef GLvoid              (LV_APIENTRY *PFNGLGETUNIFORMIVARBPROC)(GLhandleARB programObj, GLint location, GLint *params);
typedef GLvoid              (LV_APIENTRY *PFNGLGETSHADERSOURCEARBPROC)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);



// FRAGMENT SHADER SUPPORT
#define GL_FRAGMENT_SHADER_ARB                      0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB      0x8B49

// VERTEX SHADER SUPPORT

#define GL_VERTEX_SHADER_ARB                        0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB        0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB                   0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB       0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB     0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB             0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB   0x8B8A


typedef GLvoid (LV_APIENTRY *PFNGLBINDATTRIBLOCATIONARBPROC)(GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef GLvoid (LV_APIENTRY *PFNGLGETACTIVEATTRIBARBPROC)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint (LV_APIENTRY *PFNGLGETATTRIBLOCATIONARBPROC)(GLhandleARB programObj, const GLcharARB *name);



extern PFNGLDELETEOBJECTARBPROC                    glDeleteObjectARB;
extern PFNGLGETHANDLEARBPROC                       glGetHandleARB;
extern PFNGLDETACHOBJECTARBPROC                    glDetachObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC              glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC                    glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC                   glCompileShaderARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC             glCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC                    glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC                     glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC                glUseProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC                 glValidateProgramARB;
extern PFNGLUNIFORM1FARBPROC                       glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC                       glUniform2fARB;
extern PFNGLUNIFORM3FARBPROC                       glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                       glUniform4fARB;
extern PFNGLUNIFORM1IARBPROC                       glUniform1iARB;
extern PFNGLUNIFORM2IARBPROC                       glUniform2iARB;
extern PFNGLUNIFORM3IARBPROC                       glUniform3iARB;
extern PFNGLUNIFORM4IARBPROC                       glUniform4iARB;
extern PFNGLUNIFORM1FVARBPROC                      glUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC                      glUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC                      glUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC                      glUniform4fvARB;
extern PFNGLUNIFORM1IVARBPROC                      glUniform1ivARB;
extern PFNGLUNIFORM2IVARBPROC                      glUniform2ivARB;
extern PFNGLUNIFORM3IVARBPROC                      glUniform3ivARB;
extern PFNGLUNIFORM4IVARBPROC                      glUniform4ivARB;
extern PFNGLUNIFORMMATRIX2FVARBPROC                glUniformMatrix2fvARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC                glUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC                glUniformMatrix4fvARB;
extern PFNGLGETOBJECTPARAMETERFVARBPROC            glGetObjectParameterfvARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC            glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC                      glGetInfoLogARB;
extern PFNGLGETATTACHEDOBJECTSARBPROC              glGetAttachedObjectsARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC              glGetUniformLocationARB;
extern PFNGLGETACTIVEUNIFORMARBPROC                glGetActiveUniformARB;
extern PFNGLGETUNIFORMFVARBPROC                    glGetUniformfvARB;
extern PFNGLGETUNIFORMIVARBPROC                    glGetUniformivARB;
extern PFNGLGETSHADERSOURCEARBPROC                 glGetShaderSourceARB;

extern PFNGLBINDATTRIBLOCATIONARBPROC     glBindAttribLocationARB;
extern PFNGLGETACTIVEATTRIBARBPROC        glGetActiveAttribARB;
extern PFNGLGETATTRIBLOCATIONARBPROC      glGetAttribLocationARB;


#endif
