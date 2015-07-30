#include "IU_BTComm.h"


// This(NameToBtgAddr()) just redundant!!!!
// TODO: use inquiry timeout SDP_DEFAULT_INQUIRY_SECONDS
// NameToBthAddr converts a bluetooth device name to a bluetooth address,
// if required by performing inquiry with remote name requests.
// This function demonstrates device inquiry, with optional LUP flags.
ULONG NameToBthAddr(const char * pszRemoteName, BTH_ADDR * pRemoteBtAddr)
{
	INT          iResult = 0, iRetryCount = 0;
	BOOL         bContinueLookup = FALSE;
	BOOL		 bRemoteDeviceFound = FALSE;
	ULONG        ulFlags = 0, ulPQSSize = sizeof(WSAQUERYSET);
	HANDLE       hLookup = 0;
	PWSAQUERYSET pWSAQuerySet = NULL;

	if ((pszRemoteName == NULL) || (pRemoteBtAddr == NULL))
	{
		goto CleanupAndExit;
	}

	if ((pWSAQuerySet = (PWSAQUERYSET)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulPQSSize)) == NULL)
	{
		goto CleanupAndExit;
	}

	// Search for the device with the correct name
	for (iRetryCount = 0; !bRemoteDeviceFound && (iRetryCount < CXN_MAX_INQUIRY_RETRY); iRetryCount++)
	{
		// WSALookupServiceXXX() is used for both service search and device inquiry
		// LUP_CONTAINERS is the flag which signals that we're doing a device inquiry. 
		ulFlags = LUP_CONTAINERS;
		// Friendly device name (if available) will be returned in lpszServiceInstanceName
		ulFlags |= LUP_RETURN_NAME;
		// BTH_ADDR will be returned in lpcsaBuffer member of WSAQUERYSET
		ulFlags |= LUP_RETURN_ADDR;
		// Similar to:
		// ulFlags = LUP_CONTAINERS | LUP_RETURN_NAME | LUP_RETURN_ADDR;

		if (iRetryCount == 0)
		{
			//printf("*INFO* | Inquiring device from cache...\n");
		}
		else
		{
			// Flush the device cache for all inquiries, except for the first inquiry
			//
			// By setting LUP_FLUSHCACHE flag, we're asking the lookup service to do
			// a fresh lookup instead of pulling the information from device cache.
			ulFlags |= LUP_FLUSHCACHE;

			// Pause for some time before all the inquiries after the first inquiry
			//
			// Remote Name requests will arrive after device inquiry has
			// completed.  Without a window to receive IN_RANGE notifications,
			// we don't have a direct mechanism to determine when remote
			// name requests have completed.
			//printf("*INFO* | Unable to find device.  Waiting for %d seconds before re-inquiry...\n", CXN_DELAY_NEXT_INQUIRY);
			//printf("I am sleeping for a while...\n");
			Sleep(CXN_DELAY_NEXT_INQUIRY * 1000);

			//printf("*INFO* | Inquiring device ...\n");
		}

		// Start the lookup service
		iResult = 0;
		hLookup = 0;
		bContinueLookup = FALSE;
		ZeroMemory(pWSAQuerySet, ulPQSSize);
		pWSAQuerySet->dwNameSpace = NS_BTH;
		pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);

		iResult = WSALookupServiceBegin(pWSAQuerySet, ulFlags, &hLookup);

		if ((iResult == NO_ERROR) && (hLookup != NULL))
		{
			//printf("WSALookupServiceBegin() is fine!\n");
			bContinueLookup = TRUE;
		}
		else if (0 < iRetryCount)
		{
			//printf("=CRITICAL= | WSALookupServiceBegin() failed with error code %d, Error = %d\n", iResult, WSAGetLastError());
			goto CleanupAndExit;
		}

		while (bContinueLookup)
		{
			// Get information about next bluetooth device
			//
			// Note you may pass the same WSAQUERYSET from LookupBegin
			// as long as you don't need to modify any of the pointer
			// members of the structure, etc.
			//
			// ZeroMemory(pWSAQuerySet, ulPQSSize);
			// pWSAQuerySet->dwNameSpace = NS_BTH;
			// pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
			if (WSALookupServiceNext(hLookup, ulFlags, &ulPQSSize, pWSAQuerySet) == NO_ERROR)
			{
				//printf("WSALookupServiceNext() is OK lol!\n");

				if ((pWSAQuerySet->lpszServiceInstanceName != NULL))
				{
					// Found a remote bluetooth device with matching name.
					// Get the address of the device and exit the lookup.
					//printf("Again, remote name: %S\n", pWSAQuerySet->lpszServiceInstanceName);
					// Need to convert to the 'standard' address format
					//printf("Local address: %012X\n", pWSAQuerySet->lpcsaBuffer->LocalAddr);
					//printf("Remote address: %012X\n", pWSAQuerySet->lpcsaBuffer->RemoteAddr);
					CopyMemory(pRemoteBtAddr, &((PSOCKADDR_BTH)pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr,
						sizeof(*pRemoteBtAddr));
					bRemoteDeviceFound = TRUE;
					bContinueLookup = FALSE;
				}
			}
			else
			{
				if ((iResult = WSAGetLastError()) == WSA_E_NO_MORE) //No more data
				{
					// No more devices found.  Exit the lookup.
					//printf("No more device found...\n");
					bContinueLookup = FALSE;
				}
				else if (iResult == WSAEFAULT)
				{
					// The buffer for QUERYSET was insufficient.
					// In such case 3rd parameter "ulPQSSize" of function "WSALookupServiceNext()" receives
					// the required size.  So we can use this parameter to reallocate memory for QUERYSET.
					HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
					pWSAQuerySet = NULL;
					if (NULL == (pWSAQuerySet = (PWSAQUERYSET)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulPQSSize)))
					{
						//printf("!ERROR! | Unable to allocate memory for WSAQERYSET...\n");
						bContinueLookup = FALSE;
					}
					else{
						//printf("HeapAlloc() for WSAQERYSET is OK!\n");
					}
				}
				else
				{
					//printf("=CRITICAL= | WSALookupServiceNext() failed with error code %d\n", iResult);
					bContinueLookup = FALSE;
				}
			}
		}
		// End the lookup service
		WSALookupServiceEnd(hLookup);
	}

