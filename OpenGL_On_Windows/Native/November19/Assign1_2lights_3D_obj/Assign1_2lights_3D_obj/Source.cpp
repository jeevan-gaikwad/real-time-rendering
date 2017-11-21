#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<gl\GL.h>
#include<gl\glu.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#define MY_NAME_INITIAL "JCG"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool gbIsFullScreen = false;
HWND ghwnd;
bool gbActiveWindow = false;
bool gbEscapeKeyPressed = false;
HDC ghdc;
HGLRC ghrc;
GLfloat angle_pyramid = 0.0f;
GLfloat angle_cube = 0.0f;
GLfloat angle_sphere = 0.0f;
/*Lights on 3D objects*/
GLboolean isLightingOn = false;
//Light0
GLfloat light0_ambient[] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat light0_diffuse[] = { 1.0f,0.0f,0.0f,0.0f };
GLfloat light0_specular[] = { 1.0f,0.0f,0.0f,0.0f };
GLfloat light0_position[] = { 2.0f,1.0f,1.0f,0.0f }; //right top 
//Light1
GLfloat light1_ambient[] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat light1_diffuse[] = { 0.0f,0.0f,1.0f,0.0f };
GLfloat light1_specular[] = { 0.0f,0.0f,1.0f,0.0f };
GLfloat light1_position[] = { -2.0f,1.0f,1.0f,0.0f }; //left top
//Material
GLfloat material_ambient[]={ 0.0f,0.0f,0.0f,0.0f };
GLfloat material_diffuse[]={ 1.0f,1.0f,1.0f,1.0f };
GLfloat material_specular[]={ 1.0f,1.0f,1.0f,1.0f };
GLfloat material_shininess = 50.0f;

GLboolean isPyramidOn = GL_TRUE;
GLboolean isCubeOn = GL_FALSE;
GLboolean isSphereOn = GL_FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	void initialize();
	void uninitialize();
	void update();
	void display();
	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("2 Lights with 3D objects ");
	bool bDone = false;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


	if (!RegisterClassEx(&wndclass)) {
		MessageBox(NULL, TEXT("Failed to register wndclass. Exiting"), TEXT("Error"), MB_OK);
		exit(EXIT_FAILURE);
	}
	iScreenWidth = GetSystemMetrics(0);
	iScreenHeight = GetSystemMetrics(1);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName,
		szAppName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		((iScreenWidth / 2) - 400), ((iScreenHeight / 2) - 300),
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;
	initialize();
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (bDone == false) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			update();
			display();
			if (gbActiveWindow == true) {
				if (gbEscapeKeyPressed == true) {
					bDone = true;
				}
			}
		}
	}
	uninitialize();
	return((int)msg.wParam);
}
void initialize() {
	void resize(int, int);
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;
	ghdc = GetDC(ghwnd);
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//light0 i.e Red light properties
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glEnable(GL_LIGHT0); //turn ON light0
	//light1 i.e Blue light properties
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glEnable(GL_LIGHT1);//turn ON light1
	
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, material_shininess);

	resize(800, 600);
}
void resize(int width, int height) {
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void drawCube() {
	glLineWidth(1);
	glBegin(GL_QUADS);
	//front face
	glNormal3f(0.0f, 0.0, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(-1.0f, 1.0f, 1.0f); //left top
	glVertex3f(-1.0f, -1.0f, 1.0f); //left bottom
	glVertex3f(1.0f, -1.0f, 1.0f); //right bottom
	glVertex3f(1.0f, 1.0f, 1.0f); //right top

	glNormal3f(1.0f, 0.0, 0.0f);		  //right face
	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, 1.0f, 1.0f); //left top
	glVertex3f(1.0f, -1.0f, 1.0f); //left bottom
	glVertex3f(1.0f, -1.0f, -1.0f); //right bottom

	glVertex3f(1.0f, 1.0f, -1.0f); //right top

	glNormal3f(0.0f, 0.0, -1.0f);			   //back face
	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, 1.0f, -1.0f); // left top
	glVertex3f(1.0f, -1.0f, -1.0f); //left bottom
	glVertex3f(-1.0f, -1.0f, -1.0f); //right bottom
	glVertex3f(-1.0f, 1.0f, -1.0f); //right top

	glNormal3f(-1.0f, 0.0, 0.0f);					//left face
	glColor3ub(100, 149, 237); //corn flower
	glVertex3f(-1.0f, 1.0f, -1.0f); //left top
	glVertex3f(-1.0f, -1.0f, -1.0f);; //left bottom
	glVertex3f(-1.0f, -1.0f, 1.0f); //left bottom
	glVertex3f(-1.0f, 1.0f, 1.0f); //left top

	glNormal3f(0.0f, 1.0, 0.0f);				   //top face
	glColor3ub(255, 0, 255); //corn flower
	glVertex3f(-1.0f, 1.0f, 1.0f); //left top
	glVertex3f(1.0f, 1.0f, 1.0f); //left top
	glVertex3f(1.0f, 1.0f, -1.0f);; //left bottom
	glVertex3f(-1.0f, 1.0f, -1.0f); //left bottom

	glNormal3f(0.0f, -1.0, 0.0f);					//bottom face
	glColor3ub(255, 128, 0); //corn flower
	glVertex3f(-1.0f, -1.0f, 1.0f); //left top
	glVertex3f(1.0f, -1.0f, 1.0f); //left top
	glVertex3f(1.0f, -1.0f, -1.0f);; //left bottom
	glVertex3f(-1.0f, -1.0f, -1.0f); //left bottom


	glEnd();
}

