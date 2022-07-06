
#include <GL/gl.h>
#include <GL/glu.h>

#define GLUTCALLBACK

/* Display mode bit masks. */
#define GLUT_RGB			0
#define GLUT_RGBA			GLUT_RGB
#define GLUT_INDEX			1
#define GLUT_SINGLE			0
#define GLUT_DOUBLE			2
#define GLUT_ACCUM			4
#define GLUT_ALPHA			8
#define GLUT_DEPTH			16
#define GLUT_STENCIL			32
#define GLUT_MULTISAMPLE		128
#define GLUT_STEREO			256
#define GLUT_LUMINANCE			512

/* Mouse buttons. */
#define GLUT_LEFT_BUTTON		0
#define GLUT_MIDDLE_BUTTON		1
#define GLUT_RIGHT_BUTTON		2

/* Mouse button  state. */
#define GLUT_DOWN			0
#define GLUT_UP				1

/* function keys */
#define GLUT_KEY_F1			1
#define GLUT_KEY_F2			2
#define GLUT_KEY_F3			3
#define GLUT_KEY_F4			4
#define GLUT_KEY_F5			5
#define GLUT_KEY_F6			6
#define GLUT_KEY_F7			7
#define GLUT_KEY_F8			8
#define GLUT_KEY_F9			9
#define GLUT_KEY_F10			10
#define GLUT_KEY_F11			11
#define GLUT_KEY_F12			12

/* directional keys */
#define GLUT_KEY_LEFT			100
#define GLUT_KEY_UP			101
#define GLUT_KEY_RIGHT			102
#define GLUT_KEY_DOWN			103
#define GLUT_KEY_PAGE_UP		104
#define GLUT_KEY_PAGE_DOWN		105
#define GLUT_KEY_HOME			106
#define GLUT_KEY_END			107
#define GLUT_KEY_INSERT			108

/* Entry/exit  state. */
#define GLUT_LEFT			0
#define GLUT_ENTERED			1

/* Menu usage  state. */
#define GLUT_MENU_NOT_IN_USE		0
#define GLUT_MENU_IN_USE		1

/* Visibility  state. */
#define GLUT_NOT_VISIBLE		0
#define GLUT_VISIBLE			1

/* Window status  state. */
#define GLUT_HIDDEN			0
#define GLUT_FULLY_RETAINED		1
#define GLUT_PARTIALLY_RETAINED		2
#define GLUT_FULLY_COVERED		3

/* Color index component selection values. */
#define GLUT_RED			0
#define GLUT_GREEN			1
#define GLUT_BLUE			2

/* glutGet parameters. */
#define GLUT_WINDOW_X			((GLenum) 100)
#define GLUT_WINDOW_Y			((GLenum) 101)
#define GLUT_WINDOW_WIDTH		((GLenum) 102)
#define GLUT_WINDOW_HEIGHT		((GLenum) 103)
#define GLUT_WINDOW_BUFFER_SIZE		((GLenum) 104)
#define GLUT_WINDOW_STENCIL_SIZE	((GLenum) 105)
#define GLUT_WINDOW_DEPTH_SIZE		((GLenum) 106)
#define GLUT_WINDOW_RED_SIZE		((GLenum) 107)
#define GLUT_WINDOW_GREEN_SIZE		((GLenum) 108)
#define GLUT_WINDOW_BLUE_SIZE		((GLenum) 109)
#define GLUT_WINDOW_ALPHA_SIZE		((GLenum) 110)
#define GLUT_WINDOW_ACCUM_RED_SIZE	((GLenum) 111)
#define GLUT_WINDOW_ACCUM_GREEN_SIZE	((GLenum) 112)
#define GLUT_WINDOW_ACCUM_BLUE_SIZE	((GLenum) 113)
#define GLUT_WINDOW_ACCUM_ALPHA_SIZE	((GLenum) 114)
#define GLUT_WINDOW_DOUBLEBUFFER	((GLenum) 115)
#define GLUT_WINDOW_RGBA		((GLenum) 116)
#define GLUT_WINDOW_PARENT		((GLenum) 117)
#define GLUT_WINDOW_NUM_CHILDREN	((GLenum) 118)
#define GLUT_WINDOW_COLORMAP_SIZE	((GLenum) 119)
#define GLUT_WINDOW_NUM_SAMPLES		((GLenum) 120)
#define GLUT_WINDOW_STEREO		((GLenum) 121)
#define GLUT_WINDOW_CURSOR		((GLenum) 122)
#define GLUT_SCREEN_WIDTH		((GLenum) 200)
#define GLUT_SCREEN_HEIGHT		((GLenum) 201)
#define GLUT_SCREEN_WIDTH_MM		((GLenum) 202)
#define GLUT_SCREEN_HEIGHT_MM		((GLenum) 203)
#define GLUT_MENU_NUM_ITEMS		((GLenum) 300)
#define GLUT_DISPLAY_MODE_POSSIBLE	((GLenum) 400)
#define GLUT_INIT_WINDOW_X		((GLenum) 500)
#define GLUT_INIT_WINDOW_Y		((GLenum) 501)
#define GLUT_INIT_WINDOW_WIDTH		((GLenum) 502)
#define GLUT_INIT_WINDOW_HEIGHT		((GLenum) 503)
#define GLUT_INIT_DISPLAY_MODE		((GLenum) 504)
#define GLUT_ELAPSED_TIME		((GLenum) 700)

