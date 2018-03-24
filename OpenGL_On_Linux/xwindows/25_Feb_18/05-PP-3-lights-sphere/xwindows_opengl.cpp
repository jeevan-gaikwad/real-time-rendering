#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<fstream>
#include"vmath.h"
#include"Sphere.h"

using namespace std;
using namespace vmath;

bool gbFullScreen=false;
const float RADIUS = 500.0f;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;
GLXContext gGLXContext;
ofstream g_log_file;

enum{
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
GLuint gViewLightMatrixUniform;

GLuint gLight0PositionUniform,  gLight1PositionUniform, gLight2PositionUniform;
GLuint gLKeyPressedUniform;
GLuint La_uniform, L0d_uniform, L0s_uniform, L1d_uniform, L1s_uniform, L2d_uniform, L2s_uniform;
GLuint Ka_uniform, Kd_uniform, Ks_uniform;
GLuint material_shininess_uniform;
GLfloat gAngle = 0.0f;
bool gbAnimate;
bool gbLight;


mat4 gPerspectiveProjectionMatrix;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumVertices, gNumElements;

//lighting details
GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f };

//Red color light(light0)
GLfloat light0Diffuse[] = { 1.0f,0.0f,0.0f,0.0f };
GLfloat light0Specular[] = { 1.0f, 0.0f, 0.0f, 0.0f };

//Green color light(light1)
GLfloat light1Diffuse[] = { 0.0f,1.0f,0.0f,0.0f };
GLfloat light1Specular[] = { 0.0f, 1.0f, 0.0f, 0.0f };

//Blue color light(light2)
GLfloat light2Diffuse[] = { 0.0f,0.0f,1.0f,0.0f };
GLfloat light2Specular[] = { 0.0f, 0.0f, 1.0f, 0.0f };


GLfloat material_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat material_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat material_shininess = 50.0f;
void resize(int,int);
void spin();
int main(void){
	void CreateWindow(void);
	void ToggleFullscreen();
	void uninitialize();
	void initialize();
	void display();
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	char ascii_val[32];
	CreateWindow();
	bool bDone=false;
	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;

	XEvent event;
	KeySym keysym;
	g_log_file.open("log.txt", ios::out);
	if(!g_log_file.is_open()){
		cout<<"Failed to create log file"<<endl;
	}else{
	    g_log_file<<"Log file successfully created"<<endl;
	}

	initialize();
	while(bDone==false){
	
	  while(XPending(gpDisplay)){
		XNextEvent(gpDisplay,&event);

		switch(event.type){
			case MapNotify:
				break;
			case KeyPress:
				//keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				XLookupString(&event.xkey,ascii_val,sizeof(ascii_val),&keysym,NULL);
				switch(keysym){
					case XK_Escape:
							bDone=true;
							break;
					case XK_a:
					case XK_A:
						if(bIsAKeyPressed == false){
							gbAnimate = true;
							bIsAKeyPressed =  true;
						}
						else{
							gbAnimate = false;
							bIsAKeyPressed =  false;
						}
						break;	
					case XK_L:
					case XK_l:
						if(bIsLKeyPressed == false){
							gbLight = true;
							bIsLKeyPressed = true;
						}else
						{
							gbLight = false;
							bIsLKeyPressed = false;
						}
					break;
					case XK_F:
					case XK_f:
						if(gbFullScreen==false){
								ToggleFullscreen();
								gbFullScreen=true;							
						}else{
								ToggleFullscreen();
								gbFullScreen=false;
						}
						break;
				default:
						break;

				}
				break;
			case ButtonPress:
				switch(event.xbutton.button){
					case 1:// like WM_LBUTTONDOWN
						break;
					case 2:// like WM_MBUTTONDOWN
						break;
					default:
						break;

				}
			case MotionNotify:
					break;
			case ConfigureNotify:
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
			case Expose:
					break;
			case DestroyNotify:
					break;
			case 33: //close button event
					bDone=true;
			default:
				break;
		}
      }
		display();	
		if(gbAnimate)
			spin();
	}
	if(g_log_file.is_open()){
		g_log_file.close();
	}
	return(0);
}

