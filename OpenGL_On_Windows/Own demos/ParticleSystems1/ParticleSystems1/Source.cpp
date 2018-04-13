#include<iostream>
#include <windows.h>
#include <stdio.h>
#include<fstream>
#include <stdlib.h>
#include <gl\glew.h>
#include<gl\GL.h>
#include"vmath.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"opengl32.lib")

#define MY_NAME_INITIAL "JCG"
using namespace vmath; //v for vermileon
#define GRID_X 1024
#define GRID_Y 1024
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool gbIsFullScreen = false;
HWND ghwnd;
HDC ghdc;
HGLRC ghrc;
bool gbActiveWindow = false;
bool gbEscapeKeyPressed = false;
std::ofstream g_log_file;

enum {
	JCG_ATTRIBUTE_VERTEX = 0,
	JCG_ATTRIBUTE_COLOR,
	JCG_ATTRIBUTE_NORMAL,
	JCG_ATTRIBUTE_VELOCITY,
	JCG_ATTRIBUTE_START_TIME_ARRAY
};

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_particles;
GLuint gVbo_color_particles;
GLuint gVbo_position_particles;
GLuint gVbo_velocity_particles;
GLuint gVbo_start_time_particles;
GLuint gVao_cube;
GLuint gVbo_color_cube;
GLuint gVbo_position_cube;
GLuint gMVPUniform;
GLuint gLocation;

mat4 gPerspectiveProjectionMatrix;

GLfloat angle_pyramid = 0.0f, angle_cobe = 0.0f;
static GLint arrayWidth, arrayHeight;
static GLfloat *verts = NULL;
static GLfloat *colors = NULL;
static GLfloat *velocities = NULL;
static GLfloat *startTimes = NULL;
GLfloat particleTime = 0.0f;
//fwd declaration
void createPoints(GLint w, GLint h);
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
	TCHAR szAppName[] = TEXT("PP- Particle systems1");
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
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	g_log_file.open("Log.txt", std::ios::out);
	initialize();

	if (!g_log_file.is_open()) {
		std::cout << "Failed to open log new file" << std::endl;
		uninitialize();
	}
	else
		g_log_file << "Log file successfully created!" << std::endl;

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

void initialize_shaders() {
	void uninitialize();
	//-----VERTEX SHADER----
	//Create vertex shader. Vertex shader specialist
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode =         //Source code of Vertex shader
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"out vec4 out_color;" \
		"uniform mat4 u_mvp_matrix;" \
		"uniform float Time;" \
		"in vec3 Velocity;" \
		"in float StartTime;" \
		"void main(void)" \
		"{" \
		"vec4 vert;" \
		"float t = Time - StartTime;" \
		"if(t >= 0.0)" \
		"{" \
		"/* Calculate new position by using kinematic equation. p = p(ini) + vt + 0.5*a*t(sqr)  */" \
		" vert = vPosition + vec4(Velocity * t, 0.0);" \
		" vert.y -= 4.9 *t *t; /* -ve Y bcz of -ve acceleration due to gravity.*/"\
		" out_color = vColor;" \
		"}" \
		"else" \
		"{" \
		"/* Particles are not born yet  */" \
		"  vert = vPosition;" \
		"  out_color = vec4(0.0 , 0.0, 0.0, 1.0);" \
		"" \
		"}" \
		"gl_Position = u_mvp_matrix * vert;" \
		"}";

	glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL); //NULL is for NULL terminated source code string

																						   //compile vertex shader
	glCompileShader(gVertexShaderObject);
	//Shader compilation error checking goes here...
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char* szInfoLog = NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE) {
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0) {
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				g_log_file << "Vertex shader compilation log:" << szInfoLog << std::endl;
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	//-----FRAGMENT SHADER----
	//Create fragment shader. Fragment shader specialist
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//source code of fragment shader
	const GLchar *fragmentShaderSourceCode =      //Source code of Fragment shader
		"#version 450 core" \
		"\n" \
		"in vec4 out_color;"
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = out_color;" \
		"}";
	glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

	//compile fragment shader
	glCompileShader(gFragmentShaderObject);
	//Shader compilation errors goes here..
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE) {
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0) {
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL) {
				GLsizei written = 0;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				g_log_file << "Fragment shader compilation log:" << szInfoLog << std::endl;
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
}
void initialize() {
	void resize(int, int);
	void uninitialize();
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char* szInfoLog = NULL;
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
	GLenum glew_error = glewInit();	//Turn ON all graphic card extension
	if (glew_error != GLEW_OK) {
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
		uninitialize();
	}
	
	createPoints(GRID_X,GRID_Y);

	//--------
	const GLubyte *glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    g_log_file<<"GLSL version is:"<< glsl_version <<std::endl;
	//----------SHADERS-----------
	initialize_shaders();
	
	//Create shader program
	gShaderProgramObject = glCreateProgram();

	//attach shaders to the program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	//map our(RAM) memory identifier to GPU memory(VRAM) identifier
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_COLOR, "vColor");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_VELOCITY, "Velocity");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_START_TIME_ARRAY, "StartTime");

	//Link shader program
	glLinkProgram(gShaderProgramObject);
	//Linking error checks goes here...

	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE) {
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0) {
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				g_log_file << "Shader program linking log:" << szInfoLog << std::endl;
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	//Preparation to put our dynamic(uniform) data into the shader
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	gLocation = glGetUniformLocation(gShaderProgramObject, "Time");
	// Vertices, colors, shader attribs, vbo, vao initializations

	//create a vao
	glGenVertexArrays(1, &gVao_particles);
	glBindVertexArray(gVao_particles);
	//set position
	glGenBuffers(1, &gVbo_position_particles);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_particles);
	int size = arrayWidth*arrayHeight;
	glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);

	//vbo for colors	
	glGenBuffers(1, &gVbo_color_particles);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_color_particles);
	glBufferData(GL_ARRAY_BUFFER, size, colors, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_COLOR);

	//vbo for velocity
	glGenBuffers(1, &gVbo_velocity_particles);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_velocity_particles);
	glBufferData(GL_ARRAY_BUFFER, (arrayWidth * arrayHeight * 3 * sizeof(float)), velocities, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_VELOCITY, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VELOCITY);
	
	//vbo for start time
	glGenBuffers(1, &gVbo_start_time_particles);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_start_time_particles);
	glBufferData(GL_ARRAY_BUFFER, sizeof(arrayWidth * arrayHeight * sizeof(float)), startTimes, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_START_TIME_ARRAY, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_START_TIME_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);  //done with pyramid vao


	glShadeModel(GL_SMOOTH);
	// set-up depth buffer
	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// set really nice percpective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	// We will always cull back faces for better performance

	// set background color to which it will display even if it will empty. THIS LINE CAN BE IN drawRect().
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black

	gPerspectiveProjectionMatrix = mat4::identity();

	resize(800, 600);
}

