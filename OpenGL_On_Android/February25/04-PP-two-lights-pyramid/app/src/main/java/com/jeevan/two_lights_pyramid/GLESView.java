package com.jeevan.two_lights_pyramid;

import android.content.Context;
import android.graphics.Color;
import android.widget.TextView;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;
import android.view.ScaleGestureDetector; 
import android.view.ScaleGestureDetector.OnScaleGestureListener;

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

//A view for OpenGLES3 graphics which also receives touch events

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
	private final Context context;
	private GestureDetector gestureDetector;
	
	//new class memebers
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;
	
	private int[] vao_pyramid = new int[1];
	private int[] vbo_position = new int[1];
	private int[] vbo_normals = new int[1];

	private float perspectiveProjectionMatrix[]=new float[16];
	private float angle_pyramid;
	private float zoomZ = -5.0f;
	//lighting details
	private float light0Ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	private float light0Diffuse[] = { 1.0f,0.0f,0.0f,0.0f };
	private float light0Specular[] = { 1.0f, 0.0f, 0.0f, 0.0f };
	private float light0Position[] = { 2.0f, 2.0f, 0.0f, 1.0f };

	private float light1Ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	private float light1Diffuse[] = { 0.0f,0.0f,1.0f,0.0f };
	private float light1Specular[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	private float light1Position[] = { -2.0f, 2.0f, 0.0f, 1.0f };

	private float material_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	private float material_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	private float material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	private float material_shininess = 50.0f;
	private int gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
	private int gLight0PositionUniform, gLight1PositionUniform;
	private int gSingleTapUniform;
	private int L0a_uniform, L0d_uniform, L0s_uniform; //light0
	private int L1a_uniform, L1d_uniform, L1s_uniform; //light1
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
			
	}
	
	@Override //method from OnGestureListener
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY){	
		uninitialize();
		System.out.println("JCG:on scroll exit");
		//System.exit(0);
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
		"uniform mat4 u_projection_matrix;" +
		"uniform int u_lighting_enabled;" +
		"uniform vec3 u_L0a;" +
		"uniform vec3 u_L0d;" +
		"uniform vec3 u_L0s;" +
		"uniform vec4 u_light0_position;" +
		"uniform vec3 u_L1a;" +
		"uniform vec3 u_L1d;" +
		"uniform vec3 u_L1s;" +
		"uniform vec4 u_light1_position;" +
		"uniform vec3 u_Ka;" +
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_Ks;" +
		"uniform float u_material_shininess;" +
		"out vec3 phong_ads_color;" +
		"void calculate_light_ads(vec3 La,vec3 Ld, vec3 Ls)"+
		"{" +
		"}"+
		"void main(void)" +
		"{" +
		"if(u_lighting_enabled == 1)" +
		"{"+
		"vec4 eye_coordinates = u_view_matrix* u_model_matrix * vPosition;" +
		"vec3 transformed_normals = normalize(mat3(u_view_matrix*u_model_matrix) * vNormal);" +
		"vec3 light0_direction = normalize(vec3(u_light0_position) - eye_coordinates.xyz);" +
		"vec3 light1_direction = normalize(vec3(u_light1_position) - eye_coordinates.xyz);" +
		"float tn_dot_ld0 = max(dot(transformed_normals, light0_direction), 0.0);" +
		"float tn_dot_ld1 = max(dot(transformed_normals, light1_direction), 0.0);" +
		"vec3 ambient0 = u_L0a * u_Ka;" +
		"vec3 ambient1 = u_L1a * u_Ka;" +
		"vec3 diffuse0 = u_L0d * u_Kd * tn_dot_ld0;" +
		"vec3 diffuse1 = u_L1d * u_Kd * tn_dot_ld1;" +
		"vec3 reflection_vector0 = reflect(-light0_direction, transformed_normals);" +
		"vec3 reflection_vector1 = reflect(-light1_direction, transformed_normals);" +
		"vec3 viewer_vector = normalize(-eye_coordinates.xyz);" +
		"vec3 specular0 = u_L0s * u_Ks * pow(max(dot(reflection_vector0, viewer_vector),0.0), u_material_shininess);" +
		"vec3 specular1 = u_L1s * u_Ks * pow(max(dot(reflection_vector1, viewer_vector),0.0), u_material_shininess);" +
		"phong_ads_color = ambient0 + ambient1 + diffuse0 + diffuse1 + specular0 + specular1;" +
		"}" +
		"else" +
		"{" +
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" +
		"}"+
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
		"in vec3 phong_ads_color;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
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
		
		//get MVP uniform location
		
		gModelMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_model_matrix");
		gViewMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_view_matrix");
		gProjectionMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
		gSingleTapUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_lighting_enabled"); 
		L0a_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L0a");
		L0d_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L0d");
		L0s_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L0s");
		gLight0PositionUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_light0_position");

		//light1
		L1a_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L1a");
		L1d_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L1d");
		L1s_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_L1s");
		gLight1PositionUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_light1_position");

		//material ambient color intensity
		Ka_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Ka");
		Kd_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Kd");
		Ks_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Ks");
		//shininess of material
		material_shininess_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_material_shininess");
		//DATA: vertices, colors, shader attribs, vao, vbos initialization
		final float pyramidVertices[] = new float[]{
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
		
		final float pyramidNormals[] = new float[] {
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
		
		GLES30.glGenVertexArrays(1, vao_pyramid, 0); 	//NOTE additional zero
		GLES30.glBindVertexArray(vao_pyramid[0]);		//NOTE how it is used
		
		//vbo for positions
		GLES30.glGenBuffers(1, vbo_position, 0); 		//Creates a vbo. 
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_position[0]);
		
		//Create a buffer to pass our float data to GPU in native fashion
		ByteBuffer byteBuffer =  ByteBuffer.allocateDirect(pyramidVertices.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(pyramidVertices); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, pyramidVertices.length * 4, verticesBuffer, GLES30.GL_STATIC_DRAW);
		
		GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_VERTEX, 3, GLES30.GL_FLOAT, false, 0, 0);
		
		GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_VERTEX);
		
		//vbo for normals
		GLES30.glGenBuffers(1, vbo_normals, 0); 		//Creates a vbo. 
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_normals[0]);
		
		//Create a buffer to pass our float data to GPU in native fashion
		byteBuffer =  ByteBuffer.allocateDirect(pyramidNormals.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(pyramidNormals); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, pyramidNormals.length * 4, verticesBuffer, GLES30.GL_STATIC_DRAW);
		
		GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_NORMAL, 3, GLES30.GL_FLOAT, false, 0, 0);
		
		GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_NORMAL);
		
		//done with vaos
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);
		GLES30.glBindVertexArray(0);
		
		//enable depth testing
		GLES30.glEnable(GLES30.GL_DEPTH_TEST);
		GLES30.glDepthFunc(GLES30.GL_LEQUAL);
		//GLES30.glEnable(GLES30.GL_CULL_FACE); //No culling for animation
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
				//light0
				GLES30.glUniform3fv(L0a_uniform, 1, light0Ambient, 0);
				GLES30.glUniform3fv(L0d_uniform, 1, light0Diffuse, 0);
				GLES30.glUniform3fv(L0s_uniform, 1, light0Specular, 0);
				GLES30.glUniform4fv(gLight0PositionUniform, 1, light0Position, 0);

				GLES30.glUniform3fv(L1a_uniform, 1, light1Ambient, 0);
				GLES30.glUniform3fv(L1d_uniform, 1, light1Diffuse, 0);
				GLES30.glUniform3fv(L1s_uniform, 1, light1Specular, 0);
				GLES30.glUniform4fv(gLight1PositionUniform, 1, light1Position, 0);

				//set material properties
				GLES30.glUniform3fv(Ka_uniform, 1, material_ambient, 0);
				GLES30.glUniform3fv(Kd_uniform, 1, material_diffuse, 0);
				GLES30.glUniform3fv(Ks_uniform, 1, material_specular, 0);
				GLES30.glUniform1f(material_shininess_uniform, material_shininess);

		}
		else {
			GLES30.glUniform1i(gSingleTapUniform, 0);
		}

		
		
		float modelMatrix[] = new float[16];
		float viewMatrix[] = new float[16];
		
		//set modelview and modelview projection matrix to identity
		Matrix.setIdentityM(modelMatrix, 0);
		Matrix.setIdentityM(viewMatrix, 0);
		
		//multiply modelview and projection matrix to get modelViewProjection matrix
		Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, zoomZ);
		Matrix.rotateM(modelMatrix, 0, angle_pyramid , 0.0f, 1.0f, 0.0f);
		
		//pass above matrix to u_mvp_matrix
		
		GLES30.glUniformMatrix4fv(gModelMatrixUniform, 1, false, modelMatrix, 0);
		GLES30.glUniformMatrix4fv(gViewMatrixUniform, 1, false, viewMatrix, 0);
		GLES30.glUniformMatrix4fv(gProjectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);
		
		//bind vao. Start playing
		GLES30.glBindVertexArray(vao_pyramid[0]);
		
		//draw using glDrawArrays
		GLES30.glDrawArrays(GLES30.GL_TRIANGLES, 0, 12);
		
		GLES30.glBindVertexArray(0);
		
		GLES30.glUseProgram(0);
		
		update(); //change angle of rotation
		//SwapBuffers 
		requestRender();
	}
	void update(){
		if(angle_pyramid > 360.0){
			angle_pyramid = 0.0f;			
		}else{
			angle_pyramid = angle_pyramid + 1.0f;
		}
	}
	void uninitialize(){
		//destroy vao
		if(vao_pyramid[0] != 0){
			GLES30.glDeleteVertexArrays(1,vao_pyramid, 0);
			vao_pyramid[0] = 0;			
		}
		
		if(vbo_position[0] != 0){
			GLES30.glDeleteBuffers(1,vbo_position, 0);
			vbo_position[0] = 0;			
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