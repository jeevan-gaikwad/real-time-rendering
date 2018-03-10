package com.jeevan.three_lights_sphere;

import android.content.Context;
import android.graphics.Color;
import android.widget.TextView;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

//For OpenGLES
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;
import android.opengl.GLES30; 			//My phone Moto G2 supports OpenGLES 3.0
import android.opengl.Matrix;

//Java nio(non blocking i/o
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
//A view for OpenGLES3 graphics which also receives touch events

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
	private final Context context;
	private GestureDetector gestureDetector;
	
	//new class members
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;
	
	private boolean  gbOnSingleTapUpAnimation=false;
	private float   perspectiveProjectionMatrix[]=new float[16];
	private float angle;
	private float zoomZ = -2.0f;
	
	//sphere loading
	private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];
	private int numVertices, numElements;
	
	//lighting details
	private float lightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	private float lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f };

	//Red color light(light0)
	private float light0Diffuse[] = { 1.0f,0.0f,0.0f,0.0f };
	private float light0Specular[] = { 1.0f, 0.0f, 0.0f, 0.0f };

	//Green color light(light1)
	private float light1Diffuse[] = { 0.0f,1.0f,0.0f,0.0f };
	private float light1Specular[] = { 0.0f, 1.0f, 0.0f, 0.0f };

	//Blue color light(light2)
	private float light2Diffuse[] = { 0.0f,0.0f,1.0f,0.0f };
	private float light2Specular[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	private float light2Position[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	private float material_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	private float material_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	private float material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	private float material_shininess = 50.0f;
	private int gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
	private int gModelLight0MatrixUniform; //light0
	private int gViewLight0MatrixUniform;
	private int gModelLight1MatrixUniform; //light1
	private int gModelLight2MatrixUniform; //light2
	private int gLightPositionUniform, gLight2PositionUniform;
	private int gSingleTapUniform;
	private int La_uniform, L0d_uniform, L0s_uniform, L1d_uniform, L1s_uniform, L2d_uniform, L2s_uniform;
	private int Ka_uniform, Kd_uniform, Ks_uniform;
	private int material_shininess_uniform;
	private boolean gbLight;
	
	
	public GLESView(Context context){
			super(context);
			this.context = context;
			//OpenGLES version negotiation step. Set EGLContext to current supported version of OpenGL-ES
			setEGLContextClientVersion(3);
			//set renderer for drawing on the GLSurfaceView
			setRenderer(this);
			setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY); //on screen rendering
			
			gestureDetector = new GestureDetector(context, this, null, false);
			gestureDetector.setOnDoubleTapListener(this);
	}
	//Overriden method of GLSurfaceView.Renderer(init code)
	@Override
	public void onSurfaceCreated(GL10 gl,EGLConfig config){
		//OpenGL-ES version check
		String glesVersion = gl.glGetString(GL10.GL_VERSION);
		System.out.println("JCG:" + glesVersion);
		
		String glslVersion = gl.glGetString(GLES30.GL_SHADING_LANGUAGE_VERSION);
		System.out.println("JCG: Shading lang version:"+ glslVersion);
		
		initialize(gl);
	}
	@Override
	public void onSurfaceChanged(GL10 unused, int width, int height) //like resize
    {
			resize(width,height);
	}

	@Override
	public void onDrawFrame(GL10 unused){
		draw();
	}
	@Override
	public boolean onTouchEvent(MotionEvent event){
			int eventAction = event.getAction(); 
			if(!gestureDetector.onTouchEvent(event)){
				super.onTouchEvent(event);			
			}
			return true;
	}
	@Override //abstract method from OnDoubleTapListener
	public boolean onDoubleTap(MotionEvent e){		
	
		return true;
	}
	@Override //abstract method from OnDoubleTapListener
	public boolean onDoubleTapEvent(MotionEvent e){
		//nothing to do for now. Already handled in onDoubleTap
		return true;
	}
	
	@Override //abstract method from OnDoubleTapListener
	public boolean onSingleTapConfirmed(MotionEvent e){
		if(gbLight)
			gbLight=false;
		else
			gbLight=true;
		return true;
	}
	
	@Override //abstract method from OnGestureListener
	public boolean onDown(MotionEvent e){
		//already handled in onSingleTapConfirmed
		return true;
	}
	
	@Override //abstract method from OnGestureListener
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY){		
			return true;
	}
	
	@Override //method from OnGestureListener
	public void onLongPress(MotionEvent e){		
			//return true;
	}
	
	@Override //method from OnGestureListener
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY){	
		uninitialize();
		System.out.println("JCG:on scroll exit");
		System.exit(0);
		return true;
	}
	
	@Override //method from OnGestureListener
	public void onShowPress(MotionEvent e){
		//nothing to do
	}
	
	@Override //method from OnGestureListener
	public boolean onSingleTapUp(MotionEvent e){
		return true;
	}
	
	private void initialize(GL10 gl){
		//***** VERTEX SHADER ****
		vertexShaderObject = GLES30.glCreateShader(GLES30.GL_VERTEX_SHADER);
		
		//vertex shader source code
		final String vertexShaderSourceCode =  String.format(
         "#version 300 es" +
		"\n" +
		"in vec4 vPosition;" +
		"in vec3 vNormal;" +
		"uniform mat4 u_model_matrix;" +
		"uniform mat4 u_view_matrix;" +
		"uniform mat4 u_view_light0_matrix;" +
		"uniform mat4 u_projection_matrix;" +
		"uniform mat4 u_model_light0_matrix;" +
		"uniform mat4 u_model_light1_matrix;" +
		"uniform mat4 u_model_light2_matrix;" +
		"uniform vec4 u_light_position;" +
		"uniform vec4 u_light2_position;" +
		"uniform int u_lighting_enabled;" +
		"out vec3 transformed_normals_light0;" +
		"out vec3 light_direction_light0;" +
		"out vec3 viewer_vector_light0;" +
		"out vec3 transformed_normals_light1;" +
		"out vec3 light_direction_light1;" +
		"out vec3 viewer_vector_light1;" +
		"out vec3 transformed_normals_light2;" +
		"out vec3 light_direction_light2;" +
		"out vec3 viewer_vector_light2;" +
		"void main(void)" +
		"{" +
		"if(u_lighting_enabled == 1)" +
		"{" +
		"        /*light0 calculations  */       " +
		"vec4 eye_coordinates_light0 = u_view_light0_matrix * u_model_light0_matrix * vPosition;" +
		"transformed_normals_light0 = mat3(u_view_light0_matrix * u_model_light0_matrix) * vNormal;" +
		"light_direction_light0 = vec3(u_light_position) - eye_coordinates_light0.xyz;" +
		"viewer_vector_light0 = -eye_coordinates_light0.xyz;" +		
		"        /*light1 calculations  */       " +
		"vec4 eye_coordinates_light1 = u_view_matrix * u_model_light1_matrix * vPosition;" +
		"transformed_normals_light1 = mat3(u_view_matrix * u_model_light1_matrix) * vNormal;" +
		"light_direction_light1 = vec3(u_light_position) - eye_coordinates_light1.xyz;" +
		"viewer_vector_light1 = -eye_coordinates_light1.xyz;" +
		"        /*light2 calculations  */       " +
		"vec4 eye_coordinates_light2 = u_view_matrix * u_model_light2_matrix * vPosition;" +
		"transformed_normals_light2 = mat3(u_view_matrix * u_model_light2_matrix) * vNormal;" +
		"light_direction_light2 = vec3(u_light2_position) - eye_coordinates_light2.xyz;" +
		"viewer_vector_light2 = -eye_coordinates_light2.xyz;" +
		"}" +
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
		"}"
		); //source code string ends
		
		//provide above source code to shader object
		GLES30.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
		
		GLES30.glCompileShader(vertexShaderObject);
		//error checking
		int[] iShaderCompiledStatus = new int[1]; //taken as array, bcz this will be a out param
		int[] iInfoLogLength = new int[1];
		String szInfoLog = null;
		GLES30.glGetShaderiv(vertexShaderObject, GLES30.GL_COMPILE_STATUS, iShaderCompiledStatus, 0); //note additional zero
		if(iShaderCompiledStatus[0] == GLES30.GL_FALSE){
			GLES30.glGetShaderiv(vertexShaderObject, GLES30.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if(iInfoLogLength[0] > 0){
				szInfoLog = GLES30.glGetShaderInfoLog(vertexShaderObject);
				System.out.println("JCG: Vertex shader compilation log:" + szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		
		//***** FRAGMENT SHADER ****
		//create fragment shader
		fragmentShaderObject = GLES30.glCreateShader(GLES30.GL_FRAGMENT_SHADER);
		//fragment shader source code
		final String fragmentShaderSourceCode = String.format(
		"#version 300 es"+
		"\n"+
		"precision highp float;"+
		"in vec3 transformed_normals_light0;" +
		"in vec3 light_direction_light0;" +
		"in vec3 viewer_vector_light0;" +
		"in vec3 transformed_normals_light1;" +
		"in vec3 light_direction_light1;" +
		"in vec3 viewer_vector_light1;" +
		"in vec3 transformed_normals_light2;" +
		"in vec3 light_direction_light2;" +
		"in vec3 viewer_vector_light2;" +
		"uniform vec3 u_La;" +
		"uniform vec3 u_L0d;" +
		"uniform vec3 u_L0s;" +
		"uniform vec3 u_L1d;" +
		"uniform vec3 u_L1s;" +
		"uniform vec3 u_L2d;" +
		"uniform vec3 u_L2s;" +
		"uniform vec3 u_Ka;" +
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_Ks;" +
		"uniform float u_material_shininess;" +
		"uniform int u_lighting_enabled;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"vec3 phong_ads_color;"+
		"if(u_lighting_enabled == 1)" +
		"{" +
		"        /*light0 calculations  */       " +
		"vec3 normalized_transformed_normals_light0 = normalize(transformed_normals_light0);" +
		"vec3 normalized_light_direction_light0 = normalize(light_direction_light0);" +
		"vec3 normalized_viewer_vector_light0 = normalize(viewer_vector_light0);" +
		"float tn_dot_ld_light0 = max(dot(normalized_transformed_normals_light0, normalized_light_direction_light0), 0.0);" +
		"vec3 ambient = u_La * u_Ka;" +
		"vec3 diffuse_light0 = u_L0d * u_Kd * tn_dot_ld_light0;" +
		"vec3 reflection_vector_light0 = reflect(-normalized_light_direction_light0, normalized_transformed_normals_light0);" +
		"vec3 specular_light0 = u_L0s * u_Ks * pow(max(dot(reflection_vector_light0, normalized_viewer_vector_light0),0.0), u_material_shininess);" +
		"        /*light1 calculations  */       " +
		"vec3 normalized_transformed_normals_light1 = normalize(transformed_normals_light1);" +
		"vec3 normalized_light_direction_light1 = normalize(light_direction_light1);" +
		"vec3 normalized_viewer_vector_light1 = normalize(viewer_vector_light1);" +
		"float tn_dot_ld_light1 = max(dot(normalized_transformed_normals_light1, normalized_light_direction_light1), 0.0);" +
		"vec3 diffuse_light1 = u_L1d * u_Kd * tn_dot_ld_light1;" +
		"vec3 reflection_vector_light1 = reflect(-normalized_light_direction_light1, normalized_transformed_normals_light1);" +
		"vec3 specular_light1 = u_L1s * u_Ks * pow(max(dot(reflection_vector_light1, normalized_viewer_vector_light1),0.0), u_material_shininess);" +
		"        /*light2 calculations  */       " +
		"vec3 normalized_transformed_normals_light2 = normalize(transformed_normals_light2);" +
		"vec3 normalized_light_direction_light2 = normalize(light_direction_light2);" +
		"vec3 normalized_viewer_vector_light2 = normalize(viewer_vector_light2);" +
		"float tn_dot_ld_light2 = max(dot(normalized_transformed_normals_light2, normalized_light_direction_light2), 0.0);" +
		"vec3 diffuse_light2 = u_L2d * u_Kd * tn_dot_ld_light2;" +
		"vec3 reflection_vector_light2 = reflect(-normalized_light_direction_light2, normalized_transformed_normals_light2);" +
		"vec3 specular_light2 = u_L2s * u_Ks * pow(max(dot(reflection_vector_light2, normalized_viewer_vector_light2),0.0), u_material_shininess);" +
		"   /* Sum all the lights calculation to form final ads color  */" +
		"phong_ads_color = ambient + ambient + ambient + diffuse_light0 + diffuse_light1 + diffuse_light2 +specular_light0 + specular_light1 + specular_light2;" +
		"}" +
		"else" +
		"{" +
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" +
		"}" +
		"FragColor = vec4(phong_ads_color, 1.0);" +
		"}"
		);
		
		//Provide fragment shader source code to shader
		GLES30.glShaderSource(fragmentShaderObject,fragmentShaderSourceCode);
		//check compilation erros in fragment shaders
		GLES30.glCompileShader(fragmentShaderObject);
		//re-initialize variables
		iShaderCompiledStatus[0] = 0;
		iInfoLogLength[0] = 0;
		szInfoLog = null;
		GLES30.glGetShaderiv(fragmentShaderObject, GLES30.GL_COMPILE_STATUS, iShaderCompiledStatus, 0); //note additional zero
		if(iShaderCompiledStatus[0] == GLES30.GL_FALSE){
			GLES30.glGetShaderiv(fragmentShaderObject, GLES30.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if(iInfoLogLength[0] > 0){
				szInfoLog = GLES30.glGetShaderInfoLog(fragmentShaderObject);
				System.out.println("JCG: Fragment shader compilation log:" + szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		
		shaderProgramObject = GLES30.glCreateProgram();
		GLES30.glAttachShader(shaderProgramObject, vertexShaderObject);
		GLES30.glAttachShader(shaderProgramObject, fragmentShaderObject);
		
		//pre-link binding of shader program object with vertex shader attribute
		GLES30.glBindAttribLocation(shaderProgramObject, GLESMacros.JCG_ATTRIBUTE_VERTEX, "vPosition");
		GLES30.glBindAttribLocation(shaderProgramObject, GLESMacros.JCG_ATTRIBUTE_NORMAL, "vNormal");
		//link program
		GLES30.glLinkProgram(shaderProgramObject);
		int[] iShaderProgramLinkStatus = new int[1]; //taken as array, bcz this will be a out param
		iInfoLogLength[0] = 0;
		szInfoLog = null;
		GLES30.glGetProgramiv(shaderProgramObject, GLES30.GL_LINK_STATUS, iShaderProgramLinkStatus, 0); //note additional zero
		if(iShaderProgramLinkStatus[0] == GLES30.GL_FALSE){
			GLES30.glGetProgramiv(shaderProgramObject, GLES30.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
			if(iInfoLogLength[0] > 0){
				szInfoLog = GLES30.glGetProgramInfoLog(shaderProgramObject);
				System.out.println("JCG: Shader program link log:" + szInfoLog);
				uninitialize();
				System.exit(0);
			}
		}
		
		//Preparation to put our dynamic(uniform) data into the shader
		gModelMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_model_matrix");
		gViewMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_view_matrix");
		gProjectionMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
		gSingleTapUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_lighting_enabled"); //L/1 key pressed or not
		La_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_La");
		
		//material ambient color intensity
		Ka_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Ka");
		Kd_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Kd");
		Ks_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Ks");
		//shininess of material
		material_shininess_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_material_shininess");

		//light animation
		//light 0 details
		gModelLight0MatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_model_light0_matrix");
		gViewLight0MatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_view_light0_matrix");
		
		L0d_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L0d");
		L0s_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L0s");
		//light1 details
		gModelLight1MatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_model_light1_matrix");
		L1d_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L1d");
		L1s_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L1s");

		//ligh2 details
		gModelLight2MatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_model_light2_matrix");
		L2d_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L2d");
		L2s_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L2s");
		gLight2PositionUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_light2_position");

		gLightPositionUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_light_position");

		//DATA: vertices, colors, shader attribs, vao, vbos initialization
		
		Sphere sphere=new Sphere();
        float sphere_vertices[]=new float[1146];
        float sphere_normals[]=new float[1146];
        float sphere_textures[]=new float[764];
        short sphere_elements[]=new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

        // vao
        GLES30.glGenVertexArrays(1,vao_sphere,0);
        GLES30.glBindVertexArray(vao_sphere[0]);
        
        // position vbo
        GLES30.glGenBuffers(1,vbo_sphere_position,0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER,vbo_sphere_position[0]);
        
        ByteBuffer byteBuffer=ByteBuffer.allocateDirect(sphere_vertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_vertices);
        verticesBuffer.position(0);
        
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                            sphere_vertices.length * 4,
                            verticesBuffer,
                            GLES30.GL_STATIC_DRAW);
        
        GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_VERTEX,
                                     3,
                                     GLES30.GL_FLOAT,
                                     false,0,0);
        
        GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_VERTEX);
        
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER,0);
        
        // normal vbo
        GLES30.glGenBuffers(1,vbo_sphere_normal,0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER,vbo_sphere_normal[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_normals.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_normals);
        verticesBuffer.position(0);
        
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER,
                            sphere_normals.length * 4,
                            verticesBuffer,
                            GLES30.GL_STATIC_DRAW);
        
        GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_NORMAL,
                                     3,
                                     GLES30.GL_FLOAT,
                                     false,0,0);
        
        GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_NORMAL);
        
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER,0);
        
        // element vbo
        GLES30.glGenBuffers(1,vbo_sphere_element,0);
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer elementsBuffer=byteBuffer.asShortBuffer();
        elementsBuffer.put(sphere_elements);
        elementsBuffer.position(0);
        
        GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER,
                            sphere_elements.length * 2,
                            elementsBuffer,
                            GLES30.GL_STATIC_DRAW);
        
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER,0);

        GLES30.glBindVertexArray(0);
		
		//enable depth testing
		GLES30.glEnable(GLES30.GL_DEPTH_TEST);
		GLES30.glDepthFunc(GLES30.GL_LEQUAL);
		GLES30.glEnable(GLES30.GL_CULL_FACE); //No culling for animation
		GLES30.glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //black
		
		//set projection matrix to identity matrix
		Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
		
	}
	
	private void resize(int width, int height){
		GLES30.glViewport(0,0,width,height);
		
		
		//perspective projection
		Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f,((float)width/(float)height),0.1f,100.0f);
	}
	
	public void draw(){
		GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT|GLES30.GL_DEPTH_BUFFER_BIT);
		//use shader program
		GLES30.glUseProgram(shaderProgramObject);
		
		if (gbLight == true) {
			GLES30.glUniform1i(gSingleTapUniform, 1);	
			//setting light's properties
			GLES30.glUniform3fv(La_uniform, 1, lightAmbient, 0);
			GLES30.glUniform3fv(L0d_uniform, 1, light0Diffuse, 0);
			GLES30.glUniform3fv(L0s_uniform, 1, light0Specular, 0);
			//light1
			GLES30.glUniform3fv(L1d_uniform, 1, light1Diffuse, 0);
			GLES30.glUniform3fv(L1s_uniform, 1, light1Specular, 0);
			//light2
			GLES30.glUniform3fv(L2d_uniform, 1, light2Diffuse, 0);
			GLES30.glUniform3fv(L2s_uniform, 1, light2Specular, 0);
			//specify unique light position to all lights
			GLES30.glUniform4fv(gLightPositionUniform, 1, lightPosition, 0);

			//set material properties
			GLES30.glUniform3fv(Ka_uniform, 1, material_ambient, 0);
			GLES30.glUniform3fv(Kd_uniform, 1, material_diffuse, 0);
			GLES30.glUniform3fv(Ks_uniform, 1, material_specular, 0);
			GLES30.glUniform1f(material_shininess_uniform, material_shininess);
			
			//light animation
			float modelLightMatrix[] = new float[16];
			Matrix.setIdentityM(modelLightMatrix, 0);

			float viewLightMatrix[] =  new float[16];
			Matrix.setIdentityM(viewLightMatrix, 0);
			
			float rotationLightMatrix[] = new float[16];
			Matrix.setIdentityM(rotationLightMatrix, 0);
			float rotationLight1Matrix[] = new float[16];
			Matrix.setIdentityM(rotationLight1Matrix, 0);
		
			Matrix.translateM(modelLightMatrix, 0, 0.0f, 0.0f, zoomZ);
			Matrix.rotateM(rotationLightMatrix, 0, angle , 1.0f, 0.0f, 0.0f);
			Matrix.multiplyMM(modelLightMatrix, 0, modelLightMatrix, 0,rotationLightMatrix,0);
			
			GLES30.glUniformMatrix4fv(gModelLight0MatrixUniform, 1, false, modelLightMatrix, 0); //light0
			GLES30.glUniformMatrix4fv(gViewLight0MatrixUniform, 1, false, viewLightMatrix, 0); //light0
			
			//light1
			Matrix.setIdentityM(modelLightMatrix, 0);
			Matrix.setIdentityM(rotationLightMatrix, 0);
			Matrix.translateM(modelLightMatrix, 0, 0.0f, 0.0f, zoomZ);
			Matrix.rotateM(rotationLightMatrix, 0, angle , 0.0f, 1.0f, 0.0f);
			Matrix.multiplyMM(modelLightMatrix, 0, modelLightMatrix, 0,rotationLightMatrix,0);
			GLES30.glUniformMatrix4fv(gModelLight1MatrixUniform, 1, false, modelLightMatrix, 0); //light1
			
			//light2
			Matrix.setIdentityM(modelLightMatrix, 0);
			Matrix.setIdentityM(rotationLightMatrix, 0);
			Matrix.translateM(modelLightMatrix, 0, 0.0f, 0.0f, zoomZ);
			Matrix.rotateM(rotationLightMatrix, 0, angle , 0.0f, 0.0f, 1.0f);
			light2Position[0] = 360;
			GLES30.glUniform4fv(gLight2PositionUniform, 1, light2Position , 0);
			Matrix.multiplyMM(modelLightMatrix, 0, modelLightMatrix, 0,rotationLightMatrix,0);
			GLES30.glUniformMatrix4fv(gModelLight2MatrixUniform, 1, false, modelLightMatrix, 0); //light2
			
			spin(); //change angle of rotation
		}else{
			GLES30.glUniform1i(gSingleTapUniform, 0);
		}
		
		float modelMatrix[] = new float[16];
		float viewMatrix[] = new float [16];
		float rotationMatrix[] = new float [16];
		//set modelview and modelview projection matrix to identity
		Matrix.setIdentityM(modelMatrix, 0);
		Matrix.setIdentityM(viewMatrix, 0);
		//Matrix.setIdentityM(rotationMatrix, 0);
		
		//multiply modelview and projection matrix to get modelViewProjection matrix
		Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, zoomZ);
		
		//pass model View matrix to shader
		
		GLES30.glUniformMatrix4fv(gModelMatrixUniform, 1, false, modelMatrix, 0);
		GLES30.glUniformMatrix4fv(gViewMatrixUniform, 1, false, viewMatrix, 0);
		
		//pass projection matrix to shader
		
		GLES30.glUniformMatrix4fv(gProjectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);
		
		
		//bind vao. Start playing
		
		
		// bind vao
        GLES30.glBindVertexArray(vao_sphere[0]);
        
        // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
        GLES30.glDrawElements(GLES30.GL_TRIANGLES, numElements, GLES30.GL_UNSIGNED_SHORT, 0);
        
        // unbind vao`
        GLES30.glBindVertexArray(0);
	
		
		
		GLES30.glUseProgram(0);
		

		
		//SwapBuffers 
		requestRender();
	}
	void spin(){
		angle =  angle + 0.5f;
		if (angle >= 360.0f) {
			angle = angle - 360.0f;
		}
	}
	void uninitialize(){
		//destroy vao
		
		
		// destroy vao
        if(vao_sphere[0] != 0)
        {
            GLES30.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0]=0;
        }
        
        // destroy position vbo
        if(vbo_sphere_position[0] != 0)
        {
            GLES30.glDeleteBuffers(1, vbo_sphere_position, 0);
            vbo_sphere_position[0]=0;
        }
        
        // destroy normal vbo
        if(vbo_sphere_normal[0] != 0)
        {
            GLES30.glDeleteBuffers(1, vbo_sphere_normal, 0);
            vbo_sphere_normal[0]=0;
        }
        
        // destroy element vbo
        if(vbo_sphere_element[0] != 0)
        {
            GLES30.glDeleteBuffers(1, vbo_sphere_element, 0);
            vbo_sphere_element[0]=0;
        }
		
		if(shaderProgramObject != 0){
			if(vertexShaderObject != 0){
				//detach first then delete
				GLES30.glDetachShader(shaderProgramObject, vertexShaderObject);
				GLES30.glDeleteShader(vertexShaderObject);
				vertexShaderObject=0;
			}
			if(fragmentShaderObject != 0){
				//detach first then delete
				GLES30.glDetachShader(shaderProgramObject, fragmentShaderObject);
				GLES30.glDeleteShader(fragmentShaderObject);
				fragmentShaderObject=0;
			}
			if( shaderProgramObject != 0 ){
				GLES30.glDeleteProgram(shaderProgramObject);
				shaderProgramObject = 0;
			}
		}
	}
	
	
}