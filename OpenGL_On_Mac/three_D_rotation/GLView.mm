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
	
    GLuint gVao_pyramid;
	GLuint gVbo_color_pyramid;
	GLuint gVbo_position_pyramid;
	GLuint gVao_cube;
	GLuint gVbo_color_cube;
	GLuint gVbo_position_cube;
	GLuint gMVPUniform;

	GLfloat angle_pyramid;
	GLfloat angle_cobe;
    vmath::mat4 gPerspectiveProjectionMatrix;
}


-(id)initWithFrame:(NSRect)frame;
{
    //code
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
		"in vec4 vColor;" \
		"out vec4 out_color;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_color = vColor;" \
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
                fprintf(gpFile, "Fragment shader compilation log:%s",
                        szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
        }
    }
    }
    
    //Create shader program
    gShaderProgramObject = glCreateProgram();
    
    //attach shaders to the program
    glAttachShader(gShaderProgramObject, gVertexShaderObject);
    
    glAttachShader(gShaderProgramObject, gFragmentShaderObject);
    
    //map our(RAM) memory identifier to GPU memory(VRAM) identifier
    glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, JCG_ATTRIBUTE_COLOR, "vColor");
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
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");

	// Vertices, colors, shader attribs, vbo, vao initializations

	const GLfloat pyramidVertices[] = {
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

	//create a vao for triangle
	glGenVertexArrays(1, &gVao_pyramid);
	glBindVertexArray(gVao_pyramid);
	//set triangle position
	glGenBuffers(1, &gVbo_position_pyramid);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_pyramid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);

	//vbo for pyramid colors
	const GLfloat pyramidColors[] = {
		1.0f, 0.0f, 0.0f, //red
		0.0f, 1.0f, 0.0f, //green
		0.0f, 0.0f, 1.0f, //blue
		1.0f, 0.0f, 0.0f, //red
		0.0f, 1.0f, 0.0f, //green
		0.0f, 0.0f, 1.0f, //blue
		1.0f, 0.0f, 0.0f, //red
		0.0f, 1.0f, 0.0f, //green
		0.0f, 0.0f, 1.0f, //blue
		1.0f, 0.0f, 0.0f, //red
		0.0f, 1.0f, 0.0f, //green
		0.0f, 0.0f, 1.0f //blue
	};

	glGenBuffers(1, &gVbo_color_pyramid);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_color_pyramid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidColors), pyramidColors, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_COLOR);

	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);  //done with pyramid vao

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
	//cube colors
	const GLfloat cubeColors[] = {
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,

		
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,

		1.0f,0.5f,0.0f,
		1.0f,0.5f,0.0f,
		1.0f,0.5f,0.0f,
		1.0f,0.5f,0.0f,

		0.0f,0.7f,1.0f,
		0.0f,0.7f,1.0f,
		0.0f,0.7f,1.0f,
		0.0f,0.7f,1.0f,

		
	};
	glGenVertexArrays(1, &gVao_cube);
	glBindVertexArray(gVao_cube);
	//set square position
	glGenBuffers(1, &gVbo_position_cube);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
	
	glGenBuffers(1, &gVbo_color_cube);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_color_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);

	glVertexAttribPointer(JCG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(JCG_ATTRIBUTE_COLOR);

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
    //Start using shader program object
	glUseProgram(gShaderProgramObject); //run shaders

	//OpenGL drawing

	//set modleview and projection matrices to identity matrix

	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
	translationMatrix = vmath::translate(-2.0f, 0.0f, -4.0f);
	modelViewMatrix = modelViewMatrix * translationMatrix;
	rotationMatrix = vmath::rotate(angle_pyramid, 0.0f, 1.0f, 0.0f);
	modelViewMatrix = modelViewMatrix * rotationMatrix ;
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP

	// Pass above model view matrix projection matrix to vertex shader in 'u_mvp_matrix' shader variable

	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//bind triangle vao
	glBindVertexArray(gVao_pyramid);
	// Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glDrawArrays(GL_TRIANGLES, 0, 12); //3 is no of positions

	glBindVertexArray(0);
	
	//bind square vao
	glBindVertexArray(gVao_cube);
	//shift square to right side
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
	translationMatrix = vmath::translate(2.0f, 0.0f, -4.0f);
	rotationMatrix = mat4::identity();
	rotationMatrix = vmath::rotate(angle_cobe,1.0f, 0.0f, 0.0f); //x axis rotation
	rotationMatrix = rotationMatrix * vmath::rotate(angle_cobe, 0.0f, 1.0f, 0.0f); //y axis rotation 
	rotationMatrix = rotationMatrix * vmath::rotate(angle_cobe, 0.0f, 0.0f, 1.0f); //z axis rotation

	modelViewMatrix = modelViewMatrix * translationMatrix;
	modelViewMatrix = modelViewMatrix * vmath::scale(0.75f,0.75f,0.75f);
	modelViewMatrix = modelViewMatrix * rotationMatrix;
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	// Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
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
	if (angle_cobe > 360.0f) {
		angle_cobe = 0.0f;
	}
	else
		angle_cobe = angle_cobe + 0.2f;

	if (angle_pyramid > 360.0f) {
		angle_pyramid = 0.0f;
	}
	else
		angle_pyramid = angle_pyramid + 0.2f;

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
    switch(key)
    {
        case 27://Esc key
            [self release];
            [NSApp terminate:self];
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
    
    if (gVao_pyramid) {
		glDeleteVertexArrays(1, &gVao_pyramid);
		gVao_pyramid = 0;
	}

	if (gVao_cube) {
		glDeleteVertexArrays(1, &gVao_cube);
		gVao_cube = 0;
	}

	if (gVbo_position_pyramid) {
		glDeleteBuffers(1, &gVbo_position_pyramid);
		gVbo_position_pyramid = 0;
	}
	if (gVbo_position_cube) {
		glDeleteBuffers(1, &gVbo_position_cube);
		gVbo_position_cube = 0;
	}
	if (gVbo_color_pyramid) {
		glDeleteBuffers(1, &gVbo_color_pyramid);
		gVbo_color_pyramid = 0;
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


