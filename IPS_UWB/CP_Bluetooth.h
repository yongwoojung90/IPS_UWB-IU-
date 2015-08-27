#ifndef __IU_BTCOMM_H__
#define __IU_BTCOMM_H__

#include <stdio.h>
#include <initguid.h>
// Link to ws2_32.lib
#include <winsock2.h>
#include <ws2bth.h>
#include "CP_Definition.h"

//
#include <string.h>


//��.. �� �ؿ��� �� �̻ڰ� �Ҽ��ִ� ��� ã��
/////////////////////////////////////////////////////
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);
#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15
/////////////////////////////////////////////////////





typedef struct _YWstruct{

	char BT_Name[20];		// Tag's Bluetooth name
	char BT_Addr[17 + 1];	// Tag's Bluetooth MAC Address ( e.g  "00:19:01:37:BF:2E\0" )
	char str[CP_RECV_BUF_LENGTH];			// temporary member
	float distance_1;		// distance from Anchor1 to Tag
	float distance_2;		// distance from Anchor2 to Tag
	float distance_3;		// distance from Anchor3 to Tag
	float width = 0.0;		int cnt_width = 0;
	float height = 0.0;		int cnt_height = 0;
	float diagonal = 0.0;	int cnt_diagonal = 0;
	int WSA_ErrorCode;		//Windows Socket Error Code, return value of WSAGetLastError();
	int IU_ErrorCode;				//return value

	HWND hWndMain;
	HDC hDCMain;
	HANDLE hFile;
	bool keys[256];
	int draw_flag = 0;
	int flag = 0;
	wchar_t error_msg[20];
}YWstruct; // yongwoo struct �ϴ� �۸��� �� �����ȳ���/ �̸� �ٲ���!

//DWORD WINAPI DrawTrilateration(LPVOID lpParam);

//Functions for BlueTooth Communication and Get ToF data
ULONG NameToBthAddr(const char * pszRemoteName, BTH_ADDR * pRemoteBthAddr);
ULONG AddrStringToBtAddr(IN const char * pszRemoteAddr, OUT BTH_ADDR * pRemoteBtAddr);


DWORD WINAPI BluetoothThread(LPVOID hWnd);

//////////////////////////////////////
//////////////////////////////////////




//////////////////////////////////////
//void flushBuffer(char* buffer, int bufferSize);
//////////////////////////////////////

#endif