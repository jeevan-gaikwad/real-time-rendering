#import "GLView.h"
using namespace vmath;
extern FILE *gpFile;

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;
    
    GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;
	GLuint gShaderProgramObject;
	
	GLuint gVao_cube;
	GLuint gVbo_normal_cube;
	GLuint gVbo_position_cube;
	GLuint gMVPUniform;

	GLfloat angle_cube;
    vmath::mat4 gPerspectiveProjectionMatrix;
	GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
	GLuint gLdUniform, gKdUniform, gLightPositionUniform;
	GLuint gLKeyPressedUniform;
	bool gbAnimate;
	bool gbLight;
}


-(id)initWithFrame:(NSRect)frame;
{
    //code
	//Initialization
	gbLight = false;
	gbAnimate = false;
    self=[super initWithFrame:frame];
    
    if(self)
    {
        [[self window] setContentView:self];
        
    }
    
    NSOpenGLPixelFormatAttribute attrs[]=
    {
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion4_1Core,
        //specify about displayid
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADoubleBuffer,
        0}; //end of array elements
    
    NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc]initWithAttributes:attrs]autorelease];
    if(pixelFormat == nil)
    {

        fprintf(gpFile, "No valid OpenGL pixel format is available. Exit...");
        [self release];
        [NSApp terminate:self];
    }
    
    NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil]autorelease];
    
    [self setPixelFormat:pixelFormat];
    [self setOpenGLContext:glContext];
    return (self);
}

-(void)prepareOpenGL
{
    //OpenGL info
    fprintf(gpFile, "OpenGL version: %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL version  : %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    GLint swapInt = 1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    //-----VERTEX SHADER----
    //Create vertex shader. Vertex shader specialist
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar *vertexShaderSourceCode =         //Source code of Vertex shader
		"#version 410 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_LKeyPressed;"\
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Kd;" \
		"uniform vec4 u_light_position;" \
		"out vec3 diffuse_light;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyPressed == 1)" \
		"{"\
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"vec3 tnorm = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \
		"vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" \
		"diffuse_light = u_Ld * u_Kd * max(dot(s,tnorm),0.0);" \
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
                fprintf(gpFile, "Vertex shader compilation log:%s",
                        szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    //-----FRAGMENT SHADER----
    //Create fragment shader. Fragment shader specialist
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    //source code of fragment shader
    const GLchar *fragmentShaderSourceCode =      //Source code of Fragment shader
    	"#version 410 core" \
		"\n" \
		"in vec3 diffuse_light;"
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;"
		"void main(void)" \
		"{" \
		"vec4 color;" \
		"if(u_LKeyPressed == 1)" \
		"{" \
		"color = vec4(diffuse_light, 1.0);" \
		"}"\
		"else" \
		"{"
		"color = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = color;" \
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
                fprintf(gpFile, "Fragment shader compilation log:%s",
                        szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
        }
    }
    }
	fprintf(gpFile, "Fragment shader compilation success.");
    
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
                fprintf(gpFile, "Shader program linking log:%s", szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    //Preparation to put our dynamic(uniform) data into the shader
    //Preparation to put our dynamic(uniform) data into the shader
	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");
	gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");

	// Vertices, colors, shader attribs, vbo, vao initializations

	

	//cube
	const GLfloat cubeVertices[] = {
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
	
	const GLfloat cubeNormals[] = {

		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,

		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,

		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,

		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,

	};
	glGenVertexArrays(1, &gVao_cube);
	glBindVertexArray(gVao_cube);
	//set square position
	glGenBuffers(1, &gVbo_position_cube);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &gVbo_normal_cube);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_normal_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); //done with cube vao
	
    // set-up depth buffer
    glClearDepth(1.0f);
    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    // depth test to do
    glDepthFunc(GL_LEQUAL);
    
    // We will always cull back faces for better performance
    glEnable(GL_CULL_FACE);
    
    
    gPerspectiveProjectionMatrix = vmath::mat4::identity();

    //set background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext=(CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat=(CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
    
}

