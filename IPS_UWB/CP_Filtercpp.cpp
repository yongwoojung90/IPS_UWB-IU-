#include "CP_Filter.h"
#include "CP_Bluetooth.h"



CpToF cpParsingAndGetToF(char* strSrcData){

	int indexToStart = 0; // index to start of packet
	int anchorID = 0;
	char* pStrSrcData = strSrcData;
	int flag = 0;
	char distFromAnchor[4][10] = { '\0', };
	int index = 0;
	int len = 0;

	CpToF ToF = { 0.0f, };

	//search start character( '*' or '#' )
	for (indexToStart = 0; indexToStart < strlen(strSrcData); indexToStart++){
		if (*pStrSrcData == '*'){
			flag = CP_MODE_TRILATERATION;
			break;
		}
		else if (*pStrSrcData == '#'){
			flag = CP_MODE_CALIBRATION;
			break;
		}
		pStrSrcData++;
	}


	if (flag == 0)
		return ToF;		//받은 데이터에 '*'또는 '!' , '@', '#'가 없으므로 parsing하지 않고 나간다.
	else {
		anchorID = 1;
		index = 0;
		len = strlen(strSrcData) - indexToStart;
	}


	for (int i = 0; i < len && flag != 0; i++){
		switch (*pStrSrcData)
		{
		case ',': //tokenizer
			anchorID += 1;
			index = 0;
			break;
		case '=': // '=' means end of data
			//Transform ToF into real distance(cm)
			anchorID = 1;
			index = 0;
			if (flag == CP_MODE_TRILATERATION){
				ToF.Anchor[0] = CP_MODE_TRILATERATION;
				for (int j = 1; j <= CP_TOTAL_ANCHOR_NUMBER; j++){
					ToF.Anchor[j] = atof(distFromAnchor[j]);
				}
				return ToF; //사실 for문 조건에 flag == 1 도 없에고 이 바로윗줄에서 flag = 0;도 없에도 return; 때문에 나가진다.
			}
			else if (flag == CP_MODE_CALIBRATION){
				ToF.Anchor[0] = CP_MODE_CALIBRATION;
				ToF.Anchor[2] = atof(distFromAnchor[2]);
				ToF.Anchor[3] = atof(distFromAnchor[3]);
				return ToF;
			}
		default:
			if (('0' <= *pStrSrcData && *pStrSrcData <= '9') || *pStrSrcData == '.'){ //when receive error free data
				distFromAnchor[anchorID][index] = *pStrSrcData;
				index += 1;
			}
			else{
				//TO DO
				//통신과정에서 Data에 노이즈가 발생해서 (error free 하지 못하게) 
				//숫자 이외의 값이 수신되면 이 데이터는 쓰면 안된다.
				//그런데.. 그럴 일이 없을 거같긴해서 실제로 구현하지는 않는다.
				break;
			}
		}
		pStrSrcData++;
	}
}


//float cpMovingAverageFilter(float inputData, int AnchorID, float* weightArr)
//{
//	float defualtWeightArr[BUFFER_LENGTH + 1] = { 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.07f, 0.08f, 0.13f, 0.18f, 0.24f };
//	float retVal = 0.0;
//	int i = 0;
//
//	if (weightArr == NULL){ weightArr = defualtWeightArr; }
//
//	if (dataBuf[AnchorID][0] != 0.0){
//		for (i = 0; i < BUFFER_LENGTH; i++){
//			retVal += (weightArr[i] * dataBuf[AnchorID][i]);
//		}
//		retVal += weightArr[i] * inputData;
//		cpShiftBuf(retVal, dataBuf[AnchorID]);
//	}
//	else{
//		cpShiftBuf(inputData, dataBuf[AnchorID]);
//	}
//	return retVal;
//}

