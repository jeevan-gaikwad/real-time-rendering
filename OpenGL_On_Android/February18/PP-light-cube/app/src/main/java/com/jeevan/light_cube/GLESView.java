package com.jeevan.light_cube;

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

//A view for OpenGLES3 graphics which also receives touch events

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
	private final Context context;
	private GestureDetector gestureDetector;
	
	//new class members
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;
	
	private int[] vao_cube = new int[1];
	private int[] vbo_cube_position = new int[1];
	private int[] vbo_cube_normals = new int[1];
	private int   modelViewMatrixUniform, projectionMatrixUniform;
	private int   ldUniform, kdUniform, lightPositionUniform, doubleTapUniform ;
	private boolean gbOnDoubleTapLight1=false, gbOnSingleTapUpAnimation=false;
	private float perspectiveProjectionMatrix[]=new float[16];
	private float angle_cube;
	private float zoomZ = -5.0f;
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
	if(gbOnDoubleTapLight1)
		gbOnDoubleTapLight1 = false;
	else
		gbOnDoubleTapLight1 = true;
		return true;
	}
	@Override //abstract method from OnDoubleTapListener
	public boolean onDoubleTapEvent(MotionEvent e){
		//nothing to do for now. Already handled in onDoubleTap
		return true;
	}
	
	@Override //abstract method from OnDoubleTapListener
	public boolean onSingleTapConfirmed(MotionEvent e){
		if(gbOnSingleTapUpAnimation)
			gbOnSingleTapUpAnimation = false;
		else
			gbOnSingleTapUpAnimation = true;
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
		"uniform mat4 u_model_view_matrix;" +
		"uniform mat4 u_projection_matrix;" +
		"uniform mediump int u_double_tap;" +
		"uniform vec3 u_Ld;" +
		"uniform vec3 u_Kd;" +
		"uniform vec4 u_light_position;" +
		"out vec3 diffuse_light;" +
		"void main(void)" +
		"{" +
		"if(u_double_tap == 1)"+
		"{" +
		"vec4 eyeCoordinates = u_model_view_matrix * vPosition;" +
		"vec3 tnorm = normalize(mat3(u_model_view_matrix)* vNormal);" +
		"vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" +
		"diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm),0.0);" +
		"}"+
		"gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" +		
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
		"in vec3 diffuse_light;" +
		"out vec4 FragColor;" +
		"uniform int u_double_tap;" + 
		"void main(void)" +
		"{"+
		"vec4 color;" +
		"if(u_double_tap == 1)" +
		"{" +
		"color = vec4(diffuse_light,1.0);" +
		"}"+
		"else" +
		"{"+
		"color = vec4(1.0, 1.0, 1.0, 1.0);"+
		"}"+
		"FragColor = color;" +
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
		modelViewMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_model_view_matrix");
		projectionMatrixUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_projection_matrix");
		doubleTapUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_double_tap");
		ldUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Ld");
		kdUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_Kd");
		lightPositionUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_light_position");
		//DATA: vertices, colors, shader attribs, vao, vbos initialization
		
		final float cubeVertices[] = new float[]{
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
		
		final float cubeNormals[] = new float[]{

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
		
		GLES30.glGenVertexArrays(1, vao_cube, 0); 	//NOTE additional zero
		GLES30.glBindVertexArray(vao_cube[0]);		//NOTE how it is used
		
		//vbo for positions
		GLES30.glGenBuffers(1, vbo_cube_position, 0); 		//Creates a vbo. 
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_cube_position[0]);
		
		//Create a buffer to pass our float data to GPU in native fashion
		ByteBuffer byteBuffer =  ByteBuffer.allocateDirect(cubeVertices.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(cubeVertices); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, cubeVertices.length * 4, verticesBuffer, GLES30.GL_STATIC_DRAW);
		
		GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_VERTEX, 3, GLES30.GL_FLOAT, false, 0, 0);
		
		GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_VERTEX);
		
		//vbo for cube color
		GLES30.glGenBuffers(1, vbo_cube_normals, 0); 		//Creates a vbo for color
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_cube_normals[0]);
		
		//Create a buffer to pass our float data to GPU in native fashion
		byteBuffer =  ByteBuffer.allocateDirect(cubeNormals.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		FloatBuffer normalsBuffer = byteBuffer.asFloatBuffer();
		normalsBuffer.put(cubeNormals); //fill the data
		normalsBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, cubeNormals.length * 4, normalsBuffer, GLES30.GL_STATIC_DRAW);
		
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
		
		if(gbOnDoubleTapLight1){
			GLES30.glUniform1i(doubleTapUniform, 1);
			GLES30.glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f);
			GLES30.glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
			float[] lightPosition = {0.0f, 0.0f, 2.0f, 1.0f};
			GLES30.glUniform4fv(lightPositionUniform, 1, lightPosition, 0);
		}else{
			GLES30.glUniform1i(doubleTapUniform, 0);
		}
		
		float modelMatrix[] = new float[16];
		float modelViewMatrix[] = new float [16];
		float rotationMatrix[] = new float [16];
		//set modelview and modelview projection matrix to identity
		Matrix.setIdentityM(modelMatrix, 0);
		Matrix.setIdentityM(modelViewMatrix, 0);
		Matrix.setIdentityM(rotationMatrix, 0);
		
		//multiply modelview and projection matrix to get modelViewProjection matrix
		Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, zoomZ);
		Matrix.setRotateM(rotationMatrix, 0, angle_cube , 1.0f, 1.0f, 1.0f); //all axes rotation
		Matrix.multiplyMM(modelViewMatrix, 0, modelMatrix, 0,rotationMatrix,0);
		
		//pass modelView matrix to shader
		
		GLES30.glUniformMatrix4fv(modelViewMatrixUniform, 1, false, modelViewMatrix, 0);
		
		//pass projection matrix to shader
		
		GLES30.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);
		
		
		//bind vao. Start playing
		
		
		GLES30.glBindVertexArray(vao_cube[0]);
		
		//draw cube using glDrawArrays
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 0, 4); //front face
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 4, 4); //right face
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 8, 4); //back face
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 12, 4); //left face
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 16, 4); //top face
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 20, 4); //bottom face
		
		GLES30.glBindVertexArray(0);
	
		
		
		GLES30.glUseProgram(0);
		
		if(gbOnSingleTapUpAnimation == true)
			spin(); //change angle of rotation
		//SwapBuffers 
		requestRender();
	}
	void spin(){
		angle_cube =  angle_cube + 0.5f;
		if (angle_cube >= 360.0f) {
			angle_cube = angle_cube - 360.0f;
		}
	}
	void uninitialize(){
		//destroy vao
		
		
		if(vao_cube[0] != 0){
			GLES30.glDeleteVertexArrays(1,vao_cube, 0);
			vao_cube[0] = 0;			
		}
		
		if(vbo_cube_position[0] != 0){
			GLES30.glDeleteBuffers(1,vbo_cube_position, 0);
			vbo_cube_position[0] = 0;			
		}
		
		if(vbo_cube_normals[0] != 0){
			GLES30.glDeleteBuffers(1,vbo_cube_normals, 0);
			vbo_cube_normals[0] = 0;			
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