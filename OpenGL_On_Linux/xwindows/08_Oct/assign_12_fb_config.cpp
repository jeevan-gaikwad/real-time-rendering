#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
using namespace std;

bool gbFullScreen=false;

Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;
FILE *fp;
GLXContext gGLXContext;
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;
GLXFBConfig gGLXFBConfig;

void resize(int,int);
void update();
GLfloat angleTri,angleRect=360.0;
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
	initialize();
	fp=fopen("log.txt","w");
	fprintf(fp,"Writting logs\n");
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
	}
	fclose(fp);
	return(0);
}

void CreateWindow(){
	void uninitialize();
	//variable declarations
	XSetWindowAttributes winAttribs;
	
	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;

	int styleMask;
	int iNumFBConfigs = 0;
	int i;

	static int frameBufferAttributes[]={
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_STENCIL_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
		GLX_ALPHA_SIZE, 8,
		None
		//,GLX_SAMPLE_BUFFERS,1,     //Enable these two for programmable pipeline
		//GLX_SAMPLES, 4
	};

	gpDisplay=XOpenDisplay(NULL);

	if(gpDisplay==NULL){
		
		printf("Error: Unable to Open X Display.\n Exiting now....\n");
		uninitialize();
		exit(1);

	}
	
	pGLXFBConfigs=glXChooseFBConfig(gpDisplay, DefaultScreen(gpDisplay),frameBufferAttributes, &iNumFBConfigs);
	if(pGLXFBConfigs == NULL){
		printf("Failed to get valid framebuffer config. Exiting now..\n");
		uninitialize();
		exit(1);
	}
	
	printf("%d matching FB configs found.\n",iNumFBConfigs);

	//Pick that FB config which has more samples per pixel
	int bestFramebufferconfig = -1, worstFramebufferConfig = -1, bestNumberOfSamples = -1, worstNumberOfSamples = 999;

	for(i=0; i< iNumFBConfigs; i++){
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfigs[i]);
		if(pTempXVisualInfo){
			int sampleBuffer, samples;

			glXGetFBConfigAttrib(gpDisplay, pGLXFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffer);
			glXGetFBConfigAttrib(gpDisplay, pGLXFBConfigs[i], GLX_SAMPLES, &samples);

			printf("Matching framebuffer config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d :SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);

			if(bestFramebufferconfig < 0 || sampleBuffer && samples >bestNumberOfSamples ){
				bestFramebufferconfig = i;
				bestNumberOfSamples = samples;
			}
			if(worstNumberOfSamples < 0 || !sampleBuffer || samples < worstNumberOfSamples){
				worstFramebufferConfig = i;
				worstNumberOfSamples = samples;
			}

			XFree(pTempXVisualInfo);
		}

	}//for loop ends

	bestGLXFBConfig = pGLXFBConfigs[bestFramebufferconfig];
	gGLXFBConfig = bestGLXFBConfig;
	
	XFree(pGLXFBConfigs);

	gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, bestGLXFBConfig);
	
	printf("Choose visual ID=0x%lu\n", gpXVisualInfo->visualid);
	

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
									RootWindow(gpDisplay,gpXVisualInfo->screen),
									gpXVisualInfo->visual,
									AllocNone);
	gColormap=winAttribs.colormap;

	//winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask|ButtonPressMask;

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

		if(!gWindow){
			printf("Error: Failed to Create Main Window.\n Exiting now..\n");
			uninitialize();
			exit(1);
		}
	XStoreName(gpDisplay,gWindow,"FBConfig Window");
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
}
void initialize(){
	//create new GL context 4.5 for rendering
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");
	
	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB,4,
		GLX_CONTEXT_MINOR_VERSION_ARB,5,
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 
		0//end of array
	};

	gGLXContext=glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0, True , attribs);
	if(!gGLXContext){
		GLint attribs[]={
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0
		};
		printf("Failed to create GLX 4.5 context. Hence using old style GLX context\n");
		gGLXContext=glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0, True , attribs);
	}else{
		printf("Successfully create OpenGL context 4.5\n");
	}

	//verify that above created context is direct context

	if(!glXIsDirect(gpDisplay, gGLXContext)){
		printf("Indirect GLX rendering context obtained\n");
	}else{
		printf("Direct GLX rendering context obtained\n");
	}
	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	resize(giWindowWidth,giWindowHeight);
}
void drawTriangle(){
	glLineWidth(1);
	glBegin(GL_TRIANGLES);
	//front face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top
	
	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(-1.0f, -1.0f, 1.0f); //left bottom
	
	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, -1.0f, 1.0f); //right bottom
	
	//right face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top
	
	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, -1.0f, 1.0f);  //left bottom
	
	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, -1.0f, -1.0f); //right bottom
	
	//back face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top
	
	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, -1.0f, -1.0f); //left bottom
	
	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(-1.0f, -1.0f, -1.0f); //right bottom
	
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(0.0f, 1.0f, 0.0f); //top
	
	glColor3f(0.0f, 0.0f, 1.0f); //blue
	
	glVertex3f(-1.0f, -1.0f, -1.0f); //left bottom
	
	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(-1.0f, -1.0f, 1.0f); //right bottom

	glEnd();
}
void drawRectangle(){
	glLineWidth(1);
	glBegin(GL_QUADS);
	//front face
	glColor3f(1.0f, 0.0f, 0.0f); //red
	glVertex3f(-1.0f, 1.0f, 1.0f); //left top
	glVertex3f(-1.0f, -1.0f, 1.0f); //left bottom
	glVertex3f(1.0f, -1.0f, 1.0f); //right bottom
	glVertex3f(1.0f, 1.0f, 1.0f); //right top
	
	//right face
	glColor3f(0.0f, 1.0f, 0.0f); //green
	glVertex3f(1.0f, 1.0f, 1.0f); //left top
	glVertex3f(1.0f, -1.0f, 1.0f); //left bottom
	glVertex3f(1.0f, -1.0f, -1.0f); //right bottom
	
	glVertex3f(1.0f, 1.0f, -1.0f); //right top
	
	//back face
	glColor3f(0.0f, 0.0f, 1.0f); //blue
	glVertex3f(1.0f, 1.0f, -1.0f); // left top
	glVertex3f(1.0f, -1.0f, -1.0f); //left bottom
	glVertex3f(-1.0f, -1.0f, -1.0f); //right bottom
	glVertex3f(-1.0f, 1.0f, -1.0f); //right top
	
	//left face
	glColor3ub(0, 149, 237); //corn flower
	glVertex3f(-1.0f, 1.0f, -1.0f); //left top
	glVertex3f(-1.0f, -1.0f, -1.0f);; //left bottom
	glVertex3f(-1.0f, -1.0f, 1.0f); //left bottom
	glVertex3f(-1.0f, 1.0f, 1.0f); //left top

	//top face
	glColor3ub(50, 0, 144); //corn flower
	glVertex3f(-1.0f, 1.0f, 1.0f); //left top
	glVertex3f(1.0f, 1.0f, 1.0f); //left top
	glVertex3f(1.0f, 1.0f, -1.0f);; //left bottom
	glVertex3f(-1.0f, 1.0f, -1.0f); //left bottom
	
	//bottom face
	glColor3ub(255, 128, 0); //corn flower
	glVertex3f(-1.0f, -1.0f, 1.0f); //left top
	glVertex3f(1.0f, -1.0f, 1.0f); //left top
	glVertex3f(1.0f, -1.0f, -1.0f);; //left bottom
	glVertex3f(-1.0f, -1.0f, -1.0f); //left bottom
	

	glEnd();

}
void display(){
	update();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.6f,0.0f,-7.0f);
	glRotatef(angleTri,0.0f,1.0,0.0f);
	drawTriangle();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(1.0f,0.0f,-7.0f);
	glRotatef(angleRect,1.0f,0.0f,0.0f);
	drawRectangle();
	//fprintf(fp,"angleTri:%f angleRect:%f\n",angleTri,angleRect);
	glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int width, int height)
{
	if(height==0)
		height=1;
	glViewport(0.0f,0.0f,(GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

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
	if(angleTri < 360.0 ){
		angleTri+=0.1f;
	}else
		angleTri=0.0f;

	if(angleRect > 0.0 ){
		angleRect-=0.1f;
	}else
		angleRect=360.0;

}














