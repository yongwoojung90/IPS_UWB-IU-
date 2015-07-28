#ifndef __IU_BTCOMM_H__
#define __IU_BTCOMM_H__

#include <stdio.h>
#include <initguid.h>
// Link to ws2_32.lib
#include <winsock2.h>
#include <ws2bth.h>

typedef char ToF;
ToF* ptrToF; //개구림... 이렇게 전역으로 떄려서 GetToF에서 받은 pToF를 runClient에게 주지 말고 구조체를 만들던지... 내부적으로 인자를 전달하던지해야지..ㅠ 너무 구림
//하.. 요 밑에꺼 좀 이쁘게 할수있는 방법 찾자
/////////////////////////////////////////////////////
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);

#define CXN_BDADDR_STR_LEN                17   // 6 two-digit hex values plus 5 colons
#define CXN_TRANSFER_DATA_LENGTH          40  // length of the data to be transferred
#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15

char g_szRemoteName[BTH_MAX_NAME_SIZE + 1] = { 0 };  // 1 extra for trailing NULL character
char g_szRemoteAddr[CXN_BDADDR_STR_LEN + 1] = { 0 }; // 1 extra for trailing NULL character
// This just redundant!!!!
int  g_ulMaxCxnCycles = 1, g_iOutputLevel = 0;
/////////////////////////////////////////////////////

typedef struct _YWstruct{

	char BTD_Name[20];		//Tag's Bluetooth device name
	char str[100];			//temporary member
	float Dist_1;			//distance from Anchor1 to Tag
	float Dist_2;			//distance from Anchor2 to Tag
	float Dist_3;			//distance from Anchor3 to Tag

}YWstruct; // yongwoo struct 일단 작명할 꺼 생각안나서/ 이름 바꾸자!



//Functions for BlueTooth Communication
ULONG NameToBthAddr(const char * pszRemoteName, BTH_ADDR * pRemoteBthAddr);
ULONG AddrStringToBtAddr(IN const char * pszRemoteAddr, OUT BTH_ADDR * pRemoteBtAddr);
ULONG RunClientMode(ULONGLONG ululRemoteBthAddr, YWstruct* ywStruct);


#endif