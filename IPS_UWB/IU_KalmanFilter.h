#ifndef __KF_H__
#define __KF_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


float KalmanFilter(int AnchorNumber, float distance, float Q = 0.0071f, float R = 0.608f);


#endif