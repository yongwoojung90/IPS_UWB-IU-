#ifndef __DEFINITION_H__
#define __DEFINITION_H__

#include <Windows.h>

#define WM_CP_DRAW_QUBE WM_USER+30
#define WM_CP_DRAW_TAG WM_USER+31

#define CP_PACKET_LENGTH			25		// length of the Cicada Project Protocol's packet :  *00.0000,00.0000,00.0000=
#define CP_RECV_BUF_LENGTH			CP_PACKET_LENGTH*2 // 50bytes-bluetooth receive buffer length
#define CP_BT_ADDR_LEN				17+1	//Cicada Project Bluetooth Address Length, 6 two-digit hex values plus 5 colons and null charactor '\0'

#define CP_TOTAL_ANCHOR_NUMBER 3

#define CP_CALIBRATION_DISCARD 50
#define CP_CALIBRATION_SAVE 100
#define CP_CALIBRATION_COMPLETE -9999

#define CP_MODE_TRILATERATION 1
#define CP_MODE_CALIBRATION 2

//BlueToothConnectionMode
//1 - using tag's bluetooth name, 2 - using tag's bluetooth address
#define CP_BLUETOOTH_USING_NAME 1
#define CP_BLUETOOTH_USING_ADDRESS 2

typedef struct _FloatArray{
	float Anchor[4];
	FloatArray();
	FloatArray(CpQubeSize qs)
	{
		Anchor[1] = qs.width;
		Anchor[2] = qs.length;
		Anchor[3] = qs.height;
	}
	operator WPARAM() const { return WPARAM(); }
}FloatArray;

typedef FloatArray CpToF;
typedef FloatArray CpRealDistance;

typedef struct _CpQubeSize{
	float width;
	float length;
	float height;
	CpQubeSize();
	CpQubeSize(FloatArray fa)
	{
		width = fa.Anchor[1];
		length = fa.Anchor[2];
		height = fa.Anchor[3];
	}

	operator WPARAM() const { return WPARAM(); }
}CpQubeSize;

#endif
