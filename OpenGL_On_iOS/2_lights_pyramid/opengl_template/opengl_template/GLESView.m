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
    
    GLuint gVao_pyramid;
    GLuint gVbo_normals_pyramid;
    GLuint gVbo_position_pyramid;
    
    GLuint gMVPUniform;
    GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
    GLuint gLight0PositionUniform, gLight1PositionUniform;
    GLuint gLKeyPressedUniform;
    GLuint L0a_uniform, L0d_uniform, L0s_uniform; //light0
    GLuint L1a_uniform, L1d_uniform, L1s_uniform; //light1
    GLuint Ka_uniform, Kd_uniform, Ks_uniform;
    GLuint material_shininess_uniform;
    bool gbAnimate;
    bool gbLight;
    GLfloat angle_pyramid;
    
    vmath::mat4 gPerspectiveProjectionMatrix;
    float sphere_vertices[1146];
    float sphere_normals[1146];
    float sphere_textures[764];
    unsigned short sphere_elements[2280];
    unsigned int gNumVertices, gNumElements;
    
    //lighting details
    GLfloat light0Ambient[4];
    GLfloat light0Diffuse[4];
    GLfloat light0Specular[4];
    GLfloat light0Position[4];
    
    GLfloat light1Ambient[4];
    GLfloat light1Diffuse[4];
    GLfloat light1Specular[4];
    GLfloat light1Position[4];
    
    GLfloat material_ambient[4];// = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat material_diffuse[4];// = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat material_specular[4];// = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat material_shininess;// = 50.0f;

}

