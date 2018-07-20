#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"
#import "GLESView.h"
#import "Sphere.h"
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
    
    GLuint gVao_sphere;
    GLuint gVbo_normal_sphere;
    GLuint gVbo_position_sphere;
    GLuint gVbo_element_sphere;
    GLuint gMVPUniform;
    GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
    GLuint gLight0PositionUniform,  gLight1PositionUniform, gLight2PositionUniform;
    GLuint La_uniform, L0d_uniform, L0s_uniform, L1d_uniform, L1s_uniform, L2d_uniform, L2s_uniform;
    GLuint gLKeyPressedUniform;
    GLuint Ka_uniform, Kd_uniform, Ks_uniform;
    GLuint material_shininess_uniform;
    bool gbAnimate;
    bool gbLight;
    GLfloat gAngle;
    
    vmath::mat4 gPerspectiveProjectionMatrix;
    float sphere_vertices[1146];
    float sphere_normals[1146];
    float sphere_textures[764];
    unsigned short sphere_elements[2280];
    unsigned int gNumVertices, gNumElements;
    
    //lighting details
    GLfloat lightAmbient[4];
    GLfloat lightPosition[4];
    
    //light0 RED
    GLfloat light0Diffuse[4];
    GLfloat light0Specular[4];
    
    //light1 GREEN
    GLfloat light1Diffuse[4];
    GLfloat light1Specular[4];
    
    //light2 BLUE
    GLfloat light2Diffuse[4];
    GLfloat light2Specular[4];
    
    GLfloat material_ambient[4];
    GLfloat material_diffuse[4];
    GLfloat material_specular[4];
    GLfloat material_shininess;
    Sphere *sphere;
    float RADIUS;

}