//FUNCTION DEFINITIONS
void createPoints(GLint w, GLint h)
{
	GLfloat *vptr, *cptr, *velptr, *stptr;
	GLfloat i, j;
	if (verts != NULL)
		free(verts);
	verts = (float*)malloc(w * h * 3 * sizeof(float));
	colors = (float*)malloc(w * h * 3 * sizeof(float));
	velocities = (float*)malloc(w * h * 3 * sizeof(float));
	startTimes = (float*)malloc(w * h * sizeof(float));
	vptr = verts;
	cptr = colors;
	velptr = velocities;
	stptr = startTimes;

	for (i = 0.5 / w - 0.5; i < 0.5;i = i + 1.0 / w) {
		for (j = 0.5 / h - 0.5; j < 0.5;j = j + 1.0 / h) {
			/* Assign initial position. Keep Y to origin.*/
			*vptr = i-1.5;
			*(vptr + 1) = 0.0;
			*(vptr + 2) = j;
			vptr += 3; /* Jump to next vertex */

			/*Generate random color values beween 0.5 to 1.0 for bright colors*/
			*cptr = ((float)rand() / RAND_MAX)*0.5 + 0.5; //R
			*(cptr + 1) = ((float)rand() / RAND_MAX)*0.5 + 0.5; //G
			*(cptr + 2) = ((float)rand() / RAND_MAX)*0.5 + 0.5; //B
			cptr += 3; /* Jump to next color */

			/*Generate random velocities*/
			*velptr = (((float)rand()/RAND_MAX))*7.0; /* We want more incliend towards X*/
			*(velptr+1) = (((float)rand() / RAND_MAX)) * 10.0; /* Strong bias to Y bcz acceleration due to gravity affects Y*/
			*(velptr + 2) = (((float)rand() / RAND_MAX)) + 1.0; /* Less inclination towards Z*/
			*(velptr + 2) = 0.0f;
			velptr += 3;
			*stptr = ((float)rand()/RAND_MAX)*10.0; /* Random start time*/
			stptr++;
		}
	}
	arrayWidth = w;
	arrayHeight = h;
}
void resize(int width, int height) {
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)width / (GLfloat)height),0.1f,100.0f);

}

void display() {
	void update();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Start using shader program object
	glUseProgram(gShaderProgramObject); //run shaders
	glPointSize(3.0);
	//OpenGL drawing

	//set modleview and projection matrices to identity matrix

	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();
	
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
	
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP

	// Pass above model view matrix projection matrix to vertex shader in 'u_mvp_matrix' shader variable

	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	update();//update particle time
	
	glBindVertexArray(gVao_particles);
	// Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_POINTS, 0, arrayWidth*arrayHeight); 

	glBindVertexArray(0);
	
	//stop using shaders
	glUseProgram(0);

	SwapBuffers(ghdc);
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
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam)) {
		case VK_ESCAPE:
			gbEscapeKeyPressed = true;
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
	particleTime += 0.001f;
	glUniform1f(gLocation, particleTime);
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

	if (gVao_particles) {
		glDeleteVertexArrays(1, &gVao_particles);
		gVao_particles = 0;
	}

	if (gVao_cube) {
		glDeleteVertexArrays(1, &gVao_cube);
		gVao_cube = 0;
	}

	if (gVbo_position_particles) {
		glDeleteBuffers(1, &gVbo_position_particles);
		gVbo_position_particles = 0;
	}
	if (gVbo_position_cube) {
		glDeleteBuffers(1, &gVbo_position_cube);
		gVbo_position_cube = 0;
	}
	if (gVbo_color_particles) {
		glDeleteBuffers(1, &gVbo_color_particles);
		gVbo_color_particles = 0;
	}
	if (gVbo_color_cube) {
		glDeleteBuffers(1, &gVbo_color_cube);
		gVbo_color_cube = 0;
	}
	//Detach shaders
	//Detach vertex shader
	glDetachShader(gShaderProgramObject, gVertexShaderObject);

	//Detach fragment shader
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	//Now delete shader objects

	//Delete vertex shader object
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;

	//Delete fragment shader object
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	//Delete shader program object
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	//unlink shader program
	glUseProgram(0);

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;
	if (g_log_file.is_open()) {
		g_log_file << "Closing log file";
		g_log_file.close();
	}
	DestroyWindow(ghwnd);
}

