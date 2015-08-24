#include "CP_Filter.h"

static float dataBuf[4][BUFFER_LENGTH] = { 0, };

void cpShiftBuf(float newData, float* dataBuf)
{
	int i = 0;
	float temp = 0.0;
	for (i = 1; i < BUFFER_LENGTH; i++){
		temp = dataBuf[i];
		dataBuf[i - 1] = temp;
	}
	dataBuf[i - 1] = newData;
}

float cpMovingAverageFilter(float inputData, int AnchorID, float* weightArr)
{
	float defualtWeightArr[BUFFER_LENGTH + 1] = { 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.07f, 0.08f, 0.13f, 0.18f, 0.24f };
	float retVal = 0.0;
	int i = 0;

	if (weightArr == NULL){ weightArr = defualtWeightArr; }

	if (dataBuf[AnchorID][0] != 0.0){
		for (i = 0; i < BUFFER_LENGTH; i++){
			retVal += (weightArr[i] * dataBuf[AnchorID][i]);
		}
		retVal += weightArr[i] * inputData;
		cpShiftBuf(retVal, dataBuf[AnchorID]);
	}
	else{
		cpShiftBuf(inputData, dataBuf[AnchorID]);
	}
	return retVal;
}

float cpKalmanFilter(float inputData, int AnchorID, float Q, float R)
{
	static float xhat[4] = { 0.0f, };
	static float P[4] = { 0.0f, };
	static float xhatbar[4] = { 0.0f, };
	static float Pbar[4] = { 0.0f, };
	static float K[4] = { 0.0f, };

	static float xhat_last[4] = { 0.0f, };
	static float P_last[4] = { 1, 1, 1, 1 };

	xhatbar[AnchorID] = xhat_last[AnchorID];
	Pbar[AnchorID] = P_last[AnchorID] + Q;

	K[AnchorID] = Pbar[AnchorID] / (Pbar[AnchorID] + R);
	xhat[AnchorID] = xhatbar[AnchorID] + (K[AnchorID] * (inputData - xhatbar[AnchorID]));
	P[AnchorID] = (1 - K[AnchorID]) * Pbar[AnchorID];

	xhat_last[AnchorID] = xhat[AnchorID];
	P_last[AnchorID] = P[AnchorID];

	return xhat[AnchorID];
}

