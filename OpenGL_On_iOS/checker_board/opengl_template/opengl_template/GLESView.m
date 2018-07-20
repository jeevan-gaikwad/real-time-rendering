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
    
    GLuint gVao_quad;
    GLuint gVbo_position_qaud;
    GLuint vbo_texture_qaud;
    
    GLuint gTexture_sampler_uniform; //for uniform(dynamic) texture data
    GLuint gMVPUniform;
    GLubyte coloredBoardTexture[64][64][4];
    GLuint gColoredBoardTexture;
    int tap_count;
    vmath::mat4 perspectiveProjectionMatrix;
}

- (id) initWithFrame:(CGRect)frame
{
    tap_count=0;
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
        "in vec2 vTexture0_Coord;" \
        "out vec2 out_texture0_coord;" \
        "uniform mat4 u_mvp_matrix;" \
        "void main(void)" \
        "{" \
        "gl_Position = u_mvp_matrix * vPosition;" \
        "out_texture0_coord = vTexture0_Coord;" \
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
        "in vec2 out_texture0_coord;" \
        "out vec4 FragColor;" \
        "uniform sampler2D u_texture0_sampler;" \
        "void main(void)" \
        "{" \
        "FragColor = texture(u_texture0_sampler, out_texture0_coord);" \
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
        glBindAttribLocation(shaderProgramObject, JCG_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");
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
        gMVPUniform = glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
        gTexture_sampler_uniform = glGetUniformLocation(shaderProgramObject, "u_texture0_sampler");
        [self GenerateGLTextures]; //Load procedural texture
        // Vertices, colors, shader attribs, vbo, vao initializations
        
        GLfloat quadTexcoords[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f
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
        
        glGenBuffers(1, &vbo_texture_qaud);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_qaud);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexcoords), quadTexcoords, GL_STATIC_DRAW); //providing texture data statically
        
        glVertexAttribPointer(JCG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL); //note 2. We've s and t for texture coords
        glEnableVertexAttribArray(JCG_ATTRIBUTE_TEXTURE0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glBindVertexArray(0);  //done with quad1 vao
        // set-up depth buffer
        
        // enable depth testing
        glEnable(GL_DEPTH_TEST);
        // depth test to do
        glDepthFunc(GL_LEQUAL);
        
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
-(void)update
{
    
    
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
    const GLfloat quad1Vertices[] = {
        -2.0f, 1.0f, 0.0f,
        -2.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    const GLfloat tiltedQuadVertices[] = {
        2.41421f, -1.0f, -1.41421f,
        2.41421f, 1.0f, -1.41421f,
        1.0f,     1.0f,  0.0f,
        1.0f, -1.0f, 0.0f
    };
    //Start using shader program object
    glUseProgram(shaderProgramObject); //run shaders
    
    //OpenGL drawing
    
    //set modleview and projection matrices to identity matrix
    
    mat4 modelViewMatrix = mat4::identity();
    mat4 modelViewProjectionMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();
    
    translationMatrix = vmath::translate(0.0f, 0.0f, -3.6f);
    modelViewMatrix = modelViewMatrix * translationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
    
    // Pass above model view matrix projection matrix to vertex shader in 'u_mvp_matrix' shader variable
    
    glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    //bind quad1 vao
    glBindVertexArray(gVao_quad);
    //bind with pyramid texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gColoredBoardTexture);
    glUniform1i(gTexture_sampler_uniform, 0);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_qaud);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad1Vertices), quad1Vertices, GL_DYNAMIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW
    // Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Draw tilted quad
    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -3.6f);
    modelViewMatrix = modelViewMatrix * translationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; //ORDER IS IMP
    glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
    
    glBindVertexArray(gVao_quad);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_qaud);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tiltedQuadVertices), tiltedQuadVertices, GL_DYNAMIC_DRAW); //we will provide data at runtime DYNAMIC_DRAW
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); //4 is no of positions
    
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
-(GLuint) loadTextureFromBMPFile:(NSString *)texFileName :(NSString *)extension
{
    NSLog(@"Tex file name: %@ extension:%@", texFileName, extension);
    NSString *textureFileNameWithPath=[[NSBundle mainBundle]pathForResource:texFileName ofType:extension];
    UIImage *bmpImage=[[UIImage alloc]initWithContentsOfFile:textureFileNameWithPath];
    
    if(!bmpImage)
    {
        NSLog(@"Can't find %@", textureFileNameWithPath);
        return(0);
    }
    CGImageRef cgImage = bmpImage.CGImage;
    
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
-(void) GenerateGLTextures
{
    
    [self MakeColoredBoard];
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &gColoredBoardTexture);
    glBindTexture(GL_TEXTURE_2D, gColoredBoardTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, coloredBoardTexture);
    
    
}
-(void) MakeColoredBoard
{
    int i, j,c;

    for (i = 0;i < 64;i++) {
        for (j = 0;j < 64;j++) {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
            if (c == 255) {
                coloredBoardTexture[i][j][0] = (GLubyte)0; //R
                coloredBoardTexture[i][j][1] = (GLubyte)255; //G
                coloredBoardTexture[i][j][2] = (GLubyte)0; //B
            }
            else {
                coloredBoardTexture[i][j][0] = (GLubyte)255;
                coloredBoardTexture[i][j][1] = (GLubyte)255;
                coloredBoardTexture[i][j][2] = (GLubyte)255;
            }
            coloredBoardTexture[i][j][3] = (GLubyte)255; //A
        }
    }
}
- (void) touchesBegan: (NSSet *)touches withEvent:(UIEvent *) event
{
    
}

-(void)onSingleTap:(UITapGestureRecognizer *) gr
{
   tap_count++;
    
    printf("tap_count:%d\n",tap_count);
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
    
    if (gVao_quad) {
        glDeleteVertexArrays(1, &gVao_quad);
        gVao_quad = 0;
    }
    
    if (gVbo_position_qaud) {
        glDeleteBuffers(1, &gVbo_position_qaud);
        gVbo_position_qaud = 0;
    }
    
    //unload textures
    if (gColoredBoardTexture) {
        glDeleteTextures(1, &gColoredBoardTexture);
        gColoredBoardTexture = 0;
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