- (id) initWithFrame:(CGRect)frame
{
    self=[super initWithFrame:frame];
    //Initialization
    gbLight = true;
    gbAnimate = false;
    lightAmbient[0]=lightAmbient[1]=lightAmbient[2]=lightAmbient[3]=0.0f;
    
    RADIUS=500.0f;
    
    //Light0 RED
    light0Diffuse[0]=1.0f;
    light0Diffuse[1]=light0Diffuse[2]=light0Diffuse[3]=0.0f;
    
    light0Specular[0]=1.0f;
    light0Specular[1]=light0Specular[2]=light0Specular[3]=0.0f;
    
    //Light1 GREEN
    light1Diffuse[0]=light1Diffuse[2]=light1Diffuse[3]=0.0f;
    light1Diffuse[1]=1.0f;
    
    light1Specular[0]=light1Specular[2]=light1Specular[3]=0.0f;
    light1Specular[1]=1.0f;
    
    //Light2 BLUE
    light2Diffuse[0]=light2Diffuse[1]=light2Diffuse[3]=0.0f;
    light2Diffuse[2]=1.0f;
    
    light2Specular[0]=light2Specular[1]=light2Specular[3]=0.0f;
    light1Specular[2]=1.0f;
    
    lightPosition[0]=lightPosition[1]=lightPosition[2]=lightPosition[3]=0.0f;
    
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
        "uniform vec4 u_light0_position;" \
        "uniform vec4 u_light1_position;" \
        "uniform vec4 u_light2_position;" \
        "uniform int u_lighting_enabled;" \
        "out vec3 transformed_normals_light0;" \
        "out vec3 light_direction_light0;" \
        "out vec3 viewer_vector_light0;" \
        "out vec3 transformed_normals_light1;" \
        "out vec3 light_direction_light1;" \
        "out vec3 viewer_vector_light1;" \
        "out vec3 transformed_normals_light2;" \
        "out vec3 light_direction_light2;" \
        "out vec3 viewer_vector_light2;" \
        "void main(void)" \
        "{" \
        "if(u_lighting_enabled == 1)" \
        "{" \
        "        /*light0 calculations  */       " \
        "vec4 eye_coordinates_light0 = u_view_matrix * u_model_matrix * vPosition;" \
        "transformed_normals_light0 = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
        "light_direction_light0 = vec3(u_light0_position) - eye_coordinates_light0.xyz;" \
        "viewer_vector_light0 = -eye_coordinates_light0.xyz;" \
        "        /*light1 calculations  */       " \
        "vec4 eye_coordinates_light1 = u_view_matrix * u_model_matrix * vPosition;" \
        "transformed_normals_light1 = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
        "light_direction_light1 = vec3(u_light1_position) - eye_coordinates_light1.xyz;" \
        "viewer_vector_light1 = -eye_coordinates_light1.xyz;" \
        "        /*light2 calculations  */       " \
        "vec4 eye_coordinates_light2 = u_view_matrix * u_model_matrix * vPosition;" \
        "transformed_normals_light2 = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
        "light_direction_light2 = vec3(u_light2_position) - eye_coordinates_light2.xyz;" \
        "viewer_vector_light2 = -eye_coordinates_light2.xyz;" \
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
        "in vec3 transformed_normals_light0;" \
        "in vec3 light_direction_light0;" \
        "in vec3 viewer_vector_light0;" \
        "in vec3 transformed_normals_light1;" \
        "in vec3 light_direction_light1;" \
        "in vec3 viewer_vector_light1;" \
        "in vec3 transformed_normals_light2;" \
        "in vec3 light_direction_light2;" \
        "in vec3 viewer_vector_light2;" \
        "uniform vec3 u_La;" \
        "uniform vec3 u_L0d;" \
        "uniform vec3 u_L0s;" \
        "uniform vec3 u_L1d;" \
        "uniform vec3 u_L1s;" \
        "uniform vec3 u_L2d;" \
        "uniform vec3 u_L2s;" \
        "uniform vec3 u_Ka;" \
        "uniform vec3 u_Kd;" \
        "uniform vec3 u_Ks;" \
        "uniform float u_material_shininess;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "vec3 phong_ads_color;"\
        "int u_lighting_enabled=1;" \
        "if(u_lighting_enabled == 1)" \
        "{" \
        "        /*light0 calculations  */       " \
        "vec3 normalized_transformed_normals_light0 = normalize(transformed_normals_light0);" \
        "vec3 normalized_light_direction_light0 = normalize(light_direction_light0);" \
        "vec3 normalized_viewer_vector_light0 = normalize(viewer_vector_light0);" \
        "float tn_dot_ld_light0 = max(dot(normalized_transformed_normals_light0, normalized_light_direction_light0), 0.0);" \
        "vec3 ambient = u_La * u_Ka;" \
        "vec3 diffuse_light0 = u_L0d * u_Kd * tn_dot_ld_light0;" \
        "vec3 reflection_vector_light0 = reflect(-normalized_light_direction_light0, normalized_transformed_normals_light0);" \
        "vec3 specular_light0 = u_L0s * u_Ks * pow(max(dot(reflection_vector_light0, normalized_viewer_vector_light0),0.0), u_material_shininess);" \
        "        /*light1 calculations  */       " \
        "vec3 normalized_transformed_normals_light1 = normalize(transformed_normals_light1);" \
        "vec3 normalized_light_direction_light1 = normalize(light_direction_light1);" \
        "vec3 normalized_viewer_vector_light1 = normalize(viewer_vector_light1);" \
        "float tn_dot_ld_light1 = max(dot(normalized_transformed_normals_light1, normalized_light_direction_light1), 0.0);" \
        "vec3 diffuse_light1 = u_L1d * u_Kd * tn_dot_ld_light1;" \
        "vec3 reflection_vector_light1 = reflect(-normalized_light_direction_light1, normalized_transformed_normals_light1);" \
        "vec3 specular_light1 = u_L1s * u_Ks * pow(max(dot(reflection_vector_light1, normalized_viewer_vector_light1),0.0), u_material_shininess);" \
        "        /*light2 calculations  */       " \
        "vec3 normalized_transformed_normals_light2 = normalize(transformed_normals_light2);" \
        "vec3 normalized_light_direction_light2 = normalize(light_direction_light2);" \
        "vec3 normalized_viewer_vector_light2 = normalize(viewer_vector_light2);" \
        "float tn_dot_ld_light2 = max(dot(normalized_transformed_normals_light2, normalized_light_direction_light2), 0.0);" \
        "vec3 diffuse_light2 = u_L2d * u_Kd * tn_dot_ld_light2;" \
        "vec3 reflection_vector_light2 = reflect(-normalized_light_direction_light2, normalized_transformed_normals_light2);" \
        "vec3 specular_light2 = u_L2s * u_Ks * pow(max(dot(reflection_vector_light2, normalized_viewer_vector_light2),0.0), u_material_shininess);" \
        "   /* Sum all the lights calculation to form final ads color  */"
        "phong_ads_color = ambient + ambient + ambient + diffuse_light0 + diffuse_light1 + diffuse_light2 + specular_light0 + specular_light1+ specular_light2;" \
        "}" \
        "else" \
        "{" \
        "phong_ads_color = vec3(1.0, 1.0, 1.0);" \
        "}" \
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
        gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
        gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
        gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
        gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled"); //L/1 key pressed or not
        La_uniform = glGetUniformLocation(gShaderProgramObject, "u_La"); //Common
        //material ambient color intensity
        Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
        Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
        Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
        //shininess of material
        material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");
        
        //3 lights details
        
        L0d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0d");
        L0s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L0s");
        
        L1d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1d");
        L1s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L1s");
        //ligh2 details
        
        L2d_uniform = glGetUniformLocation(gShaderProgramObject, "u_L2d");
        L2s_uniform = glGetUniformLocation(gShaderProgramObject, "u_L2s");
        
        
        gLight0PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light0_position");
        gLight1PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light1_position");
        gLight2PositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light2_position");
        
        // Vertices, colors, shader attribs, vbo, vao initializations
        sphere = new Sphere();
        sphere->getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        gNumVertices = sphere->getNumberOfSphereVertices();
        gNumElements = sphere->getNumberOfSphereElements();
        
        glGenVertexArrays(1, &gVao_sphere);
        glBindVertexArray(gVao_sphere);
        //set square position
        glGenBuffers(1, &gVbo_position_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_position_sphere);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(JCG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(JCG_ATTRIBUTE_VERTEX);
        
        glGenBuffers(1, &gVbo_normal_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, gVbo_normal_sphere);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
        
        glVertexAttribPointer(JCG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(JCG_ATTRIBUTE_NORMAL);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); //done with cube vao
        
        // element vbo
        glGenBuffers(1, &gVbo_element_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_element_sphere);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);        // set-up depth buffer
        
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
    
    if (gAngle> 360.0f) {
        gAngle= 0.0f;
    }
    else
        gAngle= gAngle+ 0.2f;
    
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
    
    if (gbLight == true) {
        glUniform1i(gLKeyPressedUniform, 1);
        //setting light's properties
        glUniform3fv(La_uniform, 1, lightAmbient);
        glUniform3fv(L0d_uniform, 1, light0Diffuse);
        glUniform3fv(L0s_uniform, 1, light0Specular);
        //light1
        glUniform3fv(L1d_uniform, 1, light1Diffuse);
        glUniform3fv(L1s_uniform, 1, light1Specular);
        //light2
        glUniform3fv(L2d_uniform, 1, light2Diffuse);
        glUniform3fv(L2s_uniform, 1, light2Specular);
        
        //light0
        lightPosition[1] = RADIUS * (float)cos(gAngle);
        lightPosition[2] = RADIUS * (float)sin(gAngle);
        glUniform4fv(gLight0PositionUniform, 1, lightPosition);
        lightPosition[1] = 0.0f;
        lightPosition[2] = 0.0f;
        
        //light1
        lightPosition[0] = RADIUS * (float)cos(gAngle);
        lightPosition[2] = RADIUS * (float)sin(gAngle);
        glUniform4fv(gLight1PositionUniform, 1, lightPosition);
        glUniform4fv(gLight2PositionUniform, 1, lightPosition);
        lightPosition[0] = 0.0f;
        lightPosition[2] = 0.0f;
        
        //light2
        lightPosition[0] = RADIUS * (float)cos(gAngle);
        lightPosition[1] = RADIUS * (float)sin(gAngle);
        glUniform4fv(gLight2PositionUniform, 1, lightPosition);
        lightPosition[0] = 0.0f;
        lightPosition[1] = 0.0f;
        
        
        //set material properties
        glUniform3fv(Ka_uniform, 1, material_ambient);
        glUniform3fv(Kd_uniform, 1, material_diffuse);
        glUniform3fv(Ks_uniform, 1, material_specular);
        glUniform1f(material_shininess_uniform, material_shininess);
        
    }
    else {
        glUniform1i(gLKeyPressedUniform, 0);
    }
    
    //OpenGL drawing
    
    //set modleview and projection matrices to identity matrix
    
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    
    modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    
    
    // Pass above matrices to shaders
    
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
    
    //bind cube vao
    glBindVertexArray(gVao_sphere);
    
    
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_element_sphere);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    // *** unbind vao ***
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
   
    if (gVao_sphere) {
        glDeleteVertexArrays(1, &gVao_sphere);
        gVao_sphere = 0;
    }
    
    
    if (gVbo_normal_sphere) {
        glDeleteBuffers(1, &gVbo_normal_sphere);
        gVbo_normal_sphere = 0;
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