-(void)drawRect:(NSRect)dirtyRect
{
    [self drawView];
}

-(void)drawView
{
    [[self openGLContext]makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //Start using shader program object
	glUseProgram(gShaderProgramObject);
	
    if (gbLight == true) {
		glUniform1i(gLKeyPressedUniform, 1);
		glUniform3f(gLdUniform, 1.0f, 1.0f, 1.0f);
		glUniform3f(gKdUniform, 0.5f, 0.5f, 0.5f);
		float lightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };
		glUniform4fv(gLightPositionUniform, 1, (GLfloat*)lightPosition);
	}
	else {
		glUniform1i(gLKeyPressedUniform, 0);
	}

	//OpenGL drawing

										//set modleview and projection matrices to identity matrix

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	modelMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
	rotationMatrix = vmath::rotate(angle_cube, angle_cube, angle_cube); //arbitory all axes rotation

	modelMatrix = modelMatrix * rotationMatrix;
	
   // Pass above matrices to shaders

	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	//bind cube vao
	glBindVertexArray(gVao_cube);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions

	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glBindVertexArray(0);

	//stop using shaders
	glUseProgram(0);

	[self update]; //update angles
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}
-(void)update
{
	if (angle_cube > 360.0f) {
		angle_cube = 0.0f;
	}
	else
		angle_cube = angle_cube + 0.2f;

}
-(void)reshape
{
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    NSRect rect=[self bounds];
    
    GLfloat width=rect.size.width;
    GLfloat height=rect.size.height;
    
    if(height == 0)
        height=1;
    
    glViewport(0,0 , (GLsizei)width, (GLsizei)height);
    gPerspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)width / (GLfloat)height),0.1f,100.0f);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
}
-(CVReturn)getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    [self drawView];
    
    [pool release];
    return (kCVReturnSuccess);
}

-(BOOL)acceptsFirstResponder
{
    //code
    NSLog(@"In acceptFirstResponder");
    [[self window]makeFirstResponder:self];
    
    return(YES);
    
}

-(void)keyDown:(NSEvent *)theEvent
{
    int key=(int)[[theEvent characters]characterAtIndex:0];
	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;
    switch(key)
    {
        case 27://Esc key
            [self release];
            [NSApp terminate:self];
            break;
		case 65://A
		case 97://a
			NSLog(@"A/a key pressed");
			if (bIsAKeyPressed == false) {
				gbAnimate = true;
				bIsAKeyPressed = true;
			}
			else {
				gbAnimate = false;
				bIsAKeyPressed = false;
			}
			break;
		case 76://L
		case 108://l
			NSLog(@"L/l key pressed");
			if (bIsLKeyPressed == false) {
				gbLight = true;
				bIsLKeyPressed = true;
			}
			else {
				gbLight = false;
				bIsLKeyPressed = false;
			}
			break;
        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self]; //repainting occurs automatically
            break;
        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *)theEvent
{
    //code
    [self setNeedsDisplay:YES];
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    //mouse drag code will go here
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    [self setNeedsDisplay:YES];
}

-(void) dealloc
{
    //code
    
    if (gVbo_normal_cube) {
		glDeleteBuffers(1, &gVbo_normal_cube);
		gVbo_normal_cube = 0;
	}
	if (gVao_cube) {
		glDeleteVertexArrays(1, &gVao_cube);
		gVao_cube = 0;
	}

	if (gVbo_position_cube) {
		glDeleteBuffers(1, &gVbo_position_cube);
		gVbo_position_cube = 0;
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
	
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    [super dealloc];
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow,
                               const CVTimeStamp *pOutputTime, CVOptionFlags flagsIn,
                               CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result=[(GLView *)pDisplayLinkContext getFrameForTime:pOutputTime];
    return(result);
}


