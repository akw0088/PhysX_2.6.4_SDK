#ifndef _XENON_GLU_H_
#define _XENON_GLU_H_

#include <GL/gl.h>

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz);
void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);

int gluUnProject(GLdouble winx, GLdouble winy,GLdouble winz, 
				 const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], 
				 GLdouble *objx, GLdouble *objy, GLdouble *objz);

int gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
				const GLdouble  modelMatrix[16], const GLdouble  projMatrix[16], const GLint viewport[4], 
                GLdouble *winx, GLdouble *winy, GLdouble *winz);

#endif	_XENON_GLU_H_