CleanupAndExit:
	if (NULL != pWSAQuerySet)
	{
		HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
		pWSAQuerySet = NULL;
	}

	if (bRemoteDeviceFound)
	{
		return(0);
	}
	else
	{
		return(1);
	}
}

// This just redundant!!!!
//
// Convert a formatted BTH address string to populate a BTH_ADDR (actually a ULONGLONG)
ULONG AddrStringToBtAddr(const char * pszRemoteAddr, BTH_ADDR * pRemoteBtAddr)
{
	ULONG ulAddrData[6], ulRetCode = 0;
	BTH_ADDR BtAddrTemp = 0;
	int i;

	if ((pszRemoteAddr == NULL) || (pRemoteBtAddr == NULL))
	{
		ulRetCode = 1;
		goto CleanupAndExit;
	}

	*pRemoteBtAddr = 0;

	// Populate a 6 membered array of unsigned long integers
	// by parsing the given address in string format
	sscanf_s(pszRemoteAddr, "%02x:%02x:%02x:%02x:%02x:%02x", &ulAddrData[0], &ulAddrData[1], &ulAddrData[2],
		&ulAddrData[3], &ulAddrData[4], &ulAddrData[5]);

	// Construct a BTH_ADDR from 6 integers stored in the array
	//printf("Constructing the BTH_ADDR...\n");
	for (i = 0; i < 6; i++)
	{
		// Extract data from the first 8 lower bits.
		BtAddrTemp = (BTH_ADDR)(ulAddrData[i] & 0xFF);
		// Push 8 bits to the left
		*pRemoteBtAddr = ((*pRemoteBtAddr) << 8) + BtAddrTemp;
	}
	//printf("Remote address: 0X%X\n", pRemoteBtAddr);

CleanupAndExit:
	return ulRetCode;
}

