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

- (id) initWithFrame:(CGRect)frame
{
    self=[super initWithFrame:frame];
    //Initialization
    gbLight = true;
    gbAnimate = false;
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
        gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
        
        const GLchar *vertexShaderSourceCode =         //Source code of Vertex shader
        "#version 300 es" \
        "\n" \
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
                   printf("Vertex shader compilation log:%s",
                            szInfoLog);
                    free(szInfoLog);
                    [self release];
                }
            }
        }
        //-----FRAGMENT SHADER----
        //Create fragment shader. Fragment shader specialist
        gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
        
        //source code of fragment shader
        const GLchar *fragmentShaderSourceCode =      //Source code of Fragment shader
        "#version 300 es" \
        "\n" \
        "precision highp float;" \
        "in vec3 diffuse_light;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "vec4 color;" \
        "int u_LKeyPressed=1;" \
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
                   printf("Fragment shader compilation log:%s",
                            szInfoLog);
                    free(szInfoLog);
                    [self release];
                   
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
                   printf("Shader program linking log:%s", szInfoLog);
                    [self release];
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
        
        // enable depth testing
        glEnable(GL_DEPTH_TEST);
        // depth test to do
        glDepthFunc(GL_LEQUAL);
        
        // set background color to which it will display even if it will empty. THIS LINE CAN BE IN drawRect().
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // blue
        gPerspectiveProjectionMatrix = vmath::mat4::identity();
        
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
-(void)update
{
    
    if (angle_cube > 360.0f) {
        angle_cube = 0.0f;
    }
    else
        angle_cube = angle_cube + 0.2f;
    
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
    glUseProgram(gShaderProgramObject); //run shaders
    
    //OpenGL drawing
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

    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
    [self update];
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
    gPerspectiveProjectionMatrix = vmath::perspective(45.0f, ((GLfloat)width / (GLfloat)height),0.1f,100.0f);
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
    if(gbLight==true)
        gbLight=false;
    else
        gbLight=true;
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
