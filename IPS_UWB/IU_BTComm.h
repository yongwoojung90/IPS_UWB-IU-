#ifndef __IU_BTCOMM_H__
#define __IU_BTCOMM_H__

#include <stdio.h>
#include <initguid.h>
// Link to ws2_32.lib
#include <winsock2.h>
#include <ws2bth.h>
#include "IU_DEF.h"

//
#include <string.h>


//하.. 요 밑에꺼 좀 이쁘게 할수있는 방법 찾자
/////////////////////////////////////////////////////
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);


#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15


/////////////////////////////////////////////////////

typedef struct _YWstruct{

	char BT_Name[20];		// Tag's Bluetooth name
	char BT_Addr[17 + 1];	// Tag's Bluetooth MAC Address ( e.g  "00:19:01:37:BF:2E\0" )
	char str[IU_MIDDLE_BUFFER_LENGTH];			// temporary member
	float distance_1;		// distance from Anchor1 to Tag
	float distance_2;		// distance from Anchor2 to Tag
	float distance_3;		// distance from Anchor3 to Tag

	int WSA_ErrorCode;		//Windows Socket Error Code, return value of WSAGetLastError();
	int IU_ErrorCode;				//return value

	HWND hWndMain;
	HDC hDCMain;
	HANDLE hFile;
	bool keys[256];
	int flag = 0;
	wchar_t error_msg[20];
}YWstruct; // yongwoo struct 일단 작명할 꺼 생각안나서/ 이름 바꾸자!

DWORD WINAPI GetToF(LPVOID ywStruct);
//DWORD WINAPI DrawTrilateration(LPVOID lpParam);

//Functions for BlueTooth Communication
ULONG NameToBthAddr(const char * pszRemoteName, BTH_ADDR * pRemoteBthAddr);
ULONG AddrStringToBtAddr(IN const char * pszRemoteAddr, OUT BTH_ADDR * pRemoteBtAddr);
ULONG RunClientMode(ULONGLONG ululRemoteBthAddr, YWstruct* ywStruct);


//////////////////////////////////////
void parsing(char* string, YWstruct* ywStruct);
typedef struct __distance{
	double anchor1;
	double anchor2;
	double anchor3;
	double anchor4;
}distance;
//////////////////////////////////////




//////////////////////////////////////
void flushBuffer(char* buffer, int bufferSize);
//////////////////////////////////////

#endif