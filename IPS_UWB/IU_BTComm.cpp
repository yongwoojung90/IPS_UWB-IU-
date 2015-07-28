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
	SOCKET             LocalSocket = INVALID_SOCKET;
	SOCKADDR_BTH       SockAddrBthServer = { 0 };
	char* pszDataBufferIndex = NULL;
	char szDataBuffer[CXN_TRANSFER_DATA_LENGTH] = { 0 };
	pszDataBufferIndex = &szDataBuffer[0];
	int iTotalLengthReceived = 0, iLengthReceived = 0;
 
	SockAddrBthServer.addressFamily = AF_BTH; // Setting address family to AF_BTH indicates winsock2 to use Bluetooth sockets
	SockAddrBthServer.btAddr = (BTH_ADDR)ululRemoteAddr;
	SockAddrBthServer.serviceClassId = g_guidServiceClass;
	SockAddrBthServer.port = 1;	// Valid ports are 1 - 31. if ServiceClassId is spesified, Port should be set to 0

	//// Create a static data-string, which will be transferred to the remote Bluetooth device
	////  may make this #define and do strlen() of the string
	//strncpy_s(szData, sizeof(szData), "~!@#$%^&*()-_=+?<>1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	//	CXN_TRANSFER_DATA_LENGTH - 1);

	if ((LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM)) == INVALID_SOCKET)
	{
		//printf("=CRITICAL= | socket() call failed. Error = [%d]\n", WSAGetLastError());
		
		//ulRetCode = 1;
		goto CleanupAndExit;
	}

	if (LocalSocket != INVALID_SOCKET)
	{
		//printf("*INFO* | socket() call succeeded. Socket = [0x%X]\n", LocalSocket);
	}


	// Connect the socket (pSocket) to a given remote socket represented by address (pServerAddr)
	if (connect(LocalSocket, (struct sockaddr *) &SockAddrBthServer, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR)
	{
		wprintf(L"=CRITICAL= | connect() call failed. Error=[%d]\n", WSAGetLastError());
		//ulRetCode = 1;
		goto CleanupAndExit;
	}

	if (connect(LocalSocket, (struct sockaddr *) &SockAddrBthServer, sizeof(SOCKADDR_BTH)) != SOCKET_ERROR)
	{
		//printf("*INFO* | connect() call succeeded!\n");
	}

	while (1)
	{
		iLengthReceived = recv(LocalSocket, pszDataBufferIndex, (CXN_TRANSFER_DATA_LENGTH - iTotalLengthReceived), 0);

		switch (iLengthReceived)
		{
		case 0:
			//to make 'Socket connection' closed gracefully!
			break;
		case SOCKET_ERROR:
			//printf("=CRITICAL= | recv() call failed. Error=[%d]\n", WSAGetLastError());
			//ulRetCode = 1;
			break;
		default: // most cases when data is being read
			if (iTotalLengthReceived + iLengthReceived >= CXN_TRANSFER_DATA_LENGTH - 10){
				iTotalLengthReceived = 0;
				pszDataBufferIndex++;
				pszDataBufferIndex = '\0';
				pszDataBufferIndex = szDataBuffer;
				if (szDataBuffer[0] == 't'){
					//printf("%s", szDataBuffer);
					strcpy(ywStruct->str, szDataBuffer);
				}
				//else{
				//	memset(szDataBuffer, '\0', sizeof(szDataBuffer)*CXN_TRANSFER_DATA_LENGTH);
				//}
				break;
			}
			pszDataBufferIndex += iLengthReceived;
			iTotalLengthReceived += iLengthReceived;

			if (iLengthReceived != SOCKET_ERROR)
			{
				////printf("*INFO* | Receiving data of length = [%d]. Current Total = [%d]\n", iLengthReceived, iTotalLengthReceived);
			}
			break;
		}
	}

	//if (ulRetCode == 0)
	if (1)
	{
		if (CXN_TRANSFER_DATA_LENGTH != iTotalLengthReceived)
		{
			//printf("+WARNING+ | Data transfer aborted mid-stream. Expected Length = [%d], Actual Length = [%d]\n", CXN_TRANSFER_DATA_LENGTH, iTotalLengthReceived);
		}

		//printf("*INFO1* | Received following data string from remote device:\n%s\n", szDataBuffer);

		// Close the connection
		if (closesocket(LocalSocket) == SOCKET_ERROR)
		{
			//printf("=CRITICAL= | closesocket() call failed w/socket = [0x%X]. Error=[%d]\n", LocalSocket, WSAGetLastError());
			//ulRetCode = 1;
		}
		else
		{
			// Make the connection invalid regardless
			LocalSocket = INVALID_SOCKET;

			if (closesocket(LocalSocket) != SOCKET_ERROR)
			{
				//printf("*INFO2* | closesocket() call succeeded w/socket=[0x%X]\n", LocalSocket);
			}
		}
	}

	//일단 recv()잘됨 근데 스트림 버퍼에서 읽어오는게 문제인듯?
	////recv()넣어보자 2
	//for (int i = 0; i < CXN_TRANSFER_DATA_LENGTH; i++){
	//	szData[i] = '\0';
	//}
	//int iResult = 0;
	//do {
	//	////printf("recv() : ");
	//	iResult = recv(LocalSocket, szData, CXN_TRANSFER_DATA_LENGTH, 0);
	//	if (iResult > 0){
	//		////printf(" %d Bytes received from sender", iResult);
	//		//printf(" data = %s\n",szData);
	//		for (int i = 0; i < iResult; i++){
	//			szData[i] = '\0';
	//		}
	//	}
	//	else if (iResult == 0)
	//		//printf("Connection closed by peer!\n");
	//	else
	//		//printf("recv() failed with error code %d\n", WSAGetLastError());
	//} while (iResult > 0);


	// send() call indicates winsock2 to send the given data
	// of a specified length over a given connection.
	//printf("*INFO3* | Sending the following data string:\n\t%s\n", szData);
	//if (send(LocalSocket, szData, CXN_TRANSFER_DATA_LENGTH, 0) == SOCKET_ERROR)
	//{
	//	//printf("=CRITICAL= | send() call failed w/socket = [0x%X], szData = [%p], dataLen = [%d]. WSAGetLastError=[%d]\n", LocalSocket, szData, CXN_TRANSFER_DATA_LENGTH, WSAGetLastError());
	//	ulRetCode = 1;
	//	goto CleanupAndExit;
	//}
	//if (2 <= g_iOutputLevel)
	//{
	//	//printf("*INFO4* | send() call succeeded\n");
	//}

	// Close the socket
	if (SOCKET_ERROR == closesocket(LocalSocket))
	{
		//printf("=CRITICAL= | closesocket() call failed w/socket = [0x%X]. Error=[%d]\n", LocalSocket, WSAGetLastError());
		//ulRetCode = 1;
		goto CleanupAndExit;
	}

	LocalSocket = INVALID_SOCKET;



CleanupAndExit:
	if (LocalSocket != INVALID_SOCKET)
	{
		closesocket(LocalSocket);
		LocalSocket = INVALID_SOCKET;
	}
	return 0;
	//return ulRetCode;
}