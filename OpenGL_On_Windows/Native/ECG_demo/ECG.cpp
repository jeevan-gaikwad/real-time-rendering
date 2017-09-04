#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include<time.h>>
#include<gl\GL.h>
#include<gl/GLU.h>
#include"resource3.h"
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#define MY_NAME_INITIAL "JCG"
#define ID_TIMER 1
#define NO_OF_SECONDS 160
#define MAX_POINTS_LIMIT 25000

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool gbIsFullScreen = false;
HWND ghwnd;
bool gbActiveWindow=false;
bool gbEscapeKeyPressed = false;
HDC ghdc;
HGLRC ghrc;
GLfloat gfCurrentCardioX=-1.0f, gfCurrentCardioY=0.0f;
GLfloat gfEndCardioX = 0.8f, gfEndCardioY = 0.0f;
bool IsCardioLineDrawDone = false;

float randomFloat(float a, float b);
int currentHeight;
int currentWidth;
float xView, yView, zView=1.8f;
float graphRotationAngle;
bool drawSpehere = false;
typedef struct point {
	GLfloat x, y;
}point_t;
typedef struct cardioLinePoint {
	point_t current;
	point_t previous;
	float distance;
}cardioLinePoint_t;
typedef struct cardioLine {
	cardioLinePoint position[NO_OF_SECONDS];
	GLint arrayLength;
	point_t tipPosition;
}cardioLine_t;

cardioLine_t cardioLine;
point_t linePoints[MAX_POINTS_LIMIT];
GLfloat pointsArray[MAX_POINTS_LIMIT][2];
int lineEndIndex = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{

	void initialize();
	void uninitialize();
	void move();
	void display();
	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("ECG simulation");	
	
	bool bDone = false;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW|CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);

	srand((unsigned int)time(NULL));

	if (!RegisterClassEx(&wndclass)) {
		MessageBox(NULL, TEXT("Failed to register wndclass. Exiting"), TEXT("Error"), MB_OK);
		exit(EXIT_FAILURE);
	}
	iScreenWidth = GetSystemMetrics(0);
	iScreenHeight = GetSystemMetrics(1);
	//fp = fopen("counter.txt", "w");
	
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,szAppName,
		szAppName,
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE,
		((iScreenWidth / 2) - 400), ((iScreenHeight / 2) - 300),
		800,600,
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
			move();
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
	void drawGraphPaperPattern();
	
	void resize(int,int);
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	resize(800, 600);
}
void resize(int width, int height) {
	currentHeight = height;
	currentWidth = width;
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void drawGraphPaperPattern() {
	float i = 0.0f;

	//Draw 20 lines above and below the green line
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.5f, 0.0f);

	for (i=0.048f;i <= 1.0f;i = i + 0.048f) {
		glVertex3f(-i, 1.0f, 0.0f);
		glVertex3f(-i, -1.0f, 0.0f);

		glVertex3f(i, 1.0, 0.0f);
		glVertex3f(i, -1.0, 0.0f);
	}
	glEnd();

	glLineWidth(6);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.4f, 0.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glEnd();

	//Draw 20 lines above and below the red line
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.5f, 0.0f);

	for (i = 0.048f;i <= 1.0f;i = i + 0.048f) {
		glVertex3f(-1.0f, i, 0.0f);
		glVertex3f(1.0f, i, 0.0f);

		glVertex3f(-1.0f, -i, 0.0f);
		glVertex3f(1.0f, -i, 0.0f);
	}
	glEnd();
	glLineWidth(1);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);
	glEnd();
}

