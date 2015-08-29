#include "CP_Bluetooth.h"
#include "Trilateration_2D.h"
#include "Trilateration_3D.h"
#include "CP_DRAW.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInstance;
HDC hDC;
HGLRC hRC;
HWND hWnd;
CpRealDistance gRealDistance;
CpQubeSize gQubeSize;

bool keys[256];

//int writeFlag = 0;
//int writeCount = 0;
//float tempDist1;

bool isReadyToDraw = false;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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

static float width_3D = 0.0f;
static float height_3D = 0.0f;
static float length_3D = 0.0f;
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

	//파일입출력
	//ywStruct.hFile = CreateFile(L"ToF_Data.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  //파일 입출력하기위해서 파일 오픈해놈


	if (!CreateGLWindow(L"yongwoo creative project", 650, 480, 16, fullscreen))
	{
		return 0; // Quit If Window Was Not Created
	}

	while (flag)
	{
		//if (GetMessage(&Message, NULL, 0, 0)) //밑에 PeekMessage랑 별로 다른게 없어 보이는데??
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)){
			if (Message.message == WM_QUIT)	{ flag = false; }
			else {
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
		else {
			if (isReadyToDraw){
				if (keys[VK_ESCAPE] || !DrawGLScene()) break;
				else SwapBuffers(hDC);  // Swap Buffers (Double Buffering)

				if (keys[VK_F1]){
					keys[VK_F1] = FALSE;
					KillGLWindow();
					fullscreen = !fullscreen;
					// Recreate Our OpenGL Window
					if (!CreateGLWindow(L"yongwoo creative project", 650, 480, 16, &fullscreen)) break; // Quit If Window Was Not Created
				}
			}
		}
	}

	gluDeleteQuadric(anchor_1); // 추가 코드

	KillGLWindow();
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	DWORD ThreadID;
	HWND button;
	char Mes[100] = { 0 };
	int mode;
	switch (iMessage)
	{
	case WM_CREATE:
		button = CreateWindow(L"BUTTON", L"OK",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, 0, 10, 10,
			hWnd, (HMENU)100, g_hInstance, NULL);
		//ShowWindow(button, SW_SHOW);
		return 0;
	case WM_COMMAND:
		CloseHandle(CreateThread(NULL, 0, BluetoothThread, (LPVOID)hWnd, 0, &ThreadID)); //ToF값 받는 Thread 등록
		return 0;

	case WM_ACTIVATE:
		if (!HIWORD(wParam)) { isActive = true; }   // Check Minimization State
		else				 { isActive = false; }
		return 0;

		//스크린세이버..... 그 뭐시기냐 모니터 수명도 아껴주고 뭐 그런거 같은데 공부하고 주석 해제하자
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
		return 0;
	case WM_KEYUP:
		keys[wParam] = false;
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
	case WM_CP_DRAW:
		mode = (int)wParam;
		if (mode == CP_MODE_TRILATERATION) {
			gRealDistance = *((CpRealDistance*)lParam);
		}
		else if (mode == CP_MODE_CALIBRATION){
			gQubeSize = *((CpQubeSize*)lParam);
			width_3D = gQubeSize.width;
			length_3D = gQubeSize.length;
			isReadyToDraw = true;
		}
		TriThread(lParam, mode);
		return 0;

	case WM_USER + 1:

		return 0;
	case WM_USER + 2:
		hDC = GetDC(hWnd);
		sprintf(Mes, "data : %s", (char*)wParam); //wParam은 원래 실거리로 변환된 거리 3개 써있는 스트링이었음
		TextOutA(hDC, 10, 30, Mes, strlen(Mes));
		//TextOutA(hDC, 10, 10, (char*)lParam, wParam);
		ReleaseDC(hWnd, hDC);
		return 0;
	case WM_USER + 3:

		return 0;
	case WM_DESTROY:
		// Shutdown
		KillGLWindow();									// Kill The Window
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
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


	anchor_1 = gluNewQuadric(); // <1> 실제적으로 Quadric 객체를 생성시키는 코드이다.
	gluQuadricDrawStyle(anchor_1, GLU_LINE); // <2>  GLU_FILL, GLU_LINE, GLU_SILHOUETTE, GLU_POINT 형태로 물체를 그린다는 것
	gluQuadricNormals(anchor_1, GLU_SMOOTH); // <3> 면에 대해서 부드럽게 그린다는 것
	gluQuadricOrientation(anchor_1, GLU_OUTSIDE); // <4> 법선벡터의 방향을 물체에 대해서 바깓쪽으로 향하도록 법선벡터를 생성하라는 의미
	gluQuadricTexture(anchor_1, GL_FALSE); // <5> 텍스쳐 맵을 적용하지 않을 것으므로 텍스쳐 맵핑 좌표를 생성하지 말라는 의미

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
	//Anchor* anchor_2 = new Anchor(); /*TODO 이거 DrawGLScene()함수가 호출될때마다 계속 객체생성되니간 한번생성해서 쓸쑤있게 수정하자*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	glLoadIdentity(); // Reset The Current Modelview Matrix

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

	//if (keys[VK_SPACE]){
	//	writeFlag = 1;
	//}
	//char tempStrDrawGLScene[100] = { '\0', };
	//if (writeFlag == 1 && writeCount != 3000){
	//	//if (tempDist1 != ywStruct.distance_1){
	//	//	tempDist1 = ywStruct.distance_1;
	//	//	printf("%f\n", ywStruct.distance_1);
	//	//	writeCount++;
	//	//}
	//}
	//if (writeCount == 3000){
	//	MessageBoxA(hWnd, "기록끝! 다음 거리로!", "기록끝! 다음 거리로!", MB_OK);
	//	writeFlag = 0;
	//	writeCount = 0;
	//	printf("\n\n");
	//}
	//hDC = GetDC(hWnd);
	//sprintf(tempStrDrawGLScene, "count : %d // data: %f", writeCount, tempDist1);
	//TextOutA(hDC, 10, 100, tempStrDrawGLScene, strlen(tempStrDrawGLScene));

	glTranslatef(-20, 20, zDelta);
	glColor3f(1.0f, 1.0f, 1.0f);

	glRotatef(xAngle, 10.0f, 0.0f, 0.0f);
	glRotatef(yAngle, 0.0f, 20.0f, 0.0f);




	glBegin(GL_LINE_LOOP);
	glVertex3f(0.0f, 0.0f, length_3D);
	glVertex3f(width_3D, 0.0f, length_3D);
	glVertex3f(width_3D, -17.0f, length_3D);
	glVertex3f(0.0f, -17.0f, length_3D);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(width_3D, 0.0f, 0.0f);
	glVertex3f(width_3D, -17.0f, 0.0f);
	glVertex3f(0.0f, -17.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.0f, 00.0f, length_3D);
	glVertex3f(0.0f, 00.0f, 0.0f);

	glVertex3f(width_3D, 0.0f, length_3D);
	glVertex3f(width_3D, 0.0f, 0.0f);

	glVertex3f(width_3D, -17.0f, length_3D);
	glVertex3f(width_3D, -17.0f, 0.0f);

	glVertex3f(0.0f, -17.0f, length_3D);
	glVertex3f(0.0f, -17.0f, 0.0f);
	glEnd();


	//Trilateration_3D
	ywPos Tag = { 0, };


	//HWND debugButton;
	//debugButton = CreateWindow(L"BUTTON", L"OK",
	//	WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	//	0, 0, 10, 10,
	//	hWnd, (HMENU)100, g_hInstance, NULL);
	//char tagInfo[500] = { '\0', };
	Tag = calcTagPosition(gQubeSize, gRealDistance);
	//hDC = GetDC(hWnd);
	//TextOutA(hDC, 100, 30, tagInfo, strlen(tagInfo));
	//ReleaseDC(hWnd, hDC);

	//tag
	glPushMatrix();
	//glTranslatef(Tag.x, Tag.y, Tag.z);
	glTranslatef(Tag.x, -Tag.z, Tag.y);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluSphere(tag, 1.0, 24, 24);
	glPopMatrix();

	////anchor1 - 빨간구
	//glPushMatrix();
	//glTranslatef(0.0f, 0.0f, 0.0f);
	//glColor3f(1.0f, 0.0f, 0.0f);
	//gluSphere(anchor_1, ywStruct.distance_1 / 10.0f, 24, 24);
	//glPopMatrix();

	//// anchor2 - 초록구
	//glPushMatrix();
	//glTranslatef(width, 0.0f, 0.0f);
	//glColor3f(0.0f, 1.0f, 0.0f);
	//gluSphere(anchor_2, ywStruct.distance_2 / 10.0f, 24, 24);
	//glPopMatrix();

	//// anchor2 - 파란구
	//glPushMatrix();
	//glTranslatef(0.0f, 0.0f, height);
	//glColor3f(0.0f, 0.0f, 1.0f);
	//gluSphere(anchor_3, ywStruct.distance_3 / 10.0f, 24, 24);
	//glPopMatrix();

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