#define GLUT_WINDOW_FORMAT_ID		((GLenum) 123)

/* glutDeviceGet parameters. */
#define GLUT_HAS_KEYBOARD		((GLenum) 600)
#define GLUT_HAS_MOUSE			((GLenum) 601)
#define GLUT_HAS_SPACEBALL		((GLenum) 602)
#define GLUT_HAS_DIAL_AND_BUTTON_BOX	((GLenum) 603)
#define GLUT_HAS_TABLET			((GLenum) 604)
#define GLUT_NUM_MOUSE_BUTTONS		((GLenum) 605)
#define GLUT_NUM_SPACEBALL_BUTTONS	((GLenum) 606)
#define GLUT_NUM_BUTTON_BOX_BUTTONS	((GLenum) 607)
#define GLUT_NUM_DIALS			((GLenum) 608)
#define GLUT_NUM_TABLET_BUTTONS		((GLenum) 609)
#define GLUT_DEVICE_IGNORE_KEY_REPEAT   ((GLenum) 610)
#define GLUT_DEVICE_KEY_REPEAT          ((GLenum) 611)
#define GLUT_HAS_JOYSTICK		((GLenum) 612)
#define GLUT_OWNS_JOYSTICK		((GLenum) 613)
#define GLUT_JOYSTICK_BUTTONS		((GLenum) 614)
#define GLUT_JOYSTICK_AXES		((GLenum) 615)
#define GLUT_JOYSTICK_POLL_RATE		((GLenum) 616)

/* glutLayerGet parameters. */
#define GLUT_OVERLAY_POSSIBLE           ((GLenum) 800)
#define GLUT_LAYER_IN_USE		((GLenum) 801)
#define GLUT_HAS_OVERLAY		((GLenum) 802)
#define GLUT_TRANSPARENT_INDEX		((GLenum) 803)
#define GLUT_NORMAL_DAMAGED		((GLenum) 804)
#define GLUT_OVERLAY_DAMAGED		((GLenum) 805)

/* glutVideoResizeGet parameters. */
#define GLUT_VIDEO_RESIZE_POSSIBLE	((GLenum) 900)
#define GLUT_VIDEO_RESIZE_IN_USE	((GLenum) 901)
#define GLUT_VIDEO_RESIZE_X_DELTA	((GLenum) 902)
#define GLUT_VIDEO_RESIZE_Y_DELTA	((GLenum) 903)
#define GLUT_VIDEO_RESIZE_WIDTH_DELTA	((GLenum) 904)
#define GLUT_VIDEO_RESIZE_HEIGHT_DELTA	((GLenum) 905)
#define GLUT_VIDEO_RESIZE_X		((GLenum) 906)
#define GLUT_VIDEO_RESIZE_Y		((GLenum) 907)
#define GLUT_VIDEO_RESIZE_WIDTH		((GLenum) 908)
#define GLUT_VIDEO_RESIZE_HEIGHT	((GLenum) 909)