// It opens a socket, connects it to a remote socket, transfer some data over the connection and closes the connection.
ULONG RunClientMode(ULONGLONG ululRemoteAddr, YWstruct* ywStruct)
{
	//ULONG              ulRetCode = 0;
	SOCKET             BT_Socket = INVALID_SOCKET; //Socket도 핸들 처럼 음..ID 라고 생각하면된다? 값도 실제로 정수값을 가진다.
	SOCKADDR_BTH       BT_SockAddr = { 0 };
	char recvBuffer[IU_RECEIVE_DATA_LENGTH] = { 0 };
	int recvDataLength = 0;

	BT_SockAddr.addressFamily = AF_BTH; // Setting address family to AF_BTH indicates winsock2 to use Bluetooth sockets
	BT_SockAddr.btAddr = (BTH_ADDR)ululRemoteAddr;
	BT_SockAddr.serviceClassId = g_guidServiceClass;
	BT_SockAddr.port = 1;	// Valid ports are 1 - 31. if ServiceClassId is spesified, Port should be set to 0

	//// Create a static data-string, which will be transferred to the remote Bluetooth device
	////  may make this #define and do strlen() of the string
	//strncpy_s(szData, sizeof(szData), "~!@#$%^&*()-_=+?<>1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	//	CXN_TRANSFER_DATA_LENGTH - 1);

	//소켓 생성
	BT_Socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

	//error check
	if (BT_Socket == INVALID_SOCKET){
		ywStruct->WSA_ErrorCode = WSAGetLastError();
		ywStruct->IU_ErrorCode = IU_ERROR_SOCKET;
		goto CleanupAndExit;
		/*TO DO : 바로 위에 goto Clean~ 이거 대신 프로세스 멈추고 경고 날리는거 구현*/
	}


	if (connect(BT_Socket, (struct sockaddr *) &BT_SockAddr, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR)
	{
		ywStruct->WSA_ErrorCode = WSAGetLastError();
		ywStruct->IU_ErrorCode = IU_ERROR_CONNECTION;
		goto CleanupAndExit;
	}


	while (1) {
		recvDataLength = recv(BT_Socket, recvBuffer, IU_RECEIVE_DATA_LENGTH, 0);

		switch (recvDataLength)
		{
		case 0:
			//to make 'Socket connection' closed gracefully!
			break;
		case SOCKET_ERROR:
			ywStruct->WSA_ErrorCode = WSAGetLastError();
			break;
		default:
			//if (recvDataLength != 23) break;
			if (recvBuffer[0] == '*'){
				//printf("%s", szDataBuffer);
				strcpy(ywStruct->str, recvBuffer);
				parsing(recvBuffer, ywStruct);
				//MessageBox(ywStruct->hWndMain, L"recv complete", L"recv complete", MB_OK);
				::SendMessage(ywStruct->hWndMain, WM_USER + 1, WPARAM(recvBuffer), 0);
			}
			break;
		}
	}

	if (closesocket(BT_Socket) == SOCKET_ERROR) {
		ywStruct->WSA_ErrorCode = WSAGetLastError();
		goto CleanupAndExit;
	}
	else {
		// Make the connection invalid regardless
		BT_Socket = INVALID_SOCKET;
		closesocket(BT_Socket);
	}




CleanupAndExit:
	if (BT_Socket != INVALID_SOCKET)
	{
		closesocket(BT_Socket);
		BT_Socket = INVALID_SOCKET;
	}
	return 0;
	//return ulRetCode;
}

void parsing(char* string, YWstruct* ywStruct){
	int i = 0;
	distance data;
	int start_flag = 0;
	int index = 0;

	char anchor1[10];
	char anchor2[10];
	char anchor3[10];
	char anchor4[10];

	int index1 = 0;
	int index2 = 0;
	int index3 = 0;
	int index4 = 0;

	while (1){
		if (i > 30) i = 0;
		if (*(string + i) == '*'){		// 시작 문자
			start_flag = 1;
			index = 1;
			index1 = 0;
			index2 = 0;
			index3 = 0;
			index4 = 0;
		}
		else if (*(string + i) == '='){		// 끝 문자
			start_flag = 0;
			index = 0;

			//data.anchor1 = atof(anchor1);		// 받은 데이터 변환
			//data.anchor2 = atof(anchor2);
			//data.anchor3 = atof(anchor3);
			//data.anchor4 = atof(anchor4);
			ywStruct->distance_1 = atof(anchor1) * 20.0f;
			ywStruct->distance_2 = atof(anchor2) * 20.0f;
			ywStruct->distance_3 = atof(anchor3) * 20.0f;


			//for (i = 0; i < index1; i++)		// anchor 문자열 초기화
			//	anchor1[i] = 0;
			//for (i = 0; i < index2; i++)
			//	anchor2[i] = 0;
			//for (i = 0; i < index3; i++)
			//	anchor3[i] = 0;
			//for (i = 0; i < index4; i++)
			//	anchor4[i] = 0;

			break;
		}
		else if (*(string + i) == ','){		// Anchor 구분 문자
			index += 1;
		}
		else{
			if (index == 1){		// Anchor1
				anchor1[index1] = *(string + i);
				index1 += 1;
			}
			else if (index == 2){		// Anchor2
				anchor2[index2] = *(string + i);
				index2 += 1;
			}
			else if (index == 3){		// Anchor3
				anchor3[index3] = *(string + i);
				index3 += 1;
			}
			else if (index == 4){		// Anchor4
				anchor4[index4] = *(string + i);
				index4 += 1;
			}
		}

		i += 1;
	}

}