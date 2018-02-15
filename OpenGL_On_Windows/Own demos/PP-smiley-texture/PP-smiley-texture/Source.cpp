#include<iostream>
#include <windows.h>
#include <stdio.h>
#include<fstream>
#include <stdlib.h>
#include <gl\glew.h>
#include<gl\GL.h>
#include<math.h>
#include"vmath.h"
#include"resource.h"


#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"opengl32.lib")

#define MY_NAME_INITIAL "JCG"
#define PI 3.14159265
using namespace vmath; //v for vermileon
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
	JCG_ATTRIBUTE_TEXTURE0
};

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_quad;
GLuint gVbo_position_qaud;
GLuint gVbo_texture_smiley;

GLuint gVao_smiley_lips;
GLuint gVbo_position_smiley_lips;
GLuint gVbo_texture_smiley_lips;

GLuint gVao_hand;
GLuint gVbo_position_hand;
GLuint gVbo_texture_hand;

GLuint gVao_smiling_text;
GLuint gVbo_position_smiling_text;
GLuint gVbo_texture_smiling_text;

GLuint gMVPUniform;

mat4 gPerspectiveProjectionMatrix;

GLuint gTexture_sampler_uniform; //for uniform(dynamic) texture data
GLuint gTexture_smiley_face;
GLuint gTexture_smiley_lips;
GLuint gTexture_hand;
GLuint gTexture_keep_smiling_text;
GLfloat hand_rotation_angle;
bool gEnableHand = false;
void update();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	void initialize();
	void uninitialize();
	void display();
	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("PP- Smiley");
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
	initialize();
	g_log_file.open("Log.txt", std::ios::out);

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
void initialize_shaders() {
	void uninitialize();
	//-----VERTEX SHADER----
	//Create vertex shader. Vertex shader specialist
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode =         //Source code of Vertex shader
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec2 vTexture0_Coord;" \
		"out vec2 out_texture0_coord;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_texture0_coord = vTexture0_Coord;" \
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
		"in vec2 out_texture0_coord;" \
		"out vec4 FragColor;" \
		"uniform sampler2D u_texture0_sampler;" \
		"void main(void)" \
		"{" \
		"FragColor = texture(u_texture0_sampler, out_texture0_coord);" \
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
	int LoadGLTextures(GLuint *texture, TCHAR imageResourceId[]);
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
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");

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
//				g_log_file << "Shader program linking log:" << szInfoLog << std::endl;
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	//Preparation to put our dynamic(uniform) data into the shader
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	gTexture_sampler_uniform = glGetUniformLocation(gShaderProgramObject, "u_texture0_sampler");
	
	// Vertices, colors, shader attribs, vbo, vao initializations
	
	GLfloat smileyFaceTexcoords[] = {

		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	

	//create a vao for quad1
	glGenVertexArrays(1, &gVao_quad);
	glBindVertexArray(gVao_quad);
	//set quad1 position
	glGenBuffers(1, &gVbo_position_qaud);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_qaud);
	glBufferData(GL_ARRAY_BUFFER, 48, NULL, GL_DYNAMIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &gVbo_texture_smiley);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_texture_smiley);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smileyFaceTexcoords), smileyFaceTexcoords, GL_STATIC_DRAW); //providing texture data statically

	glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL); //note 2. We've s and t for texture coords
	glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);  //done with smiley face vao
	
	//create a vao for smiling lips
	glGenVertexArrays(1, &gVao_smiley_lips);
	glBindVertexArray(gVao_smiley_lips);
	//set quad1 position
	glGenBuffers(1, &gVbo_position_smiley_lips);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_smiley_lips);
	glBufferData(GL_ARRAY_BUFFER, 48, NULL, GL_DYNAMIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gVbo_texture_smiley_lips);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_texture_smiley_lips);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smileyFaceTexcoords), smileyFaceTexcoords, GL_STATIC_DRAW); //providing texture data statically

	glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL); //note 2. We've s and t for texture coords
	glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);  //done with quad1 vao

	//hand
	const GLfloat handPositionVertices[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	//create a vao for smiling lips
	glGenVertexArrays(1, &gVao_hand);
	glBindVertexArray(gVao_hand);
	//set quad1 position
	glGenBuffers(1, &gVbo_position_hand);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_hand);
	glBufferData(GL_ARRAY_BUFFER, sizeof(handPositionVertices), handPositionVertices, GL_STATIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gVbo_texture_hand);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_texture_hand);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smileyFaceTexcoords), smileyFaceTexcoords, GL_STATIC_DRAW); //providing texture data statically

	glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL); //note 2. We've s and t for texture coords
	glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);  //done with hand vao
	
						   //hand
	const GLfloat smilingTextPositionVertices[] = {
		-2.0f, 0.5f, 0.0f,
		-2.0f, -0.5f, 0.0f,
		2.0f, -0.5f, 0.0f,
		2.0f, 0.5f, 0.0f
	};
	//create a vao for smiling lips
	glGenVertexArrays(1, &gVao_smiling_text);
	glBindVertexArray(gVao_smiling_text);
	//set quad1 position
	glGenBuffers(1, &gVbo_position_smiling_text);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_smiling_text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smilingTextPositionVertices), smilingTextPositionVertices, GL_STATIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gVbo_texture_hand);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_texture_hand);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smileyFaceTexcoords), smileyFaceTexcoords, GL_STATIC_DRAW); //providing texture data statically

	glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL); //note 2. We've s and t for texture coords
	glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);  //done with quad1 vao



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
	//glEnable(GL_CULL_FACE);
	//Texture
	LoadGLTextures(&gTexture_smiley_face, MAKEINTRESOURCE(IDB_BITMAP_SMILEY_FACE));
	LoadGLTextures(&gTexture_smiley_lips, MAKEINTRESOURCE(IDB_BITMAP_SMILEY_LIPS));
	LoadGLTextures(&gTexture_hand, MAKEINTRESOURCE(IDB_BITMAP_HAND));
	LoadGLTextures(&gTexture_keep_smiling_text, MAKEINTRESOURCE(IDB_BITMAP_KEEP_SMILING_TEXT));
	
	glEnable(GL_TEXTURE_2D);
	// set background color to which it will display even if it will empty. THIS LINE CAN BE IN drawRect().
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black

	gPerspectiveProjectionMatrix = mat4::identity();

	resize(800, 600);
}