CpToF cpMovingAverageFilter(CpToF ToF, float* weightArr)
{
	static float dataBuf[4][CP_FILTER_BUFFER_LENGTH] = { 0, };
	float defualtWeightArr[CP_FILTER_BUFFER_LENGTH + 1] = { 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.07f, 0.08f, 0.13f, 0.18f, 0.24f };
	CpToF retVal = { 0.0f, };
	int i = 0;

	if (weightArr == NULL){ weightArr = defualtWeightArr; }
	for (int AnchorID = 1; AnchorID < 4; AnchorID++){
		if (dataBuf[AnchorID][0] != 0.0){
			for (i = 0; i < CP_FILTER_BUFFER_LENGTH; i++){
				retVal.Anchor[AnchorID] += (weightArr[i] * dataBuf[AnchorID][i]);
			}
			retVal.Anchor[AnchorID] += weightArr[i] * ToF.Anchor[AnchorID];
			cpShiftBuf(retVal.Anchor[AnchorID], dataBuf[AnchorID]);
		}
		else{
			cpShiftBuf(ToF.Anchor[AnchorID], dataBuf[AnchorID]);
		}
	}

	retVal.Anchor[0] = ToF.Anchor[0];
	return retVal;
}

void cpShiftBuf(float newData, float* dataBuf)
{
	int i = 0;
	float temp = 0.0;
	for (i = 1; i < CP_FILTER_BUFFER_LENGTH; i++){
		temp = dataBuf[i];
		dataBuf[i - 1] = temp;
	}
	dataBuf[i - 1] = newData;
}
//float cpKalmanFilter(float inputData, int AnchorID, float Q, float R)
//{
//	static float xhat[4] = { 0.0f, };
//
//	static float P[4] = { 0.0f, };
//	static float xhatbar[4] = { 0.0f, };
//	static float Pbar[4] = { 0.0f, };
//	static float K[4] = { 0.0f, };
//
//	static float xhat_last[4] = { 0.0f, };
//	static float P_last[4] = { 1, 1, 1, 1 };
//
//	xhatbar[AnchorID] = xhat_last[AnchorID];
//	Pbar[AnchorID] = P_last[AnchorID] + Q;
//
//	K[AnchorID] = Pbar[AnchorID] / (Pbar[AnchorID] + R);
//	xhat[AnchorID] = xhatbar[AnchorID] + (K[AnchorID] * (inputData - xhatbar[AnchorID]));
//	P[AnchorID] = (1 - K[AnchorID]) * Pbar[AnchorID];
//
//	xhat_last[AnchorID] = xhat[AnchorID];
//	P_last[AnchorID] = P[AnchorID];
//
//	return xhat[AnchorID];
//}

CpToF cpKalmanFilter(CpToF ToF, float Q, float R)
{
	//static float xhat[4] = { 0.0f };

	static float P[4] = { 0.0f, };
	static float xhatbar[4] = { 0.0f, };
	static float Pbar[4] = { 0.0f, };
	static float K[4] = { 0.0f, };

	static float xhat_last[4] = { 0.0f, };
	static float P_last[4] = { 1, 1, 1, 1 };

	int AnchorID = 0;
	CpToF xhat = { 0.0f }; 
	for (AnchorID = 1; AnchorID < 4; AnchorID++){
		xhatbar[AnchorID] = xhat_last[AnchorID];
		Pbar[AnchorID] = P_last[AnchorID] + Q;

		K[AnchorID] = Pbar[AnchorID] / (Pbar[AnchorID] + R);
		xhat.Anchor[AnchorID] = xhatbar[AnchorID] + (K[AnchorID] * (ToF.Anchor[AnchorID] - xhatbar[AnchorID]));
		P[AnchorID] = (1 - K[AnchorID]) * Pbar[AnchorID];

		xhat_last[AnchorID] = xhat.Anchor[AnchorID];
		P_last[AnchorID] = P[AnchorID];
	}

	xhat.Anchor[0] = ToF.Anchor[0];
	return xhat;
}

CpRealDistance cpToFtoRealdistance(CpToF ToF)
{
	CpRealDistance retVal = { 0.0f };
	int AnchorID = 0;
	//float 기울기 = 84.896;
	//float 상수항 = 35.1868;

	for (AnchorID = 1; AnchorID < 4; AnchorID++){
		retVal.Anchor[AnchorID] = 84.896*ToF.Anchor[AnchorID] - 35.1868;
	}

	retVal.Anchor[0] = ToF.Anchor[0];
	return retVal;
}