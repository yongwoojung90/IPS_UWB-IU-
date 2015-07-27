#ifndef __IU_BTCOMM_H__
#define __IU_BTCOMM_H__

#include <stdio.h>
#include <initguid.h>
// Link to ws2_32.lib
#include <winsock2.h>
#include <ws2bth.h>


ULONG NameToBthAddr(const char * pszRemoteName, BTH_ADDR * pRemoteBthAddr);
ULONG AddrStringToBtAddr(IN const char * pszRemoteAddr, OUT BTH_ADDR * pRemoteBtAddr);
// ULONG RunClientMode( ULONGLONG ululRemoteBthAddr,  int iMaxCxnCycles = 1);
ULONG RunClientMode(ULONGLONG ululRemoteBthAddr, int iMaxCxnCycles);
int IU_BTComm();

#endif