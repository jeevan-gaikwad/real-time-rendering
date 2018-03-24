#include<iostream>
#include <windows.h>
#include <stdio.h>
#include<fstream>
#include <stdlib.h>
#include <gl\glew.h>
#include<gl\GL.h>
#include"vmath.h"
#include "Sphere.h"
#include "Material_prop.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"Sphere.lib")

#define MY_NAME_INITIAL       "JCG"
#define DEFAULT_AXIS_ROTATION 'z'
#define RADIUS				  500.0f
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


GLuint gVao_sphere;
GLuint gVbo_normal_sphere;
GLuint gVbo_position_sphere;
GLuint gVbo_element_sphere;
GLuint gMVPUniform;
GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLightPositionUniform, gLight2PositionUniform;
GLuint gLKeyPressedUniform;
GLuint La_uniform, Ld_uniform, Ls_uniform, L1d_uniform, L1s_uniform, L2d_uniform, L2s_uniform;
GLuint Ka_uniform, Kd_uniform, Ks_uniform;
GLuint material_shininess_uniform;
GLfloat gAngle = 0.0f;
bool gbAnimate;
bool gbLight;
GLfloat angle_pyramid = 0.0f, angle_cobe = 0.0f;

mat4 gPerspectiveProjectionMatrix;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices, gNumElements;

//lighting details
GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,0.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 0.0f };

GLfloat material_ambient[] = { 0.24725f, 0.1995f, 0.0745f, 1.0f };
GLfloat material_diffuse[] = { 0.75164f, 0.60648f, 0.22648f, 1.0f };
GLfloat material_specular[] = { 0.628281f, 0.555802f, 0.366065f, 1.0f };
GLfloat material_shininess = 0.4 * 128;

int winWidth = 800;
int winHeight = 600;

char current_axis_rotation = DEFAULT_AXIS_ROTATION;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	void initialize();
	void uninitialize();
	void spin();
	void display();
	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("PP-24-spheres-with-light");
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

	if (!g_log_file.is_open()) {
		std::cout << "Failed to open log new file" << std::endl;
		uninitialize();
	}
	else
		g_log_file << "Log file successfully created!" << std::endl;
	initialize();

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
			if(gbAnimate == true)
				spin();
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
		"#version 440 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform vec4 u_light_position;" \
		"uniform int u_lighting_enabled;" \
		"out vec3 transformed_normals;" \
		"out vec3 light_direction;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		
		"if(u_lighting_enabled == 1)" \
		"{" \
		"        /*light calculations  */       " \
		"vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals = mat3(u_view_matrix ) * vNormal;" \
		"light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
		"viewer_vector = -eye_coordinates.xyz;" \
		"}" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
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
				g_log_file.flush();
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
		"#version 440 core" \
		"\n" \
		"in vec3 transformed_normals;" \
		"in vec3 light_direction;" \
		"in vec3 viewer_vector;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"uniform int u_lighting_enabled;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"vec3 phong_ads_color;"\
		"if(u_lighting_enabled == 1)" \
		"{" \
		"        /*light calculations  */       " \
		"vec3 normalized_transformed_normals = normalize(transformed_normals);" \
		"vec3 normalized_light_direction = normalize(light_direction);" \
		"vec3 normalized_viewer_vector = normalize(viewer_vector);" \
		"float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction), 0.0);" \
		"vec3 ambient = u_La * u_Ka;" \
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);" \
		"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector),0.0), u_material_shininess);" \
		"phong_ads_color = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = vec4(phong_ads_color, 1.0);" \
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
				g_log_file.flush();
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


	const GLubyte *glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
	g_log_file << "GLSL version is:" << glsl_version << std::endl;
	//----------SHADERS-----------
	initialize_shaders();

	//Create shader program
	gShaderProgramObject = glCreateProgram();

	//attach shaders to the program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	//map our(RAM) memory identifier to GPU memory(VRAM) identifier
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_NORMAL, "vNormal");

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
	// Vertices, colors, shader attribs, vbo, vao initializations
	//Preparation to put our dynamic(uniform) data into the shader
	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled"); //L/l key pressed or not
	
	//material ambient color intensity
	Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	//shininess of material
	material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");

	//light animation
	La_uniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	Ld_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	Ls_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	
	gLightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();
	
	
	glGenVertexArrays(1, &gVao_sphere);
	glBindVertexArray(gVao_sphere);
	//set square position
	glGenBuffers(1, &gVbo_position_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_sphere);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);

	glGenBuffers(1, &gVbo_normal_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_normal_sphere);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); //done with cube vao

						  // element vbo
	glGenBuffers(1, &gVbo_element_sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_element_sphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glShadeModel(GL_SMOOTH);
	// set-up depth buffer
	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	
	// We will always cull back faces for better performance
	//glEnable(GL_CULL_FACE);

	// set background color to which it will display even if it will empty. THIS LINE CAN BE IN drawRect().
	glClearColor(0.25f, 0.25f, 0.25f, 0.0f); // grey

	gPerspectiveProjectionMatrix = mat4::identity();
	gbAnimate = false;
	gbLight = false;
	resize(winWidth, winHeight);
}

//FUNCTION DEFINITIONS
void resize(int width, int height) {
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)width / (GLfloat)height), 0.1f, 100.0f);

}

