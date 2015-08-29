#ifndef __TRILATAERATION_3D_H__
#define __TRILATAERATION_3D_H__

#include <Windows.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
//#include <gl\glaux.h>		// Header File For The Glaux Library
#include "CP_Definition.h"

typedef struct _ywPos{
	float x;
	float y;
	float z;
}ywPos;

ywPos calcTagPosition(CpQubeSize QubeSize, CpRealDistance RealDistance);

#endif