void CreateWindow(){
	void uninitialize();
	//variable declarations
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	static int frameBufferAttributes[]={
		GLX_DEPTH_SIZE, 24,
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DOUBLEBUFFER, 1,
		GLX_ALPHA_SIZE, 8,
		None

	};

	gpDisplay=XOpenDisplay(NULL);

	if(gpDisplay==NULL){
		
		printf("Error: Unable to Open X Display.\n Exiting now....\n");
		uninitialize();
		exit(1);

	}

	defaultScreen=XDefaultScreen(gpDisplay);
	
	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
									RootWindow(gpDisplay,gpXVisualInfo->screen),
									gpXVisualInfo->visual,
									AllocNone);
	gColormap=winAttribs.colormap;

	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;

	gWindow=XCreateWindow(gpDisplay,
					RootWindow(gpDisplay, gpXVisualInfo->screen),
					0,
					0,
					giWindowWidth,
					giWindowHeight,
					0,
					gpXVisualInfo->depth,
					InputOutput,
					gpXVisualInfo->visual,
					styleMask,
					&winAttribs);

					if(!gWindow)
					{
						printf("Error: Failed to Create Main Window.\n Exiting now..\n");
						uninitialize();
						exit(1);
					}
	XStoreName(gpDisplay,gWindow,"PP-3-moving-lights");
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
}
void initialize(){
	
	void uninitialize();
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	GLenum glew_error = glewInit();	//Turn ON all graphic card extension

	if (glew_error != GLEW_OK) {
		uninitialize();
	}
	//----VERTEX SHADER-----
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//write source code of vertex shader
	const GLchar* vertexShaderSourceCode = 
		"#version 440 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform vec4 u_light0_position;" \
		"uniform vec4 u_light1_position;" \
		"uniform vec4 u_light2_position;" \
		"uniform int u_lighting_enabled;" \
		"out vec3 transformed_normals_light0;" \
		"out vec3 light_direction_light0;" \
		"out vec3 viewer_vector_light0;" \
		"out vec3 transformed_normals_light1;" \
		"out vec3 light_direction_light1;" \
		"out vec3 viewer_vector_light1;" \
		"out vec3 transformed_normals_light2;" \
		"out vec3 light_direction_light2;" \
		"out vec3 viewer_vector_light2;" \
		"void main(void)" \
		"{" \
		"if(u_lighting_enabled == 1)" \
		"{" \
		"        /*light0 calculations  */       " \
		"vec4 eye_coordinates_light0 = u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals_light0 = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction_light0 = vec3(u_light0_position) - eye_coordinates_light0.xyz;" \
		"viewer_vector_light0 = -eye_coordinates_light0.xyz;" \
		"        /*light1 calculations  */       " \
		"vec4 eye_coordinates_light1 = u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals_light1 = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction_light1 = vec3(u_light1_position) - eye_coordinates_light1.xyz;" \
		"viewer_vector_light1 = -eye_coordinates_light1.xyz;" \
		"        /*light2 calculations  */       " \
		"vec4 eye_coordinates_light2 = u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals_light2 = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction_light2 = vec3(u_light2_position) - eye_coordinates_light2.xyz;" \
		"viewer_vector_light2 = -eye_coordinates_light2.xyz;" \
		"}" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"}";


	glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
	//compile shader
	glCompileShader(gVertexShaderObject);
	//Shader compilation error checks goes here
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if(iShaderCompiledStatus == GL_FALSE){
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if(iInfoLogLength > 0){
			szInfoLog = (char*) malloc(iInfoLogLength);
			if(szInfoLog != NULL){
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				g_log_file << "Vertex shade compilation log:"<<szInfoLog <<std::endl;
				free(szInfoLog);
				uninitialize();
				exit(EXIT_FAILURE);
			}
		}
	}
	//-----FRAGMENT SHADER-----
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//source code of fragment shader
	const GLchar* fragmentShaderSourceCode = 
		"#version 440 core" \
		"\n" \
		"in vec3 transformed_normals_light0;" \
		"in vec3 light_direction_light0;" \
		"in vec3 viewer_vector_light0;" \
		"in vec3 transformed_normals_light1;" \
		"in vec3 light_direction_light1;" \
		"in vec3 viewer_vector_light1;" \
		"in vec3 transformed_normals_light2;" \
		"in vec3 light_direction_light2;" \
		"in vec3 viewer_vector_light2;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_L0d;" \
		"uniform vec3 u_L0s;" \
		"uniform vec3 u_L1d;" \
		"uniform vec3 u_L1s;" \
		"uniform vec3 u_L2d;" \
		"uniform vec3 u_L2s;" \
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
		"        /*light0 calculations  */       " \
		"vec3 normalized_transformed_normals_light0 = normalize(transformed_normals_light0);" \
		"vec3 normalized_light_direction_light0 = normalize(light_direction_light0);" \
		"vec3 normalized_viewer_vector_light0 = normalize(viewer_vector_light0);" \
		"float tn_dot_ld_light0 = max(dot(normalized_transformed_normals_light0, normalized_light_direction_light0), 0.0);" \
		"vec3 ambient = u_La * u_Ka;" \
		"vec3 diffuse_light0 = u_L0d * u_Kd * tn_dot_ld_light0;" \
		"vec3 reflection_vector_light0 = reflect(-normalized_light_direction_light0, normalized_transformed_normals_light0);" \
		"vec3 specular_light0 = u_L0s * u_Ks * pow(max(dot(reflection_vector_light0, normalized_viewer_vector_light0),0.0), u_material_shininess);" \
		"        /*light1 calculations  */       " \
		"vec3 normalized_transformed_normals_light1 = normalize(transformed_normals_light1);" \
		"vec3 normalized_light_direction_light1 = normalize(light_direction_light1);" \
		"vec3 normalized_viewer_vector_light1 = normalize(viewer_vector_light1);" \
		"float tn_dot_ld_light1 = max(dot(normalized_transformed_normals_light1, normalized_light_direction_light1), 0.0);" \
		"vec3 diffuse_light1 = u_L1d * u_Kd * tn_dot_ld_light1;" \
		"vec3 reflection_vector_light1 = reflect(-normalized_light_direction_light1, normalized_transformed_normals_light1);" \
		"vec3 specular_light1 = u_L1s * u_Ks * pow(max(dot(reflection_vector_light1, normalized_viewer_vector_light1),0.0), u_material_shininess);" \
		"        /*light2 calculations  */       " \
		"vec3 normalized_transformed_normals_light2 = normalize(transformed_normals_light2);" \
		"vec3 normalized_light_direction_light2 = normalize(light_direction_light2);" \
		"vec3 normalized_viewer_vector_light2 = normalize(viewer_vector_light2);" \
		"float tn_dot_ld_light2 = max(dot(normalized_transformed_normals_light2, normalized_light_direction_light2), 0.0);" \
		"vec3 diffuse_light2 = u_L2d * u_Kd * tn_dot_ld_light2;" \
		"vec3 reflection_vector_light2 = reflect(-normalized_light_direction_light2, normalized_transformed_normals_light2);" \
		"vec3 specular_light2 = u_L2s * u_Ks * pow(max(dot(reflection_vector_light2, normalized_viewer_vector_light2),0.0), u_material_shininess);" \
		"   /* Sum all the lights calculation to form final ads color  */"
		"phong_ads_color = ambient*3 + diffuse_light0 + diffuse_light1 + diffuse_light2 + specular_light0 + specular_light1+ specular_light2;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = vec4(phong_ads_color, 1.0);" \
		"}";


		glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//compile shader
	glCompileShader(gFragmentShaderObject);
	//compilation error checks goes here
	iInfoLogLength = 0;
	iShaderCompiledStatus = 0;
	szInfoLog = NULL;
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if(iShaderCompiledStatus == GL_FALSE){
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if(iInfoLogLength > 0){
			szInfoLog = (char*) malloc(iInfoLogLength);
			if(szInfoLog != NULL){
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				g_log_file << "Fragment shader compilation log:"<<szInfoLog <<std::endl;
				free(szInfoLog);
				uninitialize();
				exit(EXIT_FAILURE);
			}
		}
	}

	//Now create shader program for above shaders
	gShaderProgramObject =  glCreateProgram();

	//attach above shaders to this program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);
	
	//pre binding of static data before linking
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_NORMAL, "vNormal");
	//Link shader program
	glLinkProgram(gShaderProgramObject); //combine all compiled shaders
	iInfoLogLength = 0;
	GLint iShaderProgramLinkStatus = 0;
	szInfoLog = NULL;
	glGetShaderiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if(iShaderProgramLinkStatus == GL_FALSE){
		glGetShaderiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);

		if(iInfoLogLength > 0){
			szInfoLog = (char*) malloc(iInfoLogLength);
			if(szInfoLog != NULL){
				GLsizei written;
				glGetShaderInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				g_log_file << "Shader program linking log:"<<szInfoLog <<std::endl;
				free(szInfoLog);
				uninitialize();
				exit(EXIT_FAILURE);
			}
		}
	}
	//Preparation to put our dynamic(uniform) data into the shader
	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled"); //L/1 key pressed or not
	La_uniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	
	//material ambient color intensity
	Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	//shininess of material
	material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");

	//light animation
	
	
	L0d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0d");
	L0s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0s");

	L1d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1d");
	L1s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1s");
	//ligh2 details

	L2d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L2d");
	L2s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L2s");


	gLight0PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light0_position");
	gLight1PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light1_position");
	gLight2PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light2_position");
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);


	// Vertices, colors, shader attribs, vbo, vao initializations
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

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,0.0f);//black background
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	gPerspectiveProjectionMatrix = mat4::identity();
	resize(giWindowWidth,giWindowHeight);
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	void spin();
	//Run shader program
	glUseProgram(gShaderProgramObject);

	if (gbLight == true) {
		glUniform1i(gLKeyPressedUniform, 1);
		//setting light's properties
		glUniform3fv(La_uniform, 1, lightAmbient);
		glUniform3fv(L0d_uniform, 1, light0Diffuse);
		glUniform3fv(L0s_uniform, 1, light0Specular);
		//light1
		glUniform3fv(L1d_uniform, 1, light1Diffuse);
		glUniform3fv(L1s_uniform, 1, light1Specular);
		//light2
		glUniform3fv(L2d_uniform, 1, light2Diffuse);
		glUniform3fv(L2s_uniform, 1, light2Specular);
	
		//light0
		lightPosition[1] = RADIUS * (float)cos(gAngle);
		lightPosition[2] = RADIUS * (float)sin(gAngle);
		glUniform4fv(gLight0PositionUniform, 1, lightPosition);
		lightPosition[1] = 0.0f;
		lightPosition[2] = 0.0f;

		//light1
		lightPosition[0] = RADIUS * (float)cos(gAngle);
		lightPosition[2] = RADIUS * (float)sin(gAngle);
		glUniform4fv(gLight1PositionUniform, 1, lightPosition);
		glUniform4fv(gLight2PositionUniform, 1, lightPosition);
		lightPosition[0] = 0.0f;
		lightPosition[2] = 0.0f;

		//light2
		lightPosition[0] = RADIUS * (float)cos(gAngle);
		lightPosition[1] = RADIUS * (float)sin(gAngle);
		glUniform4fv(gLight2PositionUniform, 1, lightPosition);
		lightPosition[0] = 0.0f;
		lightPosition[1] = 0.0f;
		

		//set material properties
		glUniform3fv(Ka_uniform, 1, material_ambient);
		glUniform3fv(Kd_uniform, 1, material_diffuse);
		glUniform3fv(Ks_uniform, 1, material_specular);
		glUniform1f(material_shininess_uniform, material_shininess);

	}
	else {
		glUniform1i(gLKeyPressedUniform, 0);
	}



	//OpenGL drawing

	//set modleview and projection matrices to identity matrix

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f); 
	
	
   // Pass above matrices to shaders

	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	//bind cube vao
	glBindVertexArray(gVao_sphere);


	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_element_sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);



	// *** unbind vao ***
	glBindVertexArray(0);


	//Stop using shader program
	glUseProgram(0);

	glXSwapBuffers(gpDisplay,gWindow);
}

