#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<gl\GL.h>
#include <gl/GLU.h>
#include<vector>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")

#define MY_NAME_INITIAL "JCG"

#define TRUE					1
#define FALSE					0

#define BUFFER_SIZE				256
#define S_EQUAL					0
#define WIN_INIT_X				100
#define WIN_INIT_Y				100
#define WIN_WIDTH				800
#define WIN_HEIGHT				600

#define VK_F					0x46
#define VK_f					0x60

#define NR_POINT_COORDS			3
#define NR_TEXTURE_COORDS		2
#define NR_NORMAL_COORDS		3
#define NR_FACE_TOKENS			3
#define FOY_ANGLE				45
#define ZNEAR					0.1
#define ZFAR					200.0

#define VIEWPORT_BOTTOMLEFT_X	0
#define VIEWPORT_BOTTOMLEFT_Y	0

#define MONKEYHEAD_X_TRANSLATE  0.0f
#define MONKEYHEAD_Y_TRANSLATE  0.0f
#define MONKEYHEAD_Z_TRANSLATE  -5.0f

#define MONKEYHEAD_X_SCALE_FACTOR 1.5f
#define MONKEYHEAD_Y_SCALE_FACTOR 1.5f
#define MONKEYHEAD_Z_SCALE_FACTOR 1.5f

#define START_ANGLE_POS			  0.0f
#define END_ANGLE_POS			  360.0f
#define MONKEYHEAD_ANGLE_INCREMENT 1.0f

//Macros for error handling
#define ERRORBOX1(lpszErrorMessage, lpszCaption) {													   \
													MessageBox((HWND)NULL, TEXT(lpszErrorMessage),     \
													TEXT(lpszCaption), MB_ICONERROR);				   \
													ExitProcess(EXIT_FAILURE);						   \
												 }

#define ERRORBOX2(hWnd, lpszErrorMessage, lpszCaption) {													   \
													MessageBox(hWnd, TEXT(lpszErrorMessage),     \
													TEXT(lpszCaption), MB_ICONERROR);				   \
													DestroyWindow(hWnd);						   \
												 }



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool gbIsFullScreen = false;
HWND ghwnd;
bool gbActiveWindow = false;
bool gbEscapeKeyPressed = false;
HDC ghdc;
HGLRC ghrc;
GLfloat light_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat light_diffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat light_specular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat light_position[] = { 1.0f,1.0f,1.0f,0.0f };

GLfloat material_specular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_shininess[] = { 50.f };
GLboolean isLightingOn = false;
GLfloat g_rotate;
//vertex data
std::vector<std::vector<float>> g_vertices;
std::vector<std::vector<float>> g_texture;
std::vector<std::vector<float>> g_normals;
std::vector<std::vector<int>> g_face_tri, g_face_texture, g_face_normals;

