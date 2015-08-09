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

	ywStruct->hDCMain = GetDC(ywStruct->hWndMain);
	wchar_t text[IU_READ_DATA_LENGTH + 2];

	int recvLen = 0;


	//싱크 맞추는 첫번째 방법 
	//int len = 0;
	//int bufferLen = IU_READ_DATA_LENGTH;
	//while (1) {
	//	recvLen = recv(BT_Socket, midBuffer, bufferLen, 0);
	//	switch (recvLen)
	//	{
	//	case 0:
	//		//to make 'Socket connection' closed gracefully!
	//		break;
	//	case SOCKET_ERROR:
	//		ywStruct->WSA_ErrorCode = WSAGetLastError();
	//		break;
	//	default:
	//		if (midBuffer[0] == '*' && midBuffer[24] == '=') //싱크도 맞고 보낸 데이터가 정확히 다 온경우
	//		{
	//			bufferLen = IU_READ_DATA_LENGTH;
	//			midBuffer[IU_READ_DATA_LENGTH] = '\0';
	//			len = strlen(midBuffer);
	//			mbstowcs(text, midBuffer, IU_READ_DATA_LENGTH);
	//			TextOut(ywStruct->hDCMain, 100, 100, text, IU_READ_DATA_LENGTH);
	//			strcpy(ywStruct->str, midBuffer);
	//			parsing(midBuffer, ywStruct);
	//			::SendMessage(ywStruct->hWndMain, WM_USER + 1, WPARAM(midBuffer), 0);
	//		}
	//		else // 싱크가 안 맞거나 중간에 데이터가 손실된 경우
	//		{
	//			if (midBuffer[0] == '=') //만약 지금 읽어들인 Data가 엔드...뭐시기면 다음 수신될 데이터가 '*' 일 것이라고 예상 하기 때문에 버퍼의 크기를 늘려줌
	//			{
	//				bufferLen = IU_READ_DATA_LENGTH;
	//			}
	//			else 
	//			{
	//				bufferLen = 1;
	//				flushBuffer(midBuffer, recvLen + 1);
	//			}
	//		}
	//		break;
	//	}
	//}


	//싱크 맞추는 두번째 방법. 첫번째랑 두번째랑 어떤게 더 효율적인지 모르겠다.
	char midBuffer[IU_MIDDLE_BUFFER_LENGTH];
	char* pMidBuffIdx = midBuffer; // pointer to Middle Buffer
	int totalRecvLen = 0;

	while (1) {

		while (totalRecvLen < IU_MIDDLE_BUFFER_LENGTH){
			recvLen = recv(BT_Socket, (char*)pMidBuffIdx, IU_MIDDLE_BUFFER_LENGTH - totalRecvLen, 0); //pMidBuffIdx 에 형변환 해주는 이유는 현재 행 이후의 연산과정에서 int로 형변환이 되기때문
			if (recvLen > 0){
				//if (recvLen > IU_MIDDLE_BUFFER_LENGTH - totalRecvLen){
				//}
				pMidBuffIdx += recvLen;
				totalRecvLen += recvLen;
				if (totalRecvLen >= IU_MIDDLE_BUFFER_LENGTH){
					totalRecvLen = 0;

					break;
				}
			}
			else if (recvLen == 0){
				// socket connection has been closed gracefully
				/*TO DO : 연결종료됬을 때 이후의 프로세스 위한 부분 구현*/
				break;
			}
			else{
				//SOCKET_ERROR!
				//어떠한 이유에서 data를 recv하지 못했기 때문에 recv()함수가 SOCKET_ERROR를 리턴했다.
				if (recvLen == SOCKET_ERROR){
					ywStruct->WSA_ErrorCode = WSAGetLastError();
					MessageBox(ywStruct->hWndMain, L"Error", ywStruct->error_msg, MB_OK);
					break;
				}
			}
		}
		totalRecvLen = 0;


		if (recvLen > 0){
			char *temp = new char[50];
			strncpy(temp, midBuffer, recvLen);
			SendMessage(ywStruct->hWndMain, WM_USER + 2, (WPARAM)(recvLen), (LPARAM)(temp));

			//for (int i = 0; i < 26; i++){
			//	if (*(pMidBuffIdx + i) == '*' && *(pMidBuffIdx + i + 24) == '='){
			//		//MessageBoxA(NULL, midBuffer, NULL, MB_OK);
			//		//MessageBox(ywStruct->hWndMain, L"sync!", L"sync!", MB_OK);
			//		strncpy(ywStruct->str, pMidBuffIdx + i, sizeof(wchar_t) * 25);

			//		//strncpy(tempData, pMidBuffIdx + i, 50);
			//		

			//		//parsing(ywStruct->str, ywStruct);
			//		//SendMessage(ywStruct->hWndMain, WM_USER + 1, WPARAM(midBuffer), 0);
			//		//if (ywStruct->flag == 1){
			//		//	if (cnt == 10) {
			//		//		ywStruct->flag = 0;
			//		//		cnt = 0;
			//		//		MessageBox(ywStruct->hWndMain, L"Write 1000 data", L"Write 1000 data", MB_OK);
			//		//	}
			//		//	strncpy(ywStruct->str, pMidBuffIdx + i + 1, sizeof(wchar_t) * 7);
			//		//	mbstowcs(text, ywStruct->str, 7);
			//		//	text[7] = 0x0d;
			//		//	text[8] = 0x0a;
			//		//	DWORD dwWritten;
			//		//	SetFilePointer(ywStruct->hFile, 0, NULL, FILE_END);
			//		//	WriteFile(ywStruct->hFile, text, sizeof(wchar_t) * 9, &dwWritten, NULL);//거리데이터 기록으로 남기기 위해 파일출력
			//		//	cnt++;
			//		//}
			//		//flushBuffer(midBuffer, IU_READ_DATA_LENGTH * 2 + 1);
			//		recvLen = recv(BT_Socket, midBuffer, i, 0); //싱크 맞추기 위해서 뒤에 남은 i만큼만 버퍼에서 읽어들인다. 그럼 다음부턴 딱 *부분부터 버퍼에서 읽어올 수 있다.
			//		break;
			//	}
			//}
			//flushBuffer(midBuffer, IU_READ_DATA_LENGTH * 2 + 1);
		}
		else if (recvLen == 0){
			//to make 'Socket connection' closed gracefully!
			MessageBox(ywStruct->hWndMain, L"connection closed", L"connection closed", MB_OK);
			break;
		}
		else if (recvLen < 0){
			if (recvLen == SOCKET_ERROR){
				ywStruct->WSA_ErrorCode = WSAGetLastError();
				MessageBox(ywStruct->hWndMain, L"Error", ywStruct->error_msg, MB_OK);
				break;
			}
			else{
				MessageBox(ywStruct->hWndMain, L"connection error", L"connection error", MB_OK);
				break;
			}
		}
	}



	ReleaseDC(ywStruct->hWndMain, ywStruct->hDCMain);

