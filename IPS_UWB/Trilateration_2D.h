#ifndef __TRI_2D_H__
#define __TRI_2D_H__
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <math.h>
#include "CP_Bluetooth.h"

void TriThread(FloatArray* QubeSize);
CvPoint Trilateration_2D(CvPoint Anchor1, CvPoint Anchor2, CvPoint Anchor3, double r1, double r2, double r3);
double square(double x);
void on_mouseEvent(int event, int x, int y, int flag, void* param);
double getDistance(CvPoint center, CvPoint tag);

#endif