//File I/O
FILE *g_fp_logfile = NULL, *g_fp_meshfile=NULL;
char line[BUFFER_SIZE];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	void initialize();
	void update();
	void uninitialize();

	void display();
	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("Model Loading-MonkeyHead");
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
			display();
			update();
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
	void LoadMeshData(void);
	void uninitialize(void);
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	g_fp_logfile = fopen("MONKEYHEADLOADER.LOG", "w");
	if (!g_fp_logfile) {
		ERRORBOX1("Unable to create log file. Exiting", "Error");
		uninitialize();
	}

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
	pfd.cDepthBits = 24;
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

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	LoadMeshData();
	//Lighting code
	/*glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);

	glEnable(GL_LIGHT0);*/

	resize(WIN_WIDTH, WIN_HEIGHT);
}
void resize(int width, int height) {
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void LoadMeshData(void) {
	void uninitialize(void);
	g_fp_meshfile = NULL;
	g_fp_meshfile=fopen("MonkeyHead.OBJ", "r");
	if (g_fp_logfile==NULL) {
		ERRORBOX1("Unable to open Monkeyhead.OBJ.Exiting", "Error");
		uninitialize();
	}
	/*else
		ERRORBOX1("Obj file opened successfully", "SUCCESS");
*/
	char *sep_space = " ";
	char *sep_fslash = "/";

	char *first_token = NULL;
	char* token = NULL;
	char* face_tokens[NR_FACE_TOKENS];
	int nr_tokens;

	char* token_vertex_index = NULL;
	char* token_texture_index = NULL;
	char* token_normal_index = NULL;

	while (fgets(line, BUFFER_SIZE, g_fp_meshfile) != NULL) {
		first_token = strtok(line, sep_space); //will give us type of line(i.e. v, vt, vn or f)

		if (strcmp(first_token, "v") == S_EQUAL) { //vertices
			std::vector<float> vec_point_coord(NR_POINT_COORDS);
			for (int i = 0;i != NR_POINT_COORDS;i++) {
				vec_point_coord[i] = atof(strtok(NULL, sep_space));
			}
			g_vertices.push_back(vec_point_coord);
		}
		else if (strcmp(first_token, "vt") == S_EQUAL) { //texture coordinates
			std::vector<float> vec_texture_coord(NR_TEXTURE_COORDS);
			for (int i = 0;i != NR_TEXTURE_COORDS;i++) {
				vec_texture_coord[i] = atof(strtok(NULL, sep_space));
			}
			g_texture.push_back(vec_texture_coord);
		}
		else if (strcmp(first_token, "vn") == S_EQUAL) { //normals
			std::vector<float> vec_normal_coord(NR_NORMAL_COORDS);
			for (int i = 0;i != NR_NORMAL_COORDS;i++) {
				vec_normal_coord[i] = atof(strtok(NULL, sep_space));
			}
			g_normals.push_back(vec_normal_coord);
		}
		else if (strcmp(first_token, "f") == S_EQUAL) {  //face data 
			std::vector<int> triangle_vertex_indices(3), texture_vertex_indices(3), normal_vertex_indices(3);

			memset((void*)face_tokens, 0, NR_FACE_TOKENS);
			nr_tokens = 0;
			while (token = strtok(NULL, sep_space)) {
				if (strlen(token) < 3) {
					break;
				}
				face_tokens[nr_tokens] = token;
				nr_tokens++;
			}

			for (int i = 0;i != NR_FACE_TOKENS;i++) {
				token_vertex_index = strtok(face_tokens[i], sep_fslash);
				token_texture_index = strtok(NULL, sep_fslash);
				token_normal_index = strtok(NULL, sep_fslash);

				triangle_vertex_indices[i] = atoi(token_vertex_index);
				texture_vertex_indices[i] = atoi(token_texture_index);
				normal_vertex_indices[i] = atoi(token_normal_index);				
			}
			g_face_tri.push_back(triangle_vertex_indices);
			g_face_texture.push_back(texture_vertex_indices);
			g_face_normals.push_back(normal_vertex_indices);
		}
		memset((void*)line, (int)'\0', BUFFER_SIZE);
	}

	fprintf(g_fp_logfile, "g_vertices:%llu g_texture:%llu g_normals:llu g_face_tri:%llu",
		g_vertices.size(), g_texture.size(), g_normals.size(), g_face_tri.size());
	fclose(g_fp_meshfile);
	g_fp_meshfile = NULL;
}

void display() {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(MONKEYHEAD_X_TRANSLATE, MONKEYHEAD_Y_TRANSLATE, MONKEYHEAD_Z_TRANSLATE);
	glRotatef(g_rotate, 0.0f, 1.0f, 0.0f);
	glScalef(MONKEYHEAD_X_SCALE_FACTOR, MONKEYHEAD_Y_SCALE_FACTOR, MONKEYHEAD_Z_SCALE_FACTOR);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (int i = 0;i != g_face_tri.size();i++) {
		glBegin(GL_TRIANGLES);
		for (int j = 0;j != g_face_tri[i].size();j++) {
			int vi = g_face_tri[i][j] - 1;
			glVertex3f(g_vertices[vi][0], g_vertices[vi][1], g_vertices[vi][2]);
		}
		glEnd();
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
		case VK_F:
		case VK_f:	//f
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

void update(void) {
	g_rotate = g_rotate + MONKEYHEAD_ANGLE_INCREMENT;
	if (g_rotate >= END_ANGLE_POS)
		g_rotate = START_ANGLE_POS;
}