/* glutUseLayer parameters. */
#define GLUT_NORMAL			((GLenum) 0)
#define GLUT_OVERLAY			((GLenum) 1)

/* glutGetModifiers return mask. */
#define GLUT_ACTIVE_SHIFT               1
#define GLUT_ACTIVE_CTRL                2
#define GLUT_ACTIVE_ALT                 4

/* glutSetCursor parameters. */
/* Basic arrows. */
#define GLUT_CURSOR_RIGHT_ARROW		0
#define GLUT_CURSOR_LEFT_ARROW		1
/* Symbolic cursor shapes. */
#define GLUT_CURSOR_INFO		2
#define GLUT_CURSOR_DESTROY		3
#define GLUT_CURSOR_HELP		4
#define GLUT_CURSOR_CYCLE		5
#define GLUT_CURSOR_SPRAY		6
#define GLUT_CURSOR_WAIT		7
#define GLUT_CURSOR_TEXT		8
#define GLUT_CURSOR_CROSSHAIR		9
/* Directional cursors. */
#define GLUT_CURSOR_UP_DOWN		10
#define GLUT_CURSOR_LEFT_RIGHT		11
/* Sizing cursors. */
#define GLUT_CURSOR_TOP_SIDE		12
#define GLUT_CURSOR_BOTTOM_SIDE		13
#define GLUT_CURSOR_LEFT_SIDE		14
#define GLUT_CURSOR_RIGHT_SIDE		15
#define GLUT_CURSOR_TOP_LEFT_CORNER	16
#define GLUT_CURSOR_TOP_RIGHT_CORNER	17
#define GLUT_CURSOR_BOTTOM_RIGHT_CORNER	18
#define GLUT_CURSOR_BOTTOM_LEFT_CORNER	19
/* Inherit from parent window. */
#define GLUT_CURSOR_INHERIT		100
/* Blank cursor. */
#define GLUT_CURSOR_NONE		101
/* Fullscreen crosshair (if available). */
#define GLUT_CURSOR_FULL_CROSSHAIR	102




/* GLUT initialization sub-API. */
void glutInit(int *argcp, char **argv);
void glutInitDisplayMode(unsigned int mode);
//void glutInitDisplayString(const char *string);
//void glutInitWindowPosition(int x, int y);
void glutInitWindowSize(int width, int height);
void glutMainLoop(void);

/* GLUT window sub-API. */
int glutCreateWindow(const char *title);
//int glutCreateSubWindow(int win, int x, int y, int width, int height);
//void glutDestroyWindow(int win);
void glutPostRedisplay(void);
//void glutPostWindowRedisplay(int win);
void glutSwapBuffers(void);
//int glutGetWindow(void);
void glutSetWindow(int win);
void glutSetWindowTitle(const char *title);
//void glutSetIconTitle(const char *title);
//void glutPositionWindow(int x, int y);
//void glutReshapeWindow(int width, int height);
//void glutPopWindow(void);
//void glutPushWindow(void);
//void glutIconifyWindow(void);
//void glutShowWindow(void);
//void glutHideWindow(void);
//void glutFullScreen(void);
//void glutSetCursor(int cursor);
void glutWarpPointer(int x, int y);

