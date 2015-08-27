#ifndef __CP_FILTER_H__
#define __CP_FILTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CP_Bluetooth.h"
#include "CP_Definition.h"

//
CpToF cpParsingAndGetToF(char* strSrcData);

//
CpToF cpKalmanFilter(CpToF ToF, float Q = 0.0071f, float R = 0.608f);

//
#define CP_FILTER_BUFFER_LENGTH 9

//
void cpShiftBuf(float newData, float* buf);

//
CpToF cpMovingAverageFilter(CpToF ToF, float* weightArr = NULL);

//transform raw ToF(or filtered ToF) into real distance(centi-meter, cm)
CpRealDistance cpToFtoRealdistance(CpToF ToF); 

#endif