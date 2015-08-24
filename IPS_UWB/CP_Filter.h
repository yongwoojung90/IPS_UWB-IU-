#ifndef __CP_FILTER_H__
#define __CP_FILTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CP_ANCHOR_1 1
#define CP_ANCHOR_2 2
#define CP_ANCHOR_3 3


float cpKalmanFilter(int AnchorNumber, float distance, float Q = 0.0071f, float R = 0.608f);

////////////////Moving Average Filter Function////////////////////////
#define BUFFER_LENGTH 9
void shift_buf(float newData, float* buf);
float cpMovingAverageFilter(float inputData, int anchorNo, float* weightArr = NULL);
//////////////////////////////////////////////////////////////////////


#endif