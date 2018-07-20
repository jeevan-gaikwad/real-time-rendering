#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"
#import "GLESView.h"
using namespace vmath;
@implementation GLESView
{
    EAGLContext *eaglContext;
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
    
    id displayLink;
    NSInteger animationFrameInterval;
    BOOL isAnimating;
    GLint gWidth,gHeight;
    
    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;
    GLuint shaderProgramObject;
    
    GLuint vao_triangle;
    GLuint gVao_square;
    
    GLuint vao;
    GLuint vbo_position;
    GLuint vbo_color;
    GLuint mvpUniform;
    
    vmath::mat4 perspectiveProjectionMatrix;
}

- (id) initWithFrame:(CGRect)frame
{
    self=[super initWithFrame:frame];
	if(self)
	{
        CAEAGLLayer *eaglLayer=(CAEAGLLayer *) super.layer;
        
        eaglLayer.opaque=YES;
        eaglLayer.drawableProperties=[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE],
                                      kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8,
                                      kEAGLDrawablePropertyColorFormat,nil];
        eaglContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if(eaglContext == nil)
        {
            [self release];
            return(nil);
        }
        [EAGLContext setCurrentContext:eaglContext];
        glGenFramebuffers(1, &defaultFramebuffer);
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        GLint backingWidth;
        GLint backingHeight;
        
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,  GL_RENDERBUFFER, depthRenderbuffer);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Failed to create complete framebuffer object %x\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
            glDeleteFramebuffers(1, &defaultFramebuffer);
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            glDeleteRenderbuffers(1, &depthRenderbuffer);
            return(nil);
        }
        printf("Render: %s | GL VERSION: %s | GLSL Version: %s \n",glGetString(GL_RENDERER),glGetString(GL_VERSION),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
        //some hard coded initializations
        isAnimating = NO;
        animationFrameInterval = 60;
        
        //-----VERTEX SHADER----
        //Create vertex shader. Vertex shader specialist
        vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        const GLchar *vertexShaderSourceCode =         //Source code of Vertex shader
        "#version 300 es" \
        "\n" \
        "in vec4 vPosition;" \
        "uniform mat4 u_mvp_matrix;" \
        "void main(void)" \
        "{" \
        "gl_Position = u_mvp_matrix * vPosition;" \
        "}";
        
        glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL); //NULL is for NULL terminated source code string
        
        //compile vertex shader
        glCompileShader(vertexShaderObject);
        //Shader compilation error checking goes here...
        GLint iInfoLogLength = 0;
        GLint iShaderCompiledStatus = 0;
        char* szInfoLog = NULL;
        glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
        if (iShaderCompiledStatus == GL_FALSE) {
            glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
            if (iInfoLogLength > 0) {
                szInfoLog = (char*)malloc(iInfoLogLength);
                if (szInfoLog != NULL) {
                    GLsizei written;
                    glGetShaderInfoLog(vertexShaderObject, iInfoLogLength, &written, szInfoLog);
                   printf("Vertex shader compilation log:%s",
                            szInfoLog);
                    free(szInfoLog);
                    [self release];
                }
            }
        }
        //-----FRAGMENT SHADER----
        //Create fragment shader. Fragment shader specialist
        fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        //source code of fragment shader
        const GLchar *fragmentShaderSourceCode =      //Source code of Fragment shader
        "#version 300 es" \
        "\n" \
        "precision highp float;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = vec4(1.0 , 1.0, 1.0, 1.0);" \
        "}";
        glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
        
        //compile fragment shader
        glCompileShader(fragmentShaderObject);
        //Shader compilation errors goes here..
        glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
        if (iShaderCompiledStatus == GL_FALSE) {
            glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
            if (iInfoLogLength > 0) {
                szInfoLog = (char*)malloc(iInfoLogLength);
                if (szInfoLog != NULL) {
                    GLsizei written = 0;
                    glGetShaderInfoLog(fragmentShaderObject, iInfoLogLength, &written, szInfoLog);
                   printf("Fragment shader compilation log:%s",
                            szInfoLog);
                    free(szInfoLog);
                    [self release];
                   
                }
            }
        }
        
        //Create shader program
        shaderProgramObject = glCreateProgram();
        
        //attach shaders to the program
        glAttachShader(shaderProgramObject, vertexShaderObject);
        
        glAttachShader(shaderProgramObject, fragmentShaderObject);
        
        //map our(RAM) memory identifier to GPU memory(VRAM) identifier
        glBindAttribLocation(shaderProgramObject, JCG_ATTRIBUTE_VERTEX, "vPosition");
        
        //Link shader program
        glLinkProgram(shaderProgramObject);
        //Linking error checks goes here...
        
        GLint iShaderProgramLinkStatus = 0;
        glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
        if (iShaderProgramLinkStatus == GL_FALSE) {
            glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
            if (iInfoLogLength > 0) {
                szInfoLog = (char*)malloc(iInfoLogLength);
                if (szInfoLog != NULL) {
                    GLsizei written;
                    glGetProgramInfoLog(shaderProgramObject, iInfoLogLength, &written, szInfoLog);
                   printf("Shader program linking log:%s", szInfoLog);
                    [self release];
                }
            }
        }
        //Preparation to put our dynamic(uniform) data into the shader
        mvpUniform = glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
        
        const GLfloat triangleVertices[] = {
            0.0f, 1.0f, 0.0f, //apex of the triangle
            -1.0f, -1.0f, 0.0f, //left-bottom
            1.0f, -1.0f, 0.0f //right-bottom
        };
        
        //create a vao
        glGenVertexArrays(1, &vao_triangle);
        glBindVertexArray(vao_triangle);
        
        glGenBuffers(1, &vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        //square
        const GLfloat squareVertices[] = {
            -1.0f, 1.0f, 0.0f, //left top
            -1.0f, -1.0f, 0.0f,  //left bottom
            1.0f, -1.0f, 0.0f,  //right bottom
            1.0f, 1.0f, 0.0f //right top
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);  //done with triangle vao
        
        glGenVertexArrays(1, &gVao_square);
        glBindVertexArray(gVao_square);
        //set square position
        glGenBuffers(1, &vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); //done with square vao
        
        glGenVertexArrays(1, &gVao_square);
        glBindVertexArray(gVao_square);
        //set square position
        glGenBuffers(1, &vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); //done with square vao
        //unbind vao
         glBindVertexArray(0);
        // set-up depth buffer
        
        // enable depth testing
        glEnable(GL_DEPTH_TEST);
        // depth test to do
        glDepthFunc(GL_LEQUAL);
        
        // We will always cull back faces for better performance
        glEnable(GL_CULL_FACE);
        
        // set background color to which it will display even if it will empty. THIS LINE CAN BE IN drawRect().
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // blue
        perspectiveProjectionMatrix = vmath::mat4::identity();
        
		//Gesture recognition
		
		UITapGestureRecognizer *singleTapGestureRecognizer=
		[[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
		[singleTapGestureRecognizer setNumberOfTapsRequired:1];
		[singleTapGestureRecognizer setNumberOfTouchesRequired:1];
		[singleTapGestureRecognizer setDelegate:self];
		[self addGestureRecognizer :singleTapGestureRecognizer];
		
		UITapGestureRecognizer *doubleTapGestureRecognizer=[[UITapGestureRecognizer alloc]initWithTarget:self action:
		@selector(onDoubleTap:)];
		[doubleTapGestureRecognizer setNumberOfTapsRequired:2];
		[doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
		[doubleTapGestureRecognizer setDelegate:self];
		[self addGestureRecognizer:doubleTapGestureRecognizer];
		
		[singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
		
		//Swipe gesture
		UISwipeGestureRecognizer *swipeGestureRecognizer=[[UISwipeGestureRecognizer alloc]initWithTarget:self action:
		@selector(onSwipe:)];
		[self addGestureRecognizer:swipeGestureRecognizer];
		//long press gesture
		UILongPressGestureRecognizer *longPressGestureRecognizer=[[UILongPressGestureRecognizer alloc]initWithTarget:self action:
		@selector(onLongPress:)];
		[self addGestureRecognizer:longPressGestureRecognizer];
	}
	return (self);
}
/*
//Only override draw rect
- (void)drawRect:(CGRect)rect
{
	
}
*/
+(Class)layerClass
{
    return([CAEAGLLayer class]);
}
-(void)drawView:(id)sender
{
    [EAGLContext setCurrentContext:eaglContext];
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //Start using shader program object
    glUseProgram(shaderProgramObject); //run shaders
    
    //OpenGL drawing
    
    //set modleview and projection matrices to identity matrix
    
    mat4 modelViewMatrix = mat4::identity();
    mat4 modelViewProjectionMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    
    modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
    translationMatrix = vmath::translate(-2.0f, 0.0f, -4.0f);
    modelViewMatrix = modelViewMatrix * translationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
    
    // Pass above model view matrix projection matrix to vertex shader in 'u_mvp_matrix' shader variable
    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
    //bind triangle vao
    glBindVertexArray(vao_triangle);
    // Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glDrawArrays(GL_TRIANGLES, 0, 3); //3 is no of positions
    
    glBindVertexArray(0);
    
    //bind square vao
    glBindVertexArray(gVao_square);
    //shift square to right side
    modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
    translationMatrix = vmath::translate(2.0f, 0.0f, -4.0f);
    modelViewMatrix = modelViewMatrix * translationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    // Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //3 is no of positions
    
    glBindVertexArray(0);
    
    //stop using shaders
    glUseProgram(0);    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}
-(void)layoutSubviews
{
    GLint width;
    GLint height;
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    glViewport(0, 0, width, height);
    gWidth=width;
    gHeight=height;
    printf("Screen widhth: %d height:%d\n",width,height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)width / (GLfloat)height),0.1f,100.0f);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed to create complete framebuffer object:%x",glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    [self drawView:nil];
}
-(void)startAnimation
{
    if(!isAnimating)
    {
        displayLink=[NSClassFromString(@"CADisplayLink")displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink setPreferredFramesPerSecond:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        isAnimating=YES;
    }
}
-(void)stopAnimation
{
    if(isAnimating)
    {
        [displayLink invalidate];
        displayLink=nil;
        isAnimating=NO;
    }
}
//Become first responder
-(BOOL) acceptsFirstResponder
{
	//code
	return (YES);
}

- (void) touchesBegan: (NSSet *)touches withEvent:(UIEvent *) event
{
	
}

-(void)onSingleTap:(UITapGestureRecognizer *) gr
{
    
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gr
{

}

-(void)onSwipe:(UISwipeGestureRecognizer *)gr
{
	[self release];
	exit(0);
}

-(void)onLongPress:(UILongPressGestureRecognizer *)gr
{
    
}

-(void)dealloc
{
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vao_triangle) {
        glDeleteVertexArrays(1, &vao_triangle);
        vao_triangle = 0;
    }
    if (gVao_square) {
        glDeleteVertexArrays(1, &gVao_square);
        gVao_square = 0;
    }
    if (vbo_position) {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0;
    }
    
    //Detach shaders
    //Detach vertex shader
    glDetachShader(shaderProgramObject, vertexShaderObject);
    
    //Detach fragment shader
    glDetachShader(shaderProgramObject, fragmentShaderObject);
    
    //Now delete shader objects
    
    //Delete vertex shader object
    glDeleteShader(vertexShaderObject);
    vertexShaderObject = 0;
    
    //Delete fragment shader object
    glDeleteShader(fragmentShaderObject);
    fragmentShaderObject = 0;
    
    //Delete shader program object
    glDeleteProgram(shaderProgramObject);
    shaderProgramObject = 0;
    
    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer=0;
    }
    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer=0;
    }
    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1, &defaultFramebuffer);
        defaultFramebuffer=0;
    }
    if([EAGLContext currentContext]==eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
    }
    [eaglContext release];
    eaglContext=nil;
    
	[super dealloc];
}

@end