void drawPyramid() {
	glLineWidth(1);
	glBegin(GL_TRIANGLES);
	//front face
	glNormal3f(0.0f, 0.447214f, 0.894427f);
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(-1.0f, -1.0f, 1.0f); //left bottom

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, -1.0f, 1.0f); //right bottom

	glNormal3f(0.894427f, 0.447214f, 0.0f);			   //right face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, -1.0f, 1.0f);  //left bottom

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, -1.0f, -1.0f); //right bottom

	glNormal3f(0.0f, 0.447214f, -0.894427f);		//back face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, -1.0f, -1.0f); //left bottom

	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(-1.0f, -1.0f, -1.0f); //right bottom

	glNormal3f(-0.894427f, 0.447214f, 0.0f);     //left face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top

	glColor3f(0.0f, 0.0f, 1.0f); //blue

	glVertex3f(-1.0f, -1.0f, -1.0f); //left bottom

	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(-1.0f, -1.0f, 1.0f); //right bottom


	glEnd();
}
void drawSphere() {
	GLUquadric *quadric = NULL;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	quadric = gluNewQuadric();
	gluSphere(quadric, 1.25f, 30, 30);
	gluDeleteQuadric(quadric);
}
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (isPyramidOn == GL_TRUE) {
		//Pyramid rotation
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(-3.0f, 0.0f, -6.5f);
		glRotatef(angle_pyramid, 0.0f, 1.0f, 0.0f);
		drawPyramid();
	}
	else if (isCubeOn == GL_TRUE) {
		//Cube rotation
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.f, 0.0f, -6.5f);
		//glScalef(0.75f, 0.75f, 0.75f);
		glRotatef(angle_cube, 0.0f, 1.0f, 0.0f);
		drawCube();
	}
	else if (isSphereOn == GL_TRUE) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(3.0f, 0.0f, -6.5f);
		glRotatef(angle_sphere, 0.0f, 1.0f, 0.0f);
		drawSphere();
	}
	SwapBuffers(ghdc);
}

void uninitialize(void)
{
	//UNINITIALIZATION CODE
	DWORD dwStyle;
	WINDOWPLACEMENT wpPrev;
	if (gbIsFullScreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	DestroyWindow(ghwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	DWORD dwStyle;
	static WINDOWPLACEMENT wpPrev;
	BOOL isWp;
	HMONITOR hMonitor;
	MONITORINFO monitorInfo;
	BOOL isMonitorInfo;
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam)) {
		case VK_ESCAPE:
			gbEscapeKeyPressed = true;
			break;
		case 0x4c://l
			if (isLightingOn) {
				isLightingOn = false;
				glDisable(GL_LIGHTING);
			}
			else {
				isLightingOn = true;
				glEnable(GL_LIGHTING);
			}
			break;
		
		case 0x43://c
			if (isCubeOn == GL_TRUE) {
				isCubeOn = GL_FALSE;
			}
			else {
				isCubeOn = GL_TRUE;
				isPyramidOn = GL_FALSE;
				isSphereOn = GL_FALSE;
			}
			break;
		case 0x50://p
			//isPyramidOn ? GL_FALSE : GLU_TRUE;
			if (isPyramidOn == GL_TRUE) {
				isPyramidOn = GL_FALSE;
			}
			else {
				isPyramidOn = GL_TRUE;
				isCubeOn = GL_FALSE;
				isSphereOn = GL_FALSE;
			}
			break;
		case 0x53://s
			if (isSphereOn == GL_TRUE) {
				isSphereOn = GL_FALSE;
			}
			else {
				isSphereOn = GL_TRUE;
				isCubeOn = GL_FALSE;
				isPyramidOn = GL_FALSE;
			}
			break;
		case 0x46:	//f
			dwStyle = GetWindowLong(hwnd, GWL_STYLE);
			if (gbIsFullScreen == false) {
				if (dwStyle & WS_OVERLAPPEDWINDOW) {
					wpPrev.length = sizeof(WINDOWPLACEMENT);
					isWp = GetWindowPlacement(hwnd, &wpPrev);
					hMonitor = MonitorFromWindow(hwnd, MONITORINFOF_PRIMARY);
					monitorInfo.cbSize = sizeof(MONITORINFO);
					isMonitorInfo = GetMonitorInfo(hMonitor, &monitorInfo);
					if (isWp == TRUE && isMonitorInfo) {
						SetWindowLong(hwnd, GWL_STYLE, dwStyle&~WS_OVERLAPPEDWINDOW);
						SetWindowPos(hwnd, HWND_TOP,
							monitorInfo.rcMonitor.left,
							monitorInfo.rcMonitor.top,
							monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
							monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
							SWP_NOZORDER | SWP_FRAMECHANGED);
						ShowCursor(FALSE);
						gbIsFullScreen = true;
					}
				}
			}
			else { //restore
				SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
				SetWindowPlacement(hwnd, &wpPrev);
				SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
				ShowCursor(TRUE);
				gbIsFullScreen = false;
			}
			break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}
void update() {
	if (angle_pyramid < 360.0f) {
		angle_pyramid = angle_pyramid + 0.1f;
	}
	else {
		angle_pyramid = 0.0f;
	}
	if (angle_cube < 360.f) {
		angle_cube = angle_cube + 0.1f;
	}
	else
		angle_cube = 0.0f;

	if (angle_sphere < 360.f) {
		angle_sphere = angle_sphere + 0.1f;
	}
	else
		angle_sphere = 0.0f;

}