void display() {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	void set_light_rotation(char axis_of_rotation);
	void set_light_properties();
	void render_sphere();
	void draw_spheres(GLint no_of_spheres, material_props_t material_properties[], int no_of_materials);
	void set_material_properties(material_props_t);
	//Start using shader program object
	glUseProgram(gShaderProgramObject); //run shaders
	
	if (gbLight == true) {
		glUniform1i(gLKeyPressedUniform, 1);//notify shader about lighting

		set_light_properties();		

		set_light_rotation(current_axis_rotation);
	}
	else {
		glUniform1i(gLKeyPressedUniform, 0);
	}

	//Spheres drawing
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	draw_spheres(24, materials, no_of_materials); //Refer Material_prop.h for materials array details
		
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
	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;
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
	case WM_CHAR:
		switch (LOWORD(wParam)) {
		case 'x':
			current_axis_rotation = 'x';
			break;
		case 'y':
			current_axis_rotation = 'y';
			break;
		case 'z':
			current_axis_rotation = 'z';
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
				if (dwStyle & WS_OVERLAPPEDWINDOW) {
					wpPrev.length = sizeof(WINDOWPLACEMENT);
					isWp = GetWindowPlacement(hwnd, &wpPrev);
					hMonitor = MonitorFromWindow(hwnd, MONITORINFOF_PRIMARY);
					monitorInfo.cbSize = sizeof(MONITORINFO);
					isMonitorInfo = GetMonitorInfo(hMonitor, &monitorInfo);
					if (isWp == TRUE && isMonitorInfo) {
						winWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
						winHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
						SetWindowLong(hwnd, GWL_STYLE, dwStyle&~WS_OVERLAPPEDWINDOW);
						SetWindowPos(hwnd, HWND_TOP,
							monitorInfo.rcMonitor.left,
							monitorInfo.rcMonitor.top,
							winWidth,
							winHeight,
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
				winWidth = 800;
				winHeight = 600;
				gbIsFullScreen = false;
			}
			break;
		case 0x41:
			if (bIsAKeyPressed == false) {
				gbAnimate = true;
				bIsAKeyPressed = true;
			}
			else {
				gbAnimate = false;
				bIsAKeyPressed = false;
			}
			break;
		case 0x4c:
			if (bIsLKeyPressed == false) {
				gbLight = true;
				bIsLKeyPressed = true;
			}
			else {
				gbLight = false;
				bIsLKeyPressed = false;
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
void spin() {
	gAngle += 0.005f;
	if (gAngle >= 360.0f) {
		gAngle = 0;
	}
}

void draw_spheres(GLint no_of_spheres, material_props_t material_properties[], int no_of_materials) {
	resize(winWidth, winHeight);
	void render_sphere();
	void set_material_properties(material_props_t);

	//winWidth = 800;
	//winHeight = 600;
	GLint windowCenterX = winWidth / 2;
	GLint windowCenterY = winHeight / 2;
	GLint viewPortSizeX = winWidth / 8, viewPortSizeY = winHeight/8;
	glViewport(windowCenterX, windowCenterY, viewPortSizeX, viewPortSizeY);

	int y_trans = viewPortSizeX;
	int x_trans = viewPortSizeY;
	GLint   distanceBetSpheres = 130;
	int currentViewPortX = windowCenterX - x_trans*3;
	int currentViewPortY = windowCenterY + y_trans*1.8;
	glViewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);

	int i = 0, j = 0;
	for (i = 1, j = 0;i <= no_of_spheres;i++, j++) {

		int local_y_trans = 0;
		if (((i - 1) % 4) == 0 && (i - 1 != 0)) {
			//local_y_trans = distanceBetSpheres;
			//y_trans -= local_y_trans;
			currentViewPortX = windowCenterX - x_trans*3;
			currentViewPortY -= y_trans;
			glViewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
		}
		currentViewPortX += x_trans;
		glViewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
		//set material properties
		if (j <no_of_materials)
			set_material_properties(material_properties[j]); 

		//draws sphere at current viewPort position
		render_sphere();
	}
	
}

void render_sphere() { //draws sphere at current model view matrix position

	//bind cube vao
	glBindVertexArray(gVao_sphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_element_sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);
}

void set_material_properties(material_props_t material_properties) {

	//set material properties
	glUniform3fv(Ka_uniform, 1, material_properties.ambient);
	glUniform3fv(Kd_uniform, 1, material_properties.diffuse);
	glUniform3fv(Ks_uniform, 1, material_properties.specular);
	glUniform1f(material_shininess_uniform, material_properties.shininess);

}
void set_light_properties() {
	//setting light's properties
	glUniform3fv(La_uniform, 1, lightAmbient);
	glUniform3fv(Ld_uniform, 1, lightDiffuse);
	glUniform3fv(Ls_uniform, 1, lightSpecular);

	glUniform4fv(gLightPositionUniform, 1, lightPosition);
}

void set_light_rotation(char axis_of_rotation) {

	//light animation
	mat4 modelLightMatrix = mat4::identity();


	mat4 rotationLightMatrix = mat4::identity();

	modelLightMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
	GLfloat lightPosition_local[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	if (axis_of_rotation == 'x') {
		lightPosition_local[1] = RADIUS * cos(gAngle);
		lightPosition_local[2] = RADIUS * sin(gAngle);
	}
	else if (axis_of_rotation == 'y') {
		lightPosition_local[0] = RADIUS * cos(gAngle);
		lightPosition_local[2] = RADIUS * sin(gAngle);
	}
	else if (axis_of_rotation == 'z') {
		lightPosition_local[0] = RADIUS * cos(gAngle);
		lightPosition_local[1] = RADIUS * sin(gAngle);
	}
	//set the light position
	
	glUniform4fv(gLightPositionUniform, 1, lightPosition_local);
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

	
	if (gVao_sphere) {
		glDeleteVertexArrays(1, &gVao_sphere);
		gVao_sphere = 0;
	}

	
	if (gVbo_normal_sphere) {
		glDeleteBuffers(1, &gVbo_normal_sphere);
		gVbo_normal_sphere = 0;
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

