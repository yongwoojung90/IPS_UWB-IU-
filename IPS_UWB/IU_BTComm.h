#ifndef __IU_BTCOMM_H__
#define __IU_BTCOMM_H__

#include <stdio.h>
#include <initguid.h>
// Link to ws2_32.lib
#include <winsock2.h>
#include <ws2bth.h>

typedef char ToF;
ToF* ptrToF; //������... �̷��� �������� ������ GetToF���� ���� pToF�� runClient���� ���� ���� ����ü�� �������... ���������� ���ڸ� �����ϴ����ؾ���..�� �ʹ� ����
//��.. �� �ؿ��� �� �̻ڰ� �Ҽ��ִ� ��� ã��
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

}YWstruct; // yongwoo struct �ϴ� �۸��� �� �����ȳ���/ �̸� �ٲ���!



//Functions for BlueTooth Communication
ULONG NameToBthAddr(const char * pszRemoteName, BTH_ADDR * pRemoteBthAddr);
ULONG AddrStringToBtAddr(IN const char * pszRemoteAddr, OUT BTH_ADDR * pRemoteBtAddr);
ULONG RunClientMode(ULONGLONG ululRemoteBthAddr, YWstruct* ywStruct);


#endif