//FUNCTION DEFINITIONS
void resize(int width, int height) {
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)width / (GLfloat)height),0.1f,100.0f);

}
GLfloat smiley_lips_x;
GLfloat smiley_lips_y;

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const GLfloat smileyFaceVertices[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	const GLfloat smileyLipsVertices[] = {
		-smiley_lips_x, smiley_lips_y, 0.0f,
		-smiley_lips_x, -smiley_lips_y, 0.0f,
		smiley_lips_x, -smiley_lips_y, 0.0f,
		smiley_lips_x, smiley_lips_y, 0.0f
	};
	//Start using shader program object
	glUseProgram(gShaderProgramObject); //run shaders

	//OpenGL drawing

	//set modleview and projection matrices to identity matrix

	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	translationMatrix = vmath::translate(1.0f, 0.0f, -5.0f);
	modelViewMatrix = modelViewMatrix * translationMatrix;
	modelViewMatrix = modelViewMatrix * vmath::scale(1.2f, 1.2f, 1.2f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP

	// Pass above model view matrix projection matrix to vertex shader in 'u_mvp_matrix' shader variable

	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	//bind quad1 vao
	glBindVertexArray(gVao_quad);
	//bind with pyramid texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexture_smiley_face);
	glUniform1i(gTexture_sampler_uniform, 0);
	glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_qaud);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smileyFaceVertices), smileyFaceVertices, GL_DYNAMIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW
	// Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Draw smiley lips
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = vmath::translate(1.0f, -0.4f, -5.0f);
	modelViewMatrix = modelViewMatrix * translationMatrix;
	modelViewMatrix = modelViewMatrix * vmath::scale(0.73f,0.73f,0.73f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	
	glBindVertexArray(gVao_smiley_lips);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexture_smiley_lips);
	glUniform1i(gTexture_sampler_uniform, 0);
	glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_smiley_lips);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smileyLipsVertices), smileyLipsVertices, GL_DYNAMIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions

	glBindVertexArray(0);

	//Draw hand
	if (gEnableHand) {
		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = vmath::translate(-1.0f, -0.3f, -5.0f);
		modelViewMatrix = modelViewMatrix * translationMatrix;
		modelViewMatrix = modelViewMatrix * vmath::scale(0.73f, 0.73f, 0.73f);
		float val = PI / 180;
		modelViewMatrix = modelViewMatrix * vmath::rotate(cos(hand_rotation_angle*val), 0.0f, 0.0f, 1.0f);
		modelViewMatrix = modelViewMatrix * vmath::rotate(sin(hand_rotation_angle*val), 0.0f, 0.0f, 1.0f);
		modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
		glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		glBindVertexArray(gVao_hand);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTexture_hand);
		glUniform1i(gTexture_sampler_uniform, 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions
		glBindVertexArray(0);

		//draw keep smiling text
		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = vmath::translate(0.0f, -1.9f, -6.0f);
		modelViewMatrix = modelViewMatrix * translationMatrix;
				
		modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
		glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		glBindVertexArray(gVao_smiling_text);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTexture_keep_smiling_text);
		glUniform1i(gTexture_sampler_uniform, 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions
		glBindVertexArray(0);

	}

	
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
int LoadGLTextures(GLuint *texture, TCHAR imageResourceId[]) {
	HBITMAP hBitmap;
	BITMAP bmp;
	int iStatus = FALSE;
	memset((void*)&bmp, sizeof(bmp), 0);
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmap) {
		g_log_file << "texture loaded successfully" << std::endl;
		iStatus = TRUE;
		GetObject(hBitmap, sizeof(bmp), &bmp);
		glGenTextures(1, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);//rgba
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0,
			GL_RGB,
			bmp.bmWidth,
			bmp.bmHeight,
			0,
			GL_BGR,
			GL_UNSIGNED_BYTE,
			bmp.bmBits);
		glGenerateMipmap(GL_TEXTURE_2D);
		DeleteObject(hBitmap);
	}
	else {
		g_log_file << "failed to load texture" << std::endl;
	}
	return iStatus;
}
void update() {
	if (smiley_lips_x < 1.0f) {
		smiley_lips_x = smiley_lips_x + 0.0001f;
		if (smiley_lips_x > 0.55f) {
			if (smiley_lips_y < 0.5f) {
				smiley_lips_y = smiley_lips_y + 0.0001f;
			}
		}
	}
	else {
		gEnableHand = true;
	}
	if (gEnableHand == true) {
		if (hand_rotation_angle > 360.0f) {
			hand_rotation_angle = 0.0f;
		}
		else
			hand_rotation_angle += 0.4f;
	}
	
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
	
	if (gVao_quad) {
		glDeleteVertexArrays(1, &gVao_quad);
		gVao_quad = 0;
	}

	if (gVbo_position_qaud) {
		glDeleteBuffers(1, &gVbo_position_qaud);
		gVbo_position_qaud = 0;
	}
	if (gVbo_texture_smiley) {
		glDeleteBuffers(1, &gVbo_texture_smiley);
		gVbo_texture_smiley = 0;
	}

	if (gVbo_texture_smiley_lips) {
		glDeleteBuffers(1, &gVbo_texture_smiley_lips);
		gVbo_texture_smiley_lips = 0;
	}
	if (gVbo_position_smiley_lips) {
		glDeleteBuffers(1, &gVbo_position_smiley_lips);
		gVbo_position_smiley_lips = 0;
	}

	//unload textures
	if (gTexture_smiley_face) {
		glDeleteTextures(1, &gTexture_smiley_face);
		gTexture_smiley_face = 0;
	}
	if (gTexture_smiley_lips) {
		glDeleteTextures(1, &gTexture_smiley_lips);
		gTexture_smiley_lips = 0;
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

