#include "IU_BTComm.h"
#include "Trilateration_2D.h"
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hIns;
//HWND hWndMain;

YWstruct ywStruct;

bool keys[256];

LPCTSTR lpszClass = TEXT("IPS_UWB");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevIn
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hIns = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT
		, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)){
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	DWORD ThreadID;
	ywStruct.hWndMain = hWnd;
	switch (iMessage)
	{
	case WM_CREATE:
		CloseHandle(CreateThread(NULL, 0, GetToF, &ywStruct, 0, &ThreadID)); //ToF값 받는 Thread 등록
	case WM_KEYDOWN:
		keys[wParam] = TRUE;
		return 0;
		break;
	case WM_KEYUP:
		keys[wParam] = FALSE;
		return 0;
	case WM_USER + 1:
		//TriThread(&ywStruct);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

//굳이 쓰레드 돌릴 필요 없고 ToF받아서 값 확인 해보고 제대로된 값 받았으면 그때 유저 메세지 날려서 그리게 해도된다.
//DWORD WINAPI DrawTrilateration(LPVOID lpParam)
//{
//	TriThread(&ywStruct);
//	return 0;
//}

DWORD WINAPI GetToF(LPVOID ywStruct)
{
	ULONG      ulRetCode = 0;
	
	ULONGLONG  ululRemoteBthAddr = 0;
	int mode = 2;
	YWstruct* pYWstruct;
	char g_szRemoteName[BTH_MAX_NAME_SIZE + 1] = { 0 };  // 1 extra for trailing NULL character
	char g_szRemoteAddr[IU_BT_ADDR_LEN + 1] = { 0 }; // 1 extra for trailing NULL character

	pYWstruct = (YWstruct*)ywStruct;

	// Ask for Winsock version 2.2.
	WSADATA    WSAData = { 0 };
	if ((ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData)) != 0)
	{
		goto CleanupAndExit;
	}
	else
		scanf("%d\n", &mode);


	if (mode == 1)
	{
		char* BTD_Name = "UWB";
		strcpy(g_szRemoteName, BTD_Name);

		// Get address from name of the remote device and run the application in client mode
		if ((ulRetCode = NameToBthAddr(g_szRemoteName, (BTH_ADDR *)&ululRemoteBthAddr)) != 0)
		{
			goto CleanupAndExit;
		}
		ulRetCode = RunClientMode(ululRemoteBthAddr, pYWstruct);
	}
	else if (mode == 2)
	{
		char* BTD_Addr = "00:19:01:37:BF:2E";
		strcpy(g_szRemoteAddr,BTD_Addr);
		// Get address from formatted address-string of the remote device and run the application in client mode
		//  should be calling the WSAStringToAddress()
		if (0 != (ulRetCode = AddrStringToBtAddr(g_szRemoteAddr, (BTH_ADDR *)&ululRemoteBthAddr)))
		{
			goto CleanupAndExit;
		}

		ulRetCode = RunClientMode(ululRemoteBthAddr, pYWstruct);
	}


CleanupAndExit:
	return (int)(ulRetCode);
}