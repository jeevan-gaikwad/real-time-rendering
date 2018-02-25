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
#include<SOIL/SOIL.h> //for image loading
#include"vmath.h"

using namespace std;
using namespace vmath;

bool gbFullScreen=false;

Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;
GLXContext gGLXContext;
ofstream g_log_file;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;
GLXFBConfig gGLXFBConfig;

enum{
	JCG_ATTRIBUTE_VERTEX = 0,
	JCG_ATTRIBUTE_COLOR,
	JCG_ATTRIBUTE_NORMAL,
	JCG_ATTRIBUTE_TEXTURE0

};

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_pyramid;
GLuint gVao_cube;
GLuint gVbo_position;
GLuint gVbo_texture;
GLuint gMVPUniform;

mat4 gPerspectiveProjectionMatrix;

GLfloat angle_pyramid;
GLfloat angle_cube;
//Texture
GLuint gTexture_sampler_uniform; //for uniform(dynamic) texture data
GLuint gTexture_Kundali;
GLuint gTexture_Stone;

const char *texture_kundali_path="./Kundali.bmp";
const char *texture_stone_path="./Stone.bmp";

int LoadGLTextures(GLuint*, const char * image_path);


void resize(int,int);
void update();
#define ENTRY(x) g_log_file<<"Entry:"<<x<<std::endl<<std::flush;
#define EXIT(x) g_log_file<<"Exit:"<<x<<std::endl<<std::flush; 
#define LOG(x) g_log_file<<x<<std::endl<<std::flush;
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

	XEvent event;
	KeySym keysym;
	g_log_file.open("log.txt", ios::out);
	if(!g_log_file.is_open()){
		cout<<"Failed to create log file"<<endl;
	}else{
	    g_log_file<<"Log file successfully created"<<endl;
	}
	ENTRY("main");
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
		update();
	}
	EXIT("main");
	if(g_log_file.is_open()){
		g_log_file.close();
	}
	return(0);
}

void CreateWindow(void)
{
	ENTRY("CreateWindow");
	//prototype
	void uninitialize(void);
	
	//variables
	XSetWindowAttributes winAttribs;
	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;
	int iNumFBConfigs=0;
	int styleMask;
	int i;
	
	static int frameBufferAttributes[]={
		GLX_X_RENDERABLE,True,
		GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		GLX_STENCIL_SIZE,8,
		GLX_DOUBLEBUFFER,True,
		None}; // array must be terminated by 'None' -> #define None 0
	
	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR : Unable To Obtain X Display.\n");
		uninitialize();
		exit(1);
	}
	
	// get a new framebuffer config that meets our attrib requirements
	pGLXFBConfigs=glXChooseFBConfig(gpDisplay,DefaultScreen(gpDisplay),frameBufferAttributes,&iNumFBConfigs);
	if(pGLXFBConfigs==NULL)
	{
		printf( "Failed To Get Valid Framebuffer Config. Exitting Now ...\n");
		uninitialize();
		exit(1);
	}
	printf("%d Matching FB Configs Found.\n",iNumFBConfigs);
	
	// pick that FB config/visual with the most samples per pixel
	int bestFramebufferconfig=-1,worstFramebufferConfig=-1,bestNumberOfSamples=-1,worstNumberOfSamples=999;
	for(i=0;i<iNumFBConfigs;i++)
	{
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);
		if(pTempXVisualInfo)
		{
			int sampleBuffer,samples;
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS,&sampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);
			printf("Matching Framebuffer Config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);
			if(bestFramebufferconfig < 0 || sampleBuffer && samples > bestNumberOfSamples)
			{
				bestFramebufferconfig=i;
				bestNumberOfSamples=samples;
			}
			if( worstFramebufferConfig < 0 || !sampleBuffer || samples < worstNumberOfSamples)
			{
				worstFramebufferConfig=i;
			    worstNumberOfSamples=samples;
			}
		}
		XFree(pTempXVisualInfo);
	}
	bestGLXFBConfig = pGLXFBConfigs[bestFramebufferconfig];
	// set global GLXFBConfig
	gGLXFBConfig=bestGLXFBConfig;
	
	// be sure to free FBConfig list allocated by glXChooseFBConfig()
	XFree(pGLXFBConfigs);
	
	gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);
	printf("Chosen Visual ID=0x%lu\n",gpXVisualInfo->visualid );
	
	//setting window's attributes
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
										RootWindow(gpDisplay,gpXVisualInfo->screen), //you can give defaultScreen as well
										gpXVisualInfo->visual,
										AllocNone); //for 'movable' memory allocation
										
	winAttribs.event_mask=StructureNotifyMask | KeyPressMask | ButtonPressMask |
						  ExposureMask | VisibilityChangeMask | PointerMotionMask;
	
	styleMask=CWBorderPixel | CWEventMask | CWColormap;
	gColormap=winAttribs.colormap;										           
	
	gWindow=XCreateWindow(gpDisplay,
						  RootWindow(gpDisplay,gpXVisualInfo->screen),
						  0,
						  0,
						  800,
						  600,
						  0, //border width
						  gpXVisualInfo->depth, //depth of visual (depth for Colormap)          
						  InputOutput, //class(type) of your window
						  gpXVisualInfo->visual,
						  styleMask,
						  &winAttribs);
	if(!gWindow)
	{
		printf("Failure In Window Creation.\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"OpenGL Window");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_WINDOW_DELETE",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);

	EXIT("CreateWindow");
}