/* GLUT window callback sub-API. */
void glutDisplayFunc(void (*func)(void));
void glutReshapeFunc(void (*func)(int width, int height));
void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
void glutMouseFunc(void (*func)(int button, int state, int x, int y));
void glutMotionFunc(void (*func)(int x, int y));
void glutPassiveMotionFunc(void (*func)(int x, int y));
void glutEntryFunc(void (*func)(int state));
//void glutVisibilityFunc(void (*func)(int state));
void glutIdleFunc(void (*func)(void));
//void glutTimerFunc(unsigned int millis, void (*func)(int value), int value);
//void glutMenuStateFunc(void (*func)(int state));
void glutSpecialFunc(void (*func)(int key, int x, int y));
//void glutSpaceballMotionFunc(void (*func)(int x, int y, int z));
//void glutSpaceballRotateFunc(void (*func)(int x, int y, int z));
//void glutSpaceballButtonFunc(void (*func)(int button, int state));
//void glutButtonBoxFunc(void (*func)(int button, int state));
//void glutDialsFunc(void (*func)(int dial, int value));
//void glutTabletMotionFunc(void (*func)(int x, int y));
//void glutTabletButtonFunc(void (*func)(int button, int state, int x, int y));
//void glutMenuStatusFunc(void (*func)(int status, int x, int y));
//void glutOverlayDisplayFunc(void (*func)(void));
//void glutWindowStatusFunc(void (*func)(int state));
void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y));
void glutSpecialUpFunc(void (*func)(int key, int x, int y));
//void glutJoystickFunc(void (*func)(unsigned int buttonMask, int x, int y, int z), int pollInterval);

/* GLUT color index sub-API. */
//void glutSetColor(int, GLfloat red, GLfloat green, GLfloat blue);
//GLfloat glutGetColor(int ndx, int component);
//void glutCopyColormap(int win);

/* GLUT state retrieval sub-API. */
int glutGet(GLenum type);
//int glutDeviceGet(GLenum type);
//int glutExtensionSupported(const char *name);
int glutGetModifiers(void);
//int glutLayerGet(GLenum type);

/* GLUT font sub-API */
//void glutBitmapCharacter(void *font, int character);
//int glutBitmapWidth(void *font, int character);
//void glutStrokeCharacter(void *font, int character);
//int glutStrokeWidth(void *font, int character);
//int glutBitmapLength(void *font, const unsigned char *string);
//int glutStrokeLength(void *font, const unsigned char *string);

/* GLUT pre-built models sub-API */
//void glutWireSphere(GLdouble radius, GLint slices, GLint stacks);
void glutSolidSphere(double radius, int slices, int stacks);
//void glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
//void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
//void glutWireCube(GLdouble size);
void glutSolidCube(double size);
//void glutWireTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings);
//void glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings);
//void glutWireDodecahedron(void);
//void glutSolidDodecahedron(void);
//void glutWireTeapot(GLdouble size);
//void glutSolidTeapot(GLdouble size);
//void glutWireOctahedron(void);
//void glutSolidOctahedron(void);
//void glutWireTetrahedron(void);
//void glutSolidTetrahedron(void);
//void glutWireIcosahedron(void);
//void glutSolidIcosahedron(void);

/* GLUT video resize sub-API. */
//int glutVideoResizeGet(GLenum param);
//void glutSetupVideoResizing(void);
//void glutStopVideoResizing(void);
//void glutVideoResize(int x, int y, int width, int height);
//void glutVideoPan(int x, int y, int width, int height);

/* GLUT debugging sub-API. */
//void glutReportErrors(void);

/* GLUT device control sub-API. */
/* glutSetKeyRepeat modes. */
#define GLUT_KEY_REPEAT_OFF		0
#define GLUT_KEY_REPEAT_ON		1
#define GLUT_KEY_REPEAT_DEFAULT		2

/* Joystick button masks. */
#define GLUT_JOYSTICK_BUTTON_A		1
#define GLUT_JOYSTICK_BUTTON_B		2
#define GLUT_JOYSTICK_BUTTON_C		4
#define GLUT_JOYSTICK_BUTTON_D		8

//void glutIgnoreKeyRepeat(int ignore);
//void glutSetKeyRepeat(int repeatMode);
//void glutForceJoystickFunc(void);

// Extensions to GLUT
void glutRemapButtonExt(unsigned int nButton, unsigned char key, bool shifted);
void glutExitExt(int arg);