CleanupAndExit:
	if (closesocket(BT_Socket) == SOCKET_ERROR) {
		ywStruct->WSA_ErrorCode = WSAGetLastError();
		wsprintf(ywStruct->error_msg, L"WSA Error (%d)", ywStruct->WSA_ErrorCode);
		MessageBox(ywStruct->hWndMain, L"Error", ywStruct->error_msg, MB_OK);
	}
	else {
		// Make the connection invalid regardless
		BT_Socket = INVALID_SOCKET;
		closesocket(BT_Socket);
	}
	return 0;
}

void parsing(char* midData, YWstruct* ywStruct){

	int i = 0;
	int anchorID = 0;
	char* pMidData = midData;
	int flag = 0;
	char distFromAnchor[4][10];
	int index = 0;
	int len = 0;

	for (i = 0; i < strlen(midData); i++){
		if (*pMidData == '*'){
			flag = 1;
			anchorID = 1;
			index = 0;
			len = strlen(midData) - i;
			break;
		}
		pMidData++;
	}
	if (flag == 0) return; //받은 데이터에 '*'가 없으므로 parsing하지 않고 나간다.

	for (i = 0; i < len && flag == 1; i++){
		switch (*pMidData)
		{
		case ',': //tokenizer
			anchorID += 1;
			index = 0;
			break;
		case '=': //end

			//Transform ToF into real distance(cm)
			ywStruct->distance_1 = atof(distFromAnchor[1])*84.896 - 35.1868;
			ywStruct->distance_2 = atof(distFromAnchor[2])*84.896 - 35.1868;
			ywStruct->distance_3 = atof(distFromAnchor[3])*84.896 - 35.1868;
			flag = 0;
			return; //사실 for문 조건에 flag == 1 도 없에고 이 바로윗줄에서 flag = 0;도 없에도 return; 때문에 나가진다.
			break;
		default:
			if ('0' <= *pMidData && *pMidData <= '9'){ //when receive error free data
				distFromAnchor[anchorID - 1][index] = *pMidData;
				index += 1;
			}
			else{
				//TO DO
				//통신과정에서 Data에 노이즈가 발생해서 (error free 하지 못하게) 
				//숫자 이외의 값이 수신되면 이 데이터는 쓰면 안된다.
				//그런데.. 그럴 일이 없을 거같긴해서 실제로 구현하지는 않는다.
				break;
			}
		}
		pMidData++;
	}

}

void flushBuffer(char* buffer, int bufferSize)
{
	for (int i = 0; i < bufferSize; i++){
		buffer[i] = '\0';
	}
}