void display() {
	void resetCardioLine();
	void drawCircleUsingPoints(GLfloat radius);
	static bool eraseAll = true;
	GLUquadric *quadric = NULL;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(xView, yView, zView, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(graphRotationAngle, 1.0f,0.0f, 0.0f);
	drawGraphPaperPattern();
	glRotatef(-graphRotationAngle, 1.0f, 0.0f, 0.0f);
	if (IsCardioLineDrawDone == false) {
		//draw backlog first(line)
		
		glColor3f(0.0f, 0.8f, 0.0f);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glPointSize(5.0f);
		glVertexPointer(2, GL_FLOAT, 0, pointsArray);
		glDrawArrays(GL_POINTS, 0, lineEndIndex);
		glDisableClientState(GL_VERTEX_ARRAY);
		
		glTranslatef(cardioLine.tipPosition.x, cardioLine.tipPosition.y, 0.0f);
		glColor3ub(0, 255, 128);
		
		if (drawSpehere) {
			quadric = gluNewQuadric();
			gluSphere(quadric, 0.015f, 30, 30);
			gluDeleteQuadric(quadric);
		}
		else {
			glPointSize(2.0f);
			drawCircleUsingPoints(0.02f);
		}
		
		pointsArray[lineEndIndex][0] = cardioLine.tipPosition.x;
		pointsArray[lineEndIndex][1] = cardioLine.tipPosition.y;
		if (gfCurrentCardioX < gfEndCardioX)
			lineEndIndex = (++lineEndIndex) % MAX_POINTS_LIMIT;
		else {
			lineEndIndex = 0;
		}
	}
	else {
		//reset start coordinates
		resetCardioLine();
		eraseAll = true;
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

GLfloat getDistance(point_t p1, point_t p2) {
	float t1 = (float)pow((p1.x - p2.x), 2);
	float t2 = (float)pow((p1.y - p2.y), 2);
	float t3 = t1 + t2;
	float dist = sqrt(t3);

	return dist;
}
void generateRandomNumbersArray(int count) {
	GLfloat randNum;
	GLfloat distance = 0.0;
	GLint index = 0;
	float randStart = randomFloat(0.35f, 0.5f);
	point_t start = { -1.0f,randStart }, end = { -1.0f,0.0f };
	GLfloat x = -0.9f;
	bool flag = false;
	for (index = 1;index < count-1;index++) {
		cardioLine.position[index].previous.x = cardioLine.position[index - 1].current.x;
		cardioLine.position[index].previous.y = cardioLine.position[index - 1].current.y;

		start.x = cardioLine.position[index].previous.x;
		start.y = cardioLine.position[index].previous.y;
		x = x + 0.05f;
		end.x = x;
		
		if (index % 2 == 0) {
			randNum = randomFloat(0.3f, 0.5f);// generate a random Y position
			flag = true;
		}
		else
			randNum = 0.0f;
	
		if (index % 3 == 0) {
			end.y = randNum;
			distance = getDistance(start, end);
			cardioLine.position[index].distance = distance;
			cardioLine.position[index].current.y = end.y;
			cardioLine.position[index].current.x = end.x;

			cardioLine.position[index + 1].previous.y = end.y;
			cardioLine.position[index + 1].previous.x = end.x;
			if (flag) {
				randNum = randomFloat(0.0f, -0.25f);
				flag = false;
			}else
				randNum = 0.0f;
			start.x = end.x;
			start.y = end.y;
			x = x + 0.05f;
			end.x= x;
			end.y = randNum;
			distance = getDistance(start, end);
			cardioLine.position[index + 1].distance = distance;
			cardioLine.position[index + 1].current.y = end.y;
			cardioLine.position[index + 1].current.x = end.x;
			index++;
			continue;
		}
		else {
			
				end.y = 0.0f;
		}
		distance = getDistance(start, end);
		cardioLine.position[index].distance = distance;
		cardioLine.position[index].current.y = end.y;
		cardioLine.position[index].current.x = end.x;
		
	}
	
}
DWORD WINAPI PlayHeartBeatSound(_In_ LPVOID lpParameter) {
	//PlaySound(TEXT("heart_bit_sound.wav"), NULL, SND_FILENAME);
	PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE);
	return 0;
}
void drawCircleUsingPoints(GLfloat radius) {
	const GLfloat PI = 3.41159f;
	GLfloat angle = 0.0f;
	float aspectRatio = 0.0f;

	aspectRatio = (currentHeight*1.0f) / currentWidth;

	glBegin(GL_POINTS);
	for (angle = 0.0f;angle < 2 * PI;angle = angle + 0.001) {
		glVertex3f(aspectRatio*cos(angle)*radius, aspectRatio*sin(angle)*radius, 0.0f);
	}
	glEnd();

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

	
	
	void resetCardioLine();
	switch (iMsg)
	{
	case WM_CREATE:
		SetTimer(hwnd, ID_TIMER, 500, NULL);
		
		cardioLine.position[0].current.y = 0.0f;
		cardioLine.position[0].current.x = -1.0f;
		cardioLine.position[0].previous.x = -1.0f;
		cardioLine.position[0].previous.y = 0.0f;
		cardioLine.position[0].distance = 0.0f;
		resetCardioLine();
		break;
	case WM_TIMER:
		
		break;
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 0x78://x
			xView += 0.01f;
			break;
		case 0x58://X
			xView -= 0.01f;
			break;
		case 0x79://y
			yView += 0.01f;
			break;
		case 0x59://Y
			yView -= 0.01f;
			break;
		case 0x7a://z
			zView += 0.01f;
			break;
		case 0x5a://Z
			zView -= 0.01f;
			break;
		case 0x72://r
			graphRotationAngle += 0.5f;
		case 0x52://R
			graphRotationAngle -= 0.5f;
			break;
		case 0x73://s
			if (drawSpehere == false)
				drawSpehere = true;
			else
				drawSpehere = false;
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam)) {
		case VK_ESCAPE:
			gbEscapeKeyPressed = true;
			break;
		case 0x46:	//f
			dwStyle = GetWindowLong(hwnd, GWL_STYLE);
			if (gbIsFullScreen == false) {
				zView -= 0.5f;
				if (dwStyle & WS_OVERLAPPEDWINDOW) {
					wpPrev.length = sizeof(WINDOWPLACEMENT);
					isWp = GetWindowPlacement(hwnd, &wpPrev);
					hMonitor = MonitorFromWindow(hwnd, MONITORINFOF_PRIMARY);
					monitorInfo.cbSize = sizeof(MONITORINFO);
					isMonitorInfo = GetMonitorInfo(hMonitor, &monitorInfo);
					if (isWp == TRUE && isMonitorInfo) {
						SetWindowLong(hwnd,GWL_STYLE,dwStyle&~WS_OVERLAPPEDWINDOW);
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
			} else { //restore
					SetWindowLong(hwnd,GWL_STYLE ,dwStyle | WS_OVERLAPPEDWINDOW| WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
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
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}
bool getPointOnLine(point_t start, point_t end, GLfloat at, float distance, point_t* pointOnLine) {


	float t = at / distance;
	float x = (1 - t)*start.x + t*end.x;
	float y = (1 - t)*start.y + t*end.y;
	if (pointOnLine) {
		pointOnLine->x = x;
		pointOnLine->y = y;
	}
	else
		return false;
	return true;
}
void resetCardioLine() {
	cardioLine.tipPosition.x = -1.0f;
	cardioLine.tipPosition.y = 0.0f;
	gfCurrentCardioX = -1.0f;
	gfCurrentCardioY = 0.0f;
	IsCardioLineDrawDone = false;
	generateRandomNumbersArray(NO_OF_SECONDS);
}
void move() {
	static int index=0;
	static GLfloat d2 = 0.0f;
	static point_t start, end;
	point_t pointOnLine;
	static float distanceBetCurrPoints=0.0f;
	
	if (gfCurrentCardioX < gfEndCardioX ) {
		gfCurrentCardioX = gfCurrentCardioX + 0.0001f;
	}
	else {
		IsCardioLineDrawDone = true;
		index = 0;
		lineEndIndex = 0;
		return;
	}
	
	if (d2 < distanceBetCurrPoints) {
		//calculate new Y directing towards currentY
		d2 = d2 + 0.0005;
		getPointOnLine(start, end, d2, distanceBetCurrPoints,&pointOnLine);
		cardioLine.tipPosition.y = pointOnLine.y;
		cardioLine.tipPosition.x = pointOnLine.x;
	}
	else {
		//pick new Y from array
		
		//Play heart beat sound asynchronously for peak value
		if (end.y > 0.2) {
			CreateThread(
				NULL,                   // default security attributes
				0,                      // use default stack size  
				PlayHeartBeatSound,       // thread function name
				NULL,          // argument to thread function 
				0,                      // use default creation flags 
				NULL);

		}
		end.y = cardioLine.position[index].current.y;
		end.x= cardioLine.position[index].current.x;
		start.x= cardioLine.position[index].previous.x;
		start.y= cardioLine.position[index].previous.y;
		distanceBetCurrPoints = cardioLine.position[index].distance;
		index++;
		d2 = 0.0f;
					
	}
	
}
float randomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}