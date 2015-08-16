#include "IU_BTComm.h"
#include "Trilateration_2D.h"
#include "Trilateration_3D.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInstance;
HDC hDC;
HGLRC hRC;
HWND hWnd;

YWstruct ywStruct;

bool keys[256];

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
GLvoid KillGLWindow(GLvoid);
BOOL CreateGLWindow(wchar_t* title, int width, int height, int bits, bool fullscreenflag);
int DrawGLScene(GLvoid);
int InitGL(GLvoid);
GLvoid ReSizeGLScene(GLsizei width, GLsizei height);

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif
GLfloat xAngle; // Key Point
GLfloat yAngle; // Key Point
GLfloat zDelta;
GLfloat rot = 0.0f;
float radius = 15.0f;
POINT mouseDownPt; // Key Point
GLUquadricObj *anchor_1;
GLUquadricObj *anchor_2;
GLUquadricObj *anchor_3;
GLUquadricObj *tag;
float radius_1, radius_2, radius_3;
bool isActive = true; //when window is minimization state, has false value
bool fullscreen = false;
//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV


LPCTSTR lpszClass = TEXT("IPS_UWB");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevIn
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInstance = hInstance;
	bool flag = true;

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

	//���������
	ywStruct.hFile = CreateFile(L"ToF_Data.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  //���� ������ϱ����ؼ� ���� �����س�


	if (!CreateGLWindow(L"yongwoo creative project", 640, 480, 16, fullscreen))
	{
		return 0; // Quit If Window Was Not Created
	}

	while (flag)
	{
		//if (GetMessage(&Message, NULL, 0, 0)) //�ؿ� PeekMessage�� ���� �ٸ��� ���� ���̴µ�??
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)){
			if (Message.message == WM_QUIT)	{ flag = false; }
			else {
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
		else {
			if (keys[VK_ESCAPE] || !DrawGLScene()) break; // ESC Ű �Է� �޾Ұų� �׸��µ� �����ߴٸ� ���α׷� ����
			else SwapBuffers(hDC);  // Swap Buffers (Double Buffering)

			if (keys[VK_F1]){
				keys[VK_F1] = FALSE;
				KillGLWindow();
				fullscreen = !fullscreen;
				// Recreate Our OpenGL Window
				if (!CreateGLWindow(L"yongwoo creative project", 640, 480, 16, &fullscreen)) break; // Quit If Window Was Not Created
			}
		}
	}

	gluDeleteQuadric(anchor_1); // �߰� �ڵ�

	// Shutdown
	KillGLWindow();									// Kill The Window

	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	DWORD ThreadID;
	HWND button;
	char Mes[100] = { 0 };

	switch (iMessage)
	{
	case WM_CREATE:
		ywStruct.hWndMain = hWnd;

		button = CreateWindow(L"BUTTON", L"OK",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, 0, 10, 10,
			hWnd, (HMENU)100, g_hInstance, NULL);
		//ShowWindow(button, SW_SHOW);
		return 0;
	case WM_COMMAND:
		CloseHandle(CreateThread(NULL, 0, GetToF, &ywStruct, 0, &ThreadID)); //ToF�� �޴� Thread ���
		return 0;

	case WM_ACTIVATE:
		if (!HIWORD(wParam)) { isActive = true; }   // Check Minimization State
		else				 { isActive = false; }
		return 0;

		//��ũ�����̹�..... �� ���ñ�� ����� ���� �Ʋ��ְ� �� �׷��� ������ �����ϰ� �ּ� ��������
		//case WM_SYSCOMMAND:			// Intercept System Commands
		//	switch (wParam)			// Check System Calls
		//	{
		//	case SC_SCREENSAVE:		// Screensaver Trying To Start?
		//	case SC_MONITORPOWER:	// Monitor Trying To Enter Powersave?
		//		return 0;			// Prevent From Happening
		//	}
		//	break;					// Exit
	case WM_KEYDOWN:
		keys[wParam] = true;
		ywStruct.keys[wParam] = TRUE;
		ywStruct.flag = 1;
		MessageBox(hWnd, L"keyDown", L"keyDown", MB_OK);
		return 0;
	case WM_KEYUP:
		keys[wParam] = true;
		ywStruct.keys[wParam] = FALSE;
		return 0;

	case WM_LBUTTONDOWN:
	{
		int X = GET_X_LPARAM(lParam);
		int Y = GET_Y_LPARAM(lParam);

		mouseDownPt.x = X;
		mouseDownPt.y = Y;

		SetCapture(hWnd);

		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (GetCapture() == hWnd) {
			int X = GET_X_LPARAM(lParam);
			int Y = GET_Y_LPARAM(lParam);

			xAngle += (Y - mouseDownPt.y) / 3.6;
			yAngle += (X - mouseDownPt.x) / 3.6;

			InvalidateRect(hWnd, NULL, FALSE);

			mouseDownPt.x = X;
			mouseDownPt.y = Y;
		}

		return 0;
	}
	case WM_LBUTTONUP:
	{
		ReleaseCapture();

		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		int Z = GET_WHEEL_DELTA_WPARAM(wParam);
		zDelta += (GLfloat)Z / 100.0f;
		return 0;
	}

	case WM_USER + 1:
		TriThread(&ywStruct);
		return 0;
	case WM_USER + 2:
		hDC = GetDC(hWnd);
		sprintf(Mes, "data : %s", (char*)wParam);
		TextOutA(hDC, 10, 30, Mes, strlen(Mes));
		//TextOutA(hDC, 10, 10, (char*)lParam, wParam);
		ReleaseDC(hWnd, hDC);
		return 0;
	case WM_USER + 3:
		if (ywStruct.distance_1 > 0 && ywStruct.distance_2 > 0 && ywStruct.distance_3 > 0){
			radius_1 = ywStruct.distance_1 / 10.0;
			radius_2 = ywStruct.distance_2 / 10.0;
			radius_3 = ywStruct.distance_3 / 10.0;
		}
		else{
			radius_1 = 10.0;
			radius_2 = 10.0;
			radius_3 = 10.0;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

//���� ������ ���� �ʿ� ���� ToF�޾Ƽ� �� Ȯ�� �غ��� ����ε� �� �޾����� �׶� ���� �޼��� ������ �׸��� �ص��ȴ�.
DWORD WINAPI DrawTrilateration(LPVOID lpParam)
{
	TriThread(&ywStruct);
	return 0;
}

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
		strcpy(g_szRemoteAddr, BTD_Addr);
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

BOOL CreateGLWindow(wchar_t* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	WndClass;				// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	g_hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	WndClass.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	WndClass.cbClsExtra = 0;									// No Extra Window Data
	WndClass.cbWndExtra = 0;									// No Extra Window Data
	WndClass.hInstance = g_hInstance;							// Set The Instance
	WndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	WndClass.hbrBackground = NULL;									// No Background Required For GL
	WndClass.lpszMenuName = NULL;									// We Don't Want A Menu
	WndClass.lpszClassName = L"OpenGL";								// Set The Class Name

	if (!RegisterClass(&WndClass))									// Attempt To Register The Window Class
	{
		MessageBoxA(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBoxA(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBoxA(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		L"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		g_hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBoxA(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);					// If full screen mode, Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBoxA(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBoxA(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBoxA(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBoxA(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass(L"OpenGL", g_hInstance))			// Are We Able To Unregister Class
	{
		MessageBoxA(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		g_hInstance = NULL;									// Set hInstance To NULL
	}
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(255.0f, 255.0f, 255.0f, 0.5f);			// Init Background 0.0f ~ 255.0f (black to white)
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


	anchor_1 = gluNewQuadric(); // <1> ���������� Quadric ��ü�� ������Ű�� �ڵ��̴�.
	gluQuadricDrawStyle(anchor_1, GLU_LINE); // <2>  GLU_FILL, GLU_LINE, GLU_SILHOUETTE, GLU_POINT ���·� ��ü�� �׸��ٴ� ��
	gluQuadricNormals(anchor_1, GLU_SMOOTH); // <3> �鿡 ���ؼ� �ε巴�� �׸��ٴ� ��
	gluQuadricOrientation(anchor_1, GLU_OUTSIDE); // <4> ���������� ������ ��ü�� ���ؼ� �ك������� ���ϵ��� �������͸� �����϶�� �ǹ�
	gluQuadricTexture(anchor_1, GL_FALSE); // <5> �ؽ��� ���� �������� ���� �����Ƿ� �ؽ��� ���� ��ǥ�� �������� ����� �ǹ�

	anchor_2 = gluNewQuadric();
	gluQuadricDrawStyle(anchor_2, GLU_LINE);
	gluQuadricNormals(anchor_2, GLU_SMOOTH);
	gluQuadricOrientation(anchor_2, GLU_OUTSIDE);
	gluQuadricTexture(anchor_2, GL_FALSE);


	anchor_3 = gluNewQuadric();
	gluQuadricDrawStyle(anchor_3, GLU_LINE);
	gluQuadricNormals(anchor_3, GLU_SMOOTH);
	gluQuadricOrientation(anchor_3, GLU_OUTSIDE);
	gluQuadricTexture(anchor_3, GL_FALSE);

	tag = gluNewQuadric();
	gluQuadricDrawStyle(anchor_3, GLU_LINE);
	gluQuadricNormals(anchor_3, GLU_SMOOTH);
	gluQuadricOrientation(anchor_3, GLU_OUTSIDE);
	gluQuadricTexture(anchor_3, GL_FALSE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	zDelta = -100.0f;

	return TRUE;										// Initialization Went OK
}
int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	//Anchor* anchor_2 = new Anchor(); /*TODO �̰� DrawGLScene()�Լ��� ȣ��ɶ����� ��� ��ü�����Ǵϰ� �ѹ������ؼ� �����ְ� ��������*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix


	if (keys[VK_UP]){
		radius += 1.0f;
	}
	else if (keys[VK_DOWN]){
		radius -= 1.0f;
	}
	if (keys[VK_LEFT]){
		yAngle -= 0.5f;
	}
	else if (keys[VK_RIGHT]){
		yAngle += 0.5f;
	}



	glTranslatef(0.0f, 0.0f, zDelta);
	glColor3f(1.0f, 1.0f, 1.0f);

	glRotatef(xAngle, 10.0f, 0.0f, 0.0f);
	glRotatef(yAngle, 0.0f, 10.0f, 0.0f);



	glBegin(GL_LINE_LOOP);
	glVertex3f(-20.0f, 20.0f, 20.0f);
	glVertex3f(20.0f, 20.0f, 20.0f);
	glVertex3f(20.0f, -20.0f, 20.0f);
	glVertex3f(-20.0f, -20.0f, 20.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-20.0f, 20.0f, -20.0f);
	glVertex3f(20.0f, 20.0f, -20.0f);
	glVertex3f(20.0f, -20.0f, -20.0f);
	glVertex3f(-20.0f, -20.0f, -20.0f);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(-20.0f, 20.0f, 20.0f);
	glVertex3f(-20.0f, 20.0f, -20.0f);
	glVertex3f(20.0f, 20.0f, 20.0f);
	glVertex3f(20.0f, 20.0f, -20.0f);
	glVertex3f(20.0f, -20.0f, 20.0f);
	glVertex3f(20.0f, -20.0f, -20.0f);
	glVertex3f(-20.0f, -20.0f, 20.0f);
	glVertex3f(-20.0f, -20.0f, -20.0f);
	glEnd();

	glPushMatrix();
	glTranslatef(20.0f, 20.0f, 20.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(anchor_1, radius_1, 24, 24);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-20.0f, 20.0f, -20.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(anchor_2, radius_2, 24, 24);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(20.0f, 20.0f, -20.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(anchor_3, radius_3, 24, 24);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(10.0f, 10.0f, -10.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(tag, 2, 24, 24);
	glPopMatrix();

	return TRUE;										// Everything Went OK
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 200.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}