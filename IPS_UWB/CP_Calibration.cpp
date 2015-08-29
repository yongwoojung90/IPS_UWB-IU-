#include "CP_Calibration.h"

CpQubeSize cpCalibration(CpRealDistance RealDistance)
{
	static int cntDiscard = 0;
	static int cntSave = 0;
	static float width = 0.0f;		//가로
	static float length = 0.0f;		//세로
	//static float height = 0.0f;		//높이
	
	CpQubeSize retVal = { 0.0 };

	if (cntDiscard < CP_CALIBRATION_DISCARD){
		cntDiscard++;
	}
	else if (CP_CALIBRATION_DISCARD <= cntDiscard && cntDiscard < CP_CALIBRATION_SAVE){
		width += RealDistance.Anchor[2];
		length += RealDistance.Anchor[3];
		cntDiscard++;
		cntSave++;
	}
	else{
		retVal.height = CP_CALIBRATION_COMPLETE;
		retVal.width = width / (float)cntSave;
		retVal.length = length / (float)cntSave;
		cntDiscard = 0;
		cntSave = 0;
	}
	return retVal;
}