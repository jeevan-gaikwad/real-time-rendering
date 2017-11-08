#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

using namespace std;

bool gbFullScreen=false;

Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

int main(void){
	void CreateWindow(void);
	void ToggleFullscreen();
	void uninitialize();
	
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;

	CreateWindow();

	XEvent event;
	KeySym keysym;

	while(1){
		XNextEvent(gpDisplay,&event);

		switch(event.type){
			case MapNotify:
				break;
			case KeyPress:
				keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym){
					case XK_Escape:
							uninitialize();
							exit(0);
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
					break;
			case Expose:
					break;
			case DestroyNotify:
					break;
			case 33: //close button event
					uninitialize();
					exit(0);
			default:
				break;
		}
	
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

	//code
	gpDisplay=XOpenDisplay(NULL);

	if(gpDisplay==NULL){
		
		printf("Error: Unable to Open X Display.\n Exiting now....\n");
		uninitialize();
		exit(1);

	}

	defaultScreen=XDefaultScreen(gpDisplay);

	defaultDepth=DefaultDepth(gpDisplay,defaultScreen);

	gpXVisualInfo=(XVisualInfo*)malloc(sizeof(XVisualInfo));
	
	if(gpXVisualInfo==NULL){
		printf("Error: Unable to allocate memory for visual info. Exiting now...\n");
		uninitialize();
		exit(1);
	}

	if(XMatchVisualInfo(gpDisplay,defaultScreen, defaultDepth, TrueColor, gpXVisualInfo) == 0)
	{
		printf("Error: Unable to get A Visual.\n Exiting now...\n");
		uninitialize();
		exit(1);
	}

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
	XStoreName(gpDisplay,gWindow,"FirstWindow");
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
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
















