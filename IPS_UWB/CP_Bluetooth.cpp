#include "CP_Bluetooth.h"
#include "CP_Filter.h"
#include "CP_Calibration.h"




DWORD WINAPI BluetoothThread(LPVOID hWnd)
{
	int			retVal = 0;
	ULONGLONG  ululTagBlueToothAddr = 0;
	int BlueToothConnectionMode = CP_BLUETOOTH_USING_ADDRESS;
	char tagBlueToothName[BTH_MAX_NAME_SIZE + 1] = { '\0', };  // 1 extra for trailing NULL character
	char tagBlueToothAddr[CP_BT_ADDR_LEN] = { '\0', };
	WSADATA    WSAData = { 0 };

	//ready to bluetooth connection
	if ((retVal = WSAStartup(MAKEWORD(2, 2), &WSAData)) != 0) //if success to initial, WSAStartup() return 0
	{
		//goto WinsockCleanupAndExit;
	}
	if (BlueToothConnectionMode == CP_BLUETOOTH_USING_NAME)
	{
		char* BluetoothDeviceName = "UWB";
		strcpy(tagBlueToothName, BluetoothDeviceName);

		// Get address from name of the remote device and run the application in client mode
		if ((retVal = NameToBthAddr(tagBlueToothName, (BTH_ADDR *)&ululTagBlueToothAddr)) != 0)
		{
			//goto WinsockCleanupAndExit;
		}
	}
	else if (BlueToothConnectionMode == CP_BLUETOOTH_USING_ADDRESS)
	{
		char* BluetoothDeviceAddress = "00:19:01:37:BF:2E";
		strcpy(tagBlueToothAddr, BluetoothDeviceAddress);
		// Get address from formatted address-string of the remote device and run the application in client mode
		//  should be calling the WSAStringToAddress()
		if (0 != (retVal = AddrStringToBtAddr(tagBlueToothAddr, (BTH_ADDR *)&ululTagBlueToothAddr)))
		{
			//goto WinsockCleanupAndExit;
		}
	}

	////start bluetooth communication and recveive ToF data from Tag(UWB trx device)
	SOCKET             BT_Socket = INVALID_SOCKET;
	SOCKADDR_BTH       BT_SockAddr = { 0 };

	BT_SockAddr.addressFamily = AF_BTH;							// Setting address family to AF_BTH indicates winsock2 to use Bluetooth sockets
	BT_SockAddr.btAddr = (BTH_ADDR)ululTagBlueToothAddr;
	BT_SockAddr.serviceClassId = g_guidServiceClass;
	BT_SockAddr.port = 1;								// Valid ports are 1 - 31. if ServiceClassId is spesified, Port should be set to 0
	BT_Socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

	int recvLen = 0;

	if (BT_Socket == INVALID_SOCKET){
		goto CleanupAndExit;
	}
	if (connect(BT_Socket, (struct sockaddr *) &BT_SockAddr, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR){
		goto CleanupAndExit;
	}

	char  recvBuffer[CP_RECV_BUF_LENGTH] = { '\0' };		//receive data buffer
	char* pRecvDataBuf = recvBuffer;	//pointer to receive data buffer
	int   totalRecvLen = 0;

	CpToF RawToF;
	CpToF MovingAverageFilteredToF;
	CpToF KalmanFilteredToF;
	CpRealDistance RealDistance;
	CpQubeSize QubeSize;

	while (totalRecvLen < CP_RECV_BUF_LENGTH){
		recvLen = recv(BT_Socket, (char*)pRecvDataBuf, CP_RECV_BUF_LENGTH - totalRecvLen, 0); //pRecvDataBuf 에 형변환 해주는 이유는 현재 행 이후의 연산과정에서 int로 형변환이 되기때문
		if (recvLen > 0){

			pRecvDataBuf += recvLen;
			totalRecvLen += recvLen;

			if (totalRecvLen >= CP_RECV_BUF_LENGTH){

				//parse three ToF data from receive buffer
				RawToF = cpParsingAndGetToF(recvBuffer);

				//Moving Average Filtering
				MovingAverageFilteredToF = cpMovingAverageFilter(RawToF);

				//Kalman Filtering
				KalmanFilteredToF = cpKalmanFilter(MovingAverageFilteredToF);

				//transform ToF value into RealDistance(centi meter, cm)
				RealDistance = cpToFtoRealdistance(KalmanFilteredToF);

				if (RealDistance.Anchor[0] == CP_MODE_CALIBRATION){
					QubeSize = cpCalibration(RealDistance);
					if (QubeSize.height == CP_CALIBRATION_COMPLETE){
						SendMessage((HWND)hWnd, WM_CP_DRAW, (WPARAM)CP_MODE_CALIBRATION, (LPARAM)&QubeSize);
						MessageBoxA((HWND)hWnd, "Calibration Complete", "Calibration Complete", MB_OK);
					}
				}
				else{
					SendMessage((HWND)hWnd, WM_CP_DRAW, (WPARAM)CP_MODE_TRILATERATION, (LPARAM)&RealDistance);
				}

				//SendMessage(ywStruct->hWndMain, WM_USER + 2, (WPARAM)(totalRecvLen), (LPARAM)(recvBuffer));
				//if (ywStruct->draw_flag == 1) SendMessage(ywStruct->hWndMain, WM_USER + 1, (WPARAM)ywStruct->width, (LPARAM)ywStruct->height);
				//SendMessage(ywStruct->hWndMain, WM_USER + 3, (WPARAM)(totalRecvLen), (LPARAM)(recvBuffer));
				totalRecvLen = 0;
				pRecvDataBuf = recvBuffer;
			}
		}
		else if (recvLen == 0){
			// socket connection has been closed gracefully
			/*TO DO : 연결종료됬을 때 이후의 프로세스 위한 부분 구현*/
			return 0;
			goto CleanupAndExit;
		}
		else{
			//SOCKET_ERROR!
			//어떠한 이유에서 data를 recv하지 못했기 때문에 recv()함수가 SOCKET_ERROR를 리턴했다.
			goto CleanupAndExit;
		}
	}

CleanupAndExit:
	if (recvLen == SOCKET_ERROR){
		//ywStruct->WSA_ErrorCode = WSAGetLastError();
		//MessageBox(ywStruct->hWndMain, L"Error", ywStruct->error_msg, MB_OK);
	}
	if (closesocket(BT_Socket) == SOCKET_ERROR) {
		//ywStruct->WSA_ErrorCode = WSAGetLastError();
		//wsprintf(ywStruct->error_msg, L"WSA Error (%d)", ywStruct->WSA_ErrorCode);
		//MessageBox(ywStruct->hWndMain, L"Error", ywStruct->error_msg, MB_OK);
	}
	else {
		// Make the connection invalid regardless
		BT_Socket = INVALID_SOCKET;
		closesocket(BT_Socket);
	}


	return 0; //error free terminate thread

WinsockCleanupAndExit:
	WSACleanup();
	/* retVal has winsock error code */
	/* https://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx */
	return retVal;
}


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


// It opens a socket, connects it to a remote socket, recveive some data and closes the connection.

