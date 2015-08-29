#ifndef __CP_DRAW_H__
#define __CP_DRAW_H__

#include "CP_Definition.h"
#include <gl\GL.h>
#include <gl\GLU.h>

GLvoid KillGLWindow();
BOOL CreateGLWindow(wchar_t* title, int width, int height, int bits, bool fullscreenflag);
int DrawGLScene();
int InitGL();
GLvoid ReSizeGLScene(GLsizei width, GLsizei height);

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif


#endif