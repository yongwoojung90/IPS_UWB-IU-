#include "CP_Filter.h"

static float dataBuf[4][BUFFER_LENGTH] = { 0, };

void shift_buf(float newData, float* dataBuf)
{
	int i = 0;
	float temp = 0.0;
	for (i = 1; i < BUFFER_LENGTH; i++){
		temp = dataBuf[i];
		dataBuf[i - 1] = temp;
	}
	dataBuf[i - 1] = newData;
}

float cpMovingAverageFilter(float inputData, int anchorNo, float* weightArr)
{
	float defualtWeightArr[BUFFER_LENGTH + 1] = { 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.07f, 0.08f, 0.13f, 0.18f, 0.24f };
	float retVal = 0.0;
	int i = 0;

	if (weightArr == NULL){ weightArr = defualtWeightArr; }

	if (dataBuf[anchorNo][0] != 0.0){
		for (i = 0; i < BUFFER_LENGTH; i++){
			retVal += (weightArr[i] * dataBuf[anchorNo][i]);
		}
		retVal += weightArr[i] * inputData;
		shift_buf(retVal, dataBuf[anchorNo]);
	}
	else{
		shift_buf(inputData, dataBuf[anchorNo]);
	}
	return retVal;
}




float cpKalmanFilter(int AnchorNumber, float distance, float Q, float R)
{
	static float xhat[4] = { 0.0f, };
	static float P[4] = { 0.0f, };
	static float xhatbar[4] = { 0.0f, };
	static float Pbar[4] = { 0.0f, };
	static float K[4] = { 0.0f, };

	static float xhat_last[4] = { 0.0f, };
	static float P_last[4] = { 1, 1, 1, 1 };

	xhatbar[AnchorNumber] = xhat_last[AnchorNumber];
	Pbar[AnchorNumber] = P_last[AnchorNumber] + Q;

	K[AnchorNumber] = Pbar[AnchorNumber] / (Pbar[AnchorNumber] + R);
	xhat[AnchorNumber] = xhatbar[AnchorNumber] + (K[AnchorNumber] * (distance - xhatbar[AnchorNumber]));
	P[AnchorNumber] = (1 - K[AnchorNumber]) * Pbar[AnchorNumber];

	xhat_last[AnchorNumber] = xhat[AnchorNumber];
	P_last[AnchorNumber] = P[AnchorNumber];

	return xhat[AnchorNumber];
}