void initialize(){
	void uninitialize();
	
	ENTRY("initialize");
	// create a new GL context 4.5 for rendering
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	
	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,4, 
		GLX_CONTEXT_MINOR_VERSION_ARB,5, 
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 
		0 }; // array must be terminated by 0
	g_log_file<<"createing context attrib"<<std::endl<<std::flush;	
	gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	g_log_file<<" context attrib created"<<std::endl<<std::flush;	

	if(!gGLXContext) // fallback to safe old style 2.x context
	{
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0 }; // array must be terminated by 0
		printf("Failed To Create GLX 4.5 context. Hence Using Old-Style GLX Context\n");
		gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	}
	else // successfully created 4.1 context
	{
		printf("OpenGL Context 4.5 Is Created.\n");
	}
	
	// verifying that context is a direct context
	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("Indirect GLX Rendering Context Obtained\n");
	}
	else
	{
		printf("Direct GLX Rendering Context Obtained\n" );
	}
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	



	GLenum glew_error = glewInit();	//Turn ON all graphic card extension
	g_log_file<<"glew init done"<<std::flush;
	if (glew_error != GLEW_OK) {
		uninitialize();
	}
	//----VERTEX SHADER-----
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//write source code of vertex shader
	const GLchar* vertexShaderSourceCode = 
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
	g_log_file<<"Vertex shader compilation done"<<std::flush;
	//-----FRAGMENT SHADER-----
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//source code of fragment shader
	const GLchar* fragmentShaderSourceCode = 
		"#version 450 core" \
		"\n" \
		"in vec2 out_texture0_coord;" \
		"out vec4 FragColor;" \
		"uniform sampler2D u_texture0_sampler;" \
		"void main(void)" \
		"{" \
		"FragColor = texture(u_texture0_sampler, out_texture0_coord);" \
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

	g_log_file<<"Fragment shader compilation done"<<std::flush;
	//Now create shader program for above shaders
	gShaderProgramObject =  glCreateProgram();

	//attach above shaders to this program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);
	
	//pre binding of static data before linking
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");
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
	LOG("Shader program linking done");
	//get GPU memory location for our dynamic(uniform) data
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	gTexture_sampler_uniform = glGetUniformLocation(gShaderProgramObject, "u_texture0_sampler");
	//multicolored triangle
	const GLfloat pyramidVertices [] = {
	 0.0f, 1.0f, 0.0f, //apex of the triangle
	 -1.0f, -1.0f, 1.0f, //left-bottom
	 1.0f, -1.0f, 1.0f, //right-bottom
	 //right face
	 0.0f, 1.0f, 0.0f, //apex
	 1.0f, -1.0f, 1.0f,//left bottom
	 1.0f, -1.0f, -1.0f, //right bottom
	 //back face
	 0.0f, 1.0f, 0.0f, //apex
	 1.0f, -1.0f, -1.0f,
	 -1.0f, -1.0f, -1.0f,
	 //left face
	 0.0f, 1.0f, 0.0f, //apex
	 -1.0f, -1.0f, -1.0f, //left bottom
	 -1.0f, -1.0f, 1.0f //right bottom
	};
	
	const GLfloat pyramidTexCoords [] = {
		0.5f, 1.0f,
		0.0,  0.0,
		1.0,  0.0,

		0.5f, 1.0f,
		1.0f, 0.0f,
		0.0f,  0.0f,

		0.5f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.5f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f

	};
	//vao for pyramid
	glGenVertexArrays(1,&gVao_pyramid);
	glBindVertexArray(gVao_pyramid);
	
	glGenBuffers(1, &gVbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);

	glGenBuffers(1, &gVbo_texture);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_texture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidTexCoords), pyramidTexCoords, GL_STATIC_DRAW);
	
	glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);

        //Cube
	const GLfloat cubeVertices [] = {
		//front face
		-1.0f, 1.0f, 1.0f, //left top
		-1.0f, -1.0f, 1.0f,  //left bottom
		1.0f, -1.0f, 1.0f,  //right bottom
		1.0f, 1.0f, 1.0f, //right top
		//right face
		1.0f, 1.0f, 1.0f,//left top
		1.0f, -1.0f, 1.0f, //left bottom
		1.0f, -1.0f, -1.0f, //right bottom
		1.0f, 1.0f, -1.0f,//right top

		//back face
		1.0f, 1.0f, -1.0f,//left top
		1.0f, -1.0f, -1.0f,//left bottom
		-1.0f, -1.0f, -1.0f, //right bottom
		-1.0f, 1.0f, -1.0f, //right top

		//left face
		- 1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		//top face
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,

		//bottom face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f
	};
		//cube texture coords
	const GLfloat cubeTexcoords[] = {
		
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,


		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f

	};


		//vao for cube
	glGenVertexArrays(1,&gVao_cube);
	glBindVertexArray(gVao_cube);
	
	glGenBuffers(1, &gVbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	
	glGenBuffers(1, &gVbo_texture);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_texture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexcoords), cubeTexcoords, GL_STATIC_DRAW);
	
	glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	LOG("Done with vaos and vbos");
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,0.0f);//black background
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//load textures
	LoadGLTextures(&gTexture_Kundali, texture_kundali_path);
	LoadGLTextures(&gTexture_Stone, texture_stone_path);
	gPerspectiveProjectionMatrix = mat4::identity();
	resize(giWindowWidth,giWindowHeight);
	EXIT("initialize");
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	//Run shader program
	glUseProgram(gShaderProgramObject);
	
	//OpenGL drawing
	//triangle
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();
	modelViewMatrix = modelViewMatrix * translate(-2.0f, 0.0f, -6.0f);
	modelViewMatrix *= vmath::rotate(angle_pyramid, 0.0f, 1.0f, 0.0f);

	modelViewProjectionMatrix =  gPerspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(gVao_pyramid);
	
	//bind with pyramid texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexture_Stone);
	glUniform1i(gTexture_sampler_uniform, 0);

	glDrawArrays(GL_TRIANGLES, 0, 12);

	//square
	 modelViewMatrix = mat4::identity();
	 modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = modelViewMatrix * translate(2.0f, 0.0f, -6.0f);
	modelViewMatrix *= scale(0.75f, 0.75f, 0.75f);
	modelViewMatrix *= vmath::rotate(angle_cube, 1.0f, 0.0f, 0.0f);
	modelViewMatrix *= vmath::rotate(angle_cube, 0.0f, 1.0f, 0.0f);
	modelViewMatrix *= vmath::rotate(angle_cube, 0.0f, 0.0f, 1.0f);

	modelViewProjectionMatrix =  gPerspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(gVao_cube);
	//bind with cube texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexture_Kundali);
	glUniform1i(gTexture_sampler_uniform, 0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glBindVertexArray(0);

	//Stop using shader program
	glUseProgram(0);

	glXSwapBuffers(gpDisplay,gWindow);
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

	if(gVao_pyramid){
		glDeleteVertexArrays(1, &gVao_pyramid);
		gVao_pyramid=0;
	}
	
	if(gVao_cube){
		glDeleteVertexArrays(1, &gVao_cube);
		gVao_cube=0;
	}

	if(gVbo_position){
		glDeleteBuffers(1, &gVbo_position);
		gVbo_position = 0;
	}
	
	if(gVbo_texture){
		glDeleteBuffers(1, &gVbo_texture);
		gVbo_texture = 0;
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

void update(){
	if(angle_pyramid > 360.0){
		angle_pyramid = angle_cube = 0.0f;
	}else{
		angle_pyramid = angle_pyramid + 0.1f;
		angle_cube = angle_cube + 0.1f;
	}

}


int LoadGLTextures(GLuint *texture,const char * image_path){
	unsigned char * image_data = NULL;
	int width, height;
	//GLuint texture;
	image_data = SOIL_load_image(image_path, &width, &height, 0, SOIL_LOAD_AUTO);
	if(image_data == NULL ){
		fprintf(stderr, "Failed to open %s image \n",image_path);
		exit(1);
	}
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);//rgba
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, 
			GL_RGB,
			width,
			height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			(void*)image_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void*)image_data);
		SOIL_free_image_data(image_data);
	return 1;
}