- (id) initWithFrame:(CGRect)frame
{
    self=[super initWithFrame:frame];
    gbLight = false;
    //light0
    light0Ambient[0]=light0Ambient[1]=light0Ambient[2]=0.0f;
    light0Ambient[3]=1.0f;
    
    light0Diffuse[0]= 1.0f;
    light0Diffuse[1]=light0Diffuse[2]=light0Diffuse[3]=0.0f;
    light0Specular[0]= 1.0f;
    light0Specular[1]=light0Specular[2]=light0Specular[3]=0.0f;
    
    light0Position[0]=light0Position[1]=2.0f;
    light0Position[2]=0.0f;
    light0Position[3]=1.0f;
    
    //light1
    light1Ambient[0]=light1Ambient[1]=light1Ambient[2]=0.0f;
    light1Ambient[3]=1.0f;
    
    light1Diffuse[0]=light1Diffuse[1]=light1Diffuse[3]=0.0f;
    light1Diffuse[2]= 1.0f;
    
    light1Specular[0]=light1Specular[1]=light1Specular[3]=0.0f;
    light1Specular[2]=1.0f;
    
    light1Position[0]=-2.0f;
    light1Position[1]=2.0f;
    light1Position[2]=0.0f;
    light1Position[3]=1.0f;
    
    material_ambient[0]=material_ambient[1]=material_ambient[2]=0.0f;
    material_ambient[3]=1.0f;
    
    material_diffuse[0]=material_diffuse[1]=material_diffuse[2]=material_diffuse[3]=1.0f;
    material_specular[0]=material_specular[1]=material_specular[2]=material_specular[3]=1.0f;
    material_shininess = 50.0f;
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
        "in vec4 vPosition;" \
        "in vec3 vNormal;" \
        "uniform mat4 u_model_matrix;" \
        "uniform mat4 u_view_matrix;" \
        "uniform mat4 u_projection_matrix;" \
        "uniform int u_lighting_enabled;" \
        "uniform vec3 u_L0a;" \
        "uniform vec3 u_L0d;" \
        "uniform vec3 u_L0s;" \
        "uniform vec4 u_light0_position;" \
        "uniform vec3 u_L1a;" \
        "uniform vec3 u_L1d;" \
        "uniform vec3 u_L1s;" \
        "uniform vec4 u_light1_position;" \
        "uniform vec3 u_Ka;" \
        "uniform vec3 u_Kd;" \
        "uniform vec3 u_Ks;" \
        "uniform float u_material_shininess;" \
        "out vec3 phong_ads_color;" \
        "void calculate_light_ads(vec3 La,vec3 Ld, vec3 Ls)"\
        "{" \
        "}"\
        "void main(void)" \
        "{" \
        "if(u_lighting_enabled == 1)" \
        "{"\
        "vec4 eye_coordinates = u_view_matrix* u_model_matrix * vPosition;" \
        "vec3 transformed_normals = normalize(mat3(u_view_matrix*u_model_matrix) * vNormal);" \
        "vec3 light0_direction = normalize(vec3(u_light0_position) - eye_coordinates.xyz);" \
        "vec3 light1_direction = normalize(vec3(u_light1_position) - eye_coordinates.xyz);" \
        "float tn_dot_ld0 = max(dot(transformed_normals, light0_direction), 0.0);" \
        "float tn_dot_ld1 = max(dot(transformed_normals, light1_direction), 0.0);" \
        "vec3 ambient0 = u_L0a * u_Ka;" \
        "vec3 ambient1 = u_L1a * u_Ka;" \
        "vec3 diffuse0 = u_L0d * u_Kd * tn_dot_ld0;" \
        "vec3 diffuse1 = u_L1d * u_Kd * tn_dot_ld1;" \
        "vec3 reflection_vector0 = reflect(-light0_direction, transformed_normals);" \
        "vec3 reflection_vector1 = reflect(-light1_direction, transformed_normals);" \
        "vec3 viewer_vector = normalize(-eye_coordinates.xyz);" \
        "vec3 specular0 = u_L0s * u_Ks * pow(max(dot(reflection_vector0, viewer_vector),0.0), u_material_shininess);" \
        "vec3 specular1 = u_L1s * u_Ks * pow(max(dot(reflection_vector1, viewer_vector),0.0), u_material_shininess);" \
        "phong_ads_color = ambient0 + ambient1 + diffuse0 + diffuse1 + specular0 + specular1;"
        "}" \
        "else" \
        "{"
        "phong_ads_color = vec3(1.0, 1.0, 1.0);" \
        "}"\
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
        "in vec3 phong_ads_color;"
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = vec4(phong_ads_color, 1.0);" \
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
        gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled"); //L/1 key pressed or not
        L0a_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0a");
        L0d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0d");
        L0s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0s");
        gLight0PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light0_position");
        
        //light1
        L1a_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1a");
        L1d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1d");
        L1s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1s");
        gLight1PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light1_position");
        
        //material ambient color intensity
        Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
        Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
        Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
        //shininess of material
        material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");
        
        const GLfloat pyramidVertices[] = {
            //front face
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
        
        const GLfloat pyramidNormals[] = {
            0.0f, 0.447214f, 0.894427f, //front face normals
            0.0f, 0.447214f, 0.894427f, //front face normals
            0.0f, 0.447214f, 0.894427f, //front face normals
            
            0.894427f, 0.447214f, 0.0f, //right face
            0.894427f, 0.447214f, 0.0f, //right face
            0.894427f, 0.447214f, 0.0f, //right face
            
            0.0f, 0.447214f, -0.894427f, //back face
            0.0f, 0.447214f, -0.894427f, //back face
            0.0f, 0.447214f, -0.894427f, //back face
            
            -0.894427f, 0.447214f, 0.0f //left face
            - 0.894427f, 0.447214f, 0.0f //left face
            - 0.894427f, 0.447214f, 0.0f //left face
            
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
        
        glGenBuffers(1, &gVbo_normals_pyramid);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_normals_pyramid);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
        
        glVertexAttribPointer(JCG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(JCG_ATTRIBUTE_NORMAL);
        
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);  //done with pyramid vao
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
    
    if (angle_pyramid > 360.0f) {
        angle_pyramid = 0.0f;
    }
    else
        angle_pyramid = angle_pyramid + 0.2f;
    
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
        //setting light's properties
        //light0
        glUniform3fv(L0a_uniform, 1, light0Ambient);
        glUniform3fv(L0d_uniform, 1, light0Diffuse);
        glUniform3fv(L0s_uniform, 1, light0Specular);
        glUniform4fv(gLight0PositionUniform, 1, light0Position);
        
        glUniform3fv(L1a_uniform, 1, light1Ambient);
        glUniform3fv(L1d_uniform, 1, light1Diffuse);
        glUniform3fv(L1s_uniform, 1, light1Specular);
        glUniform4fv(gLight1PositionUniform, 1, light1Position);
        
        //set material properties
        glUniform3fv(Ka_uniform, 1, material_ambient);
        glUniform3fv(Kd_uniform, 1, material_diffuse);
        glUniform3fv(Ks_uniform, 1, material_specular);
        glUniform1f(material_shininess_uniform, material_shininess);
        
        //setting materia properties
    }
    else {
        glUniform1i(gLKeyPressedUniform, 0);
    }
    
    //OpenGL drawing
    
    //set modleview and projection matrices to identity matrix
    
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();
    
    modelMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
    rotationMatrix = vmath::rotate(angle_pyramid, 0.0f, 1.0f, 0.0f);
    modelMatrix = modelMatrix * rotationMatrix;
    // Pass above matrices to shaders
    
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    //bind pyramid vao
    glBindVertexArray(gVao_pyramid);
    // Draw either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glDrawArrays(GL_TRIANGLES, 0, 12); //3 is no of positions
    
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
   
    if (gVao_pyramid) {
        glDeleteVertexArrays(1, &gVao_pyramid);
        gVao_pyramid = 0;
    }
    
    if (gVbo_position_pyramid) {
        glDeleteBuffers(1, &gVbo_position_pyramid);
        gVbo_position_pyramid = 0;
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