void spin() {
	gAngle += 0.0051f;
	if (gAngle >= 360.0f) {
		gAngle = gAngle - 360.0f;
	}
}


void resize(int width, int height)
{
	if(height==0)
		height=1;
	glViewport(0.0f,0.0f,(GLsizei)width, (GLsizei)height);

	gPerspectiveProjectionMatrix = perspective(45.0f, ((GLfloat)width/(GLfloat)height), 0.1f, 100.0f);	

}
void ToggleFullscreen()
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));

	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=gbFullScreen? 0:1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(gpDisplay,
		RootWindow(gpDisplay,gpXVisualInfo->screen),
		False,
		StructureNotifyMask,
		&xev);

}

void uninitialize(){
	
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();
	//vao, vbo cleanup

	if (gVao_sphere) {
		glDeleteVertexArrays(1, &gVao_sphere);
		gVao_sphere = 0;
	}

	
	if (gVbo_normal_sphere) {
		glDeleteBuffers(1, &gVbo_normal_sphere);
		gVbo_normal_sphere = 0;
	}

	//Shader cleanup
	//Detach shaders from shader program
	glDetachShader(gShaderProgramObject, gVertexShaderObject); //vertex shader
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	//Now delete shader objects
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;

	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	//Delete program object;

	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	if(currentGLXContext !=NULL && currentGLXContext == gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}

	if(gGLXContext){
		glXDestroyContext(gpDisplay,gGLXContext);
	}

	if(gWindow){
		XDestroyWindow(gpDisplay,gWindow);

	}
	if(gColormap){
		XFreeColormap(gpDisplay,gColormap);
	}
	
	if(gpXVisualInfo){
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	if(gpDisplay){
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}

}














