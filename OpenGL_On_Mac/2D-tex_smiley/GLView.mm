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
	
	GLuint gVao_quad;
	GLuint gVbo_position_qaud;
	GLuint gVbo_texture_smiley;
	
	GLuint gTexture_smiley_face;
	GLuint gTexture_sampler_uniform; //for uniform(dynamic) texture data
	GLuint gMVPUniform;
	
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
                fprintf(gpFile, "Shader program linking log:%s", szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    //Preparation to put our dynamic(uniform) data into the shader
    //Preparation to put our dynamic(uniform) data into the shader
	gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	gTexture_sampler_uniform = glGetUniformLocation(gShaderProgramObject, "u_texture0_sampler");
	
	gTexture_smiley_face=[self loadTextureFromBMPFile:"Smiley-512x512.bmp"];
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

	glBindVertexArray(0);  //done with quad1 vao
	
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
	const GLfloat smileyFaceVertices[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
    //Start using shader program object
	glUseProgram(gShaderProgramObject); //run shaders

	//OpenGL drawing

	//set modleview and projection matrices to identity matrix

	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	translationMatrix = vmath::translate(0.0f, 0.0f, -3.6f);
	modelViewMatrix = modelViewMatrix * translationMatrix;
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

	
	glBindVertexArray(0);

	//stop using shaders
	glUseProgram(0);

	
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
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

-(GLuint) loadTextureFromBMPFile:(const char *)texFileName
{
	NSBundle *mainBundle=[NSBundle mainBundle];
	NSString *appDirName=[mainBundle bundlePath];
	NSString *parentDirPath=[appDirName stringByDeletingLastPathComponent];
	NSString *textureFileNameWithPath=[NSString stringWithFormat:@"%@/%s", parentDirPath, texFileName];
	
	NSImage *bmpImage=[[NSImage alloc] initWithContentsOfFile:textureFileNameWithPath];
	if(!bmpImage)
	{
		NSLog(@"Can't find %@", textureFileNameWithPath);
		return(0);
	}
	
	CGImageRef cgImage = [bmpImage CGImageForProposedRect:nil context:nil hints:nil];
	
	int w = (int) CGImageGetWidth(cgImage);
	int h = (int) CGImageGetHeight(cgImage);
	CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
	
	void *pixels = (void*) CFDataGetBytePtr(imageData);
	
	GLuint bmpTexture;
	glGenTextures(1, &bmpTexture);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, bmpTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,pixels);
	
	glGenerateMipmap(GL_TEXTURE_2D);
	CFRelease(imageData);
	return (bmpTexture);
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
    
    if (gVao_quad) {
		glDeleteVertexArrays(1, &gVao_quad);
		gVao_quad = 0;
	}

	if (gVbo_position_qaud) {
		glDeleteBuffers(1, &gVbo_position_qaud);
		gVbo_position_qaud = 0;
	}
	
	//unload textures
	if (gTexture_smiley_face) {
		glDeleteTextures(1, &gTexture_smiley_face);
		gTexture_smiley_face = 0;
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


