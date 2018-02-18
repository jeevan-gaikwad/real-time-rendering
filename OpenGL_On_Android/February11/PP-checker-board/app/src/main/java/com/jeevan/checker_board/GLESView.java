package com.jeevan.checker_board;

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
import java.nio.IntBuffer;

//texture related imports
import android.graphics.BitmapFactory; //texture factory. Provides image object
import android.graphics.Bitmap; // For PNG image
import android.opengl.GLUtils;  //for texImage2D() call

//A view for OpenGLES3 graphics which also receives touch events

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
	private final Context context;
	private GestureDetector gestureDetector;
	
	//new class memebers
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;
	
	private int[] vao_checker_board = new int[1];
	private int[] vbo_position = new int[1];
	private int[] vbo_texure = new int[1];
	private int mvpUniform;
	private int texture0_sampler_uniform;
	
	private int[] texture_checker_board = new int[1];
	private float perspectiveProjectionMatrix[]=new float[16];
	private int onTouchCount;
	private int checkImageHeight = 128;
	private int checkImageWidth  = 128;
	private int textureArraySize = checkImageHeight * checkImageHeight* 4;
	private int[][][] coloredBoardTexture = new int[checkImageHeight][checkImageWidth][4];;
	private byte[] checkerBoard = new byte[textureArraySize]; 
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
		if(onTouchCount>3)
			onTouchCount = 0;
		else
			onTouchCount++;
		System.out.println("JCG: on single tap event received. Count is:" + onTouchCount);
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
		"in vec2 vTexture0_Coord;" +
		"out vec2 out_texture0_coord;" +
		"uniform mat4 u_mvp_matrix;" +
		"void main(void)" +
		"{" +
		"gl_Position = u_mvp_matrix * vPosition;" +		
		"out_texture0_coord = vTexture0_Coord;" +
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
		"in vec4 out_color;" +
		"in vec2 out_texture0_coord;" +
		"precision highp float;"+
		"uniform highp sampler2D u_texture0_sampler;" +
		"out vec4 FragColor;" +
		"void main(void)"+
		"{"+
		"FragColor = texture(u_texture0_sampler, out_texture0_coord);" +
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
		GLES30.glBindAttribLocation(shaderProgramObject, GLESMacros.JCG_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");
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
		float quadVertices[] = {
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};
		float chckerBoardTexcoords[] = {

			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
		//get MVP uniform location
		mvpUniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
		texture0_sampler_uniform = GLES30.glGetUniformLocation(shaderProgramObject, "u_texture0_sampler");
		
		//load textures		
		GenerateGLTextures(); //load checker board texture
		
		//DATA: vertices, colors, shader attribs, vao, vbos initialization
		
		GLES30.glGenVertexArrays(1, vao_checker_board, 0); 	//NOTE additional zero
		GLES30.glBindVertexArray(vao_checker_board[0]);		//NOTE how it is used
		
		//vbo for positions
		GLES30.glGenBuffers(1, vbo_position, 0); 		//Creates a vbo. 
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_position[0]);
		
		//Create a buffer to pass our float data to GPU in native fashion
		ByteBuffer byteBuffer =  ByteBuffer.allocateDirect(quadVertices.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(quadVertices); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, quadVertices.length * 4, null, GLES30.GL_DYNAMIC_DRAW); //we will provide data at runtime
		
		GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_VERTEX, 3, GLES30.GL_FLOAT, false, 0, 0);
		
		GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_VERTEX);
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);
		
		
		//vbo for texture	
		GLES30.glGenBuffers(1, vbo_texure, 0); 		//Creates a vbo. 
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_texure[0]);
		
		//Create a buffer to pass our float data to GPU in native fashion
		byteBuffer =  ByteBuffer.allocateDirect(chckerBoardTexcoords.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(chckerBoardTexcoords); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, chckerBoardTexcoords.length*4,verticesBuffer , GLES30.GL_STATIC_DRAW);
		
		GLES30.glVertexAttribPointer(GLESMacros.JCG_ATTRIBUTE_TEXTURE0, 2, GLES30.GL_FLOAT, false, 0, 0);
		
		GLES30.glEnableVertexAttribArray(GLESMacros.JCG_ATTRIBUTE_TEXTURE0);
			
		//done with vaos
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);
		GLES30.glBindVertexArray(0);
		
		//enable depth testing
		GLES30.glEnable(GLES30.GL_DEPTH_TEST);
		GLES30.glDepthFunc(GLES30.GL_LEQUAL);
		//GLES30.glEnable(GLES30.GL_CULL_FACE); //No culling for animation
		GLES30.glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //black
		//GLES30.glEnable(GLES30.GL_TEXTURE_2D);
		//set projection matrix to identity matrix
		Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
		
	}
	
	private void resize(int width, int height){
		GLES30.glViewport(0,0,width,height);
		
		
		//perspective projection
		Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f,((float)width/(float)height),0.1f,100.0f);
	}
	
	public void draw(){
		float quad1Vertices[] = {
		-2.0f, 1.0f, 0.0f,
		-2.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
		float tiltedQuadVertices[] = {
		2.41421f, -1.0f, -1.41421f,
		2.41421f, 1.0f, -1.41421f,
		1.0f,     1.0f,  0.0f,
		1.0f, -1.0f, 0.0f
	};
		ByteBuffer byteBuffer =  ByteBuffer.allocateDirect(quad1Vertices.length * 4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(quad1Vertices); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT|GLES30.GL_DEPTH_BUFFER_BIT);
		//use shader program
		GLES30.glUseProgram(shaderProgramObject);
		float modelViewMatrix[] = new float[16];
		float modelViewProjectionMatrix[] = new float [16];
		//set modelview and modelview projection matrix to identity
		Matrix.setIdentityM(modelViewMatrix, 0);
		Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		
		//multiply modelview and projection matrix to get modelViewProjection matrix
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.6f);
		Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0,modelViewMatrix,0);
		
		//pass above matrix to u_mvp_matrix
		
		GLES30.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);
		
		
		
		//bind vao. Start playing
		GLES30.glBindVertexArray(vao_checker_board[0]);
		
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_position[0]);
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, tiltedQuadVertices.length * 4, verticesBuffer, GLES30.GL_DYNAMIC_DRAW);
		
		//draw using glDrawArrays
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 0, 4);
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0); //unbind texture vbo
		
		Matrix.setIdentityM(modelViewMatrix, 0);
		Matrix.setIdentityM(modelViewProjectionMatrix, 0);
		
		//multiply modelview and projection matrix to get modelViewProjection matrix
		Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.6f);
		Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0,modelViewMatrix,0);
		
		//pass above matrix to u_mvp_matrix
		byteBuffer =  ByteBuffer.allocateDirect(tiltedQuadVertices.length*4); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		verticesBuffer = byteBuffer.asFloatBuffer();
		verticesBuffer.put(tiltedQuadVertices); //fill the data
		verticesBuffer.position(0); //Zero indicates, from where to start using the data
		
		
		GLES30.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);
		GLES30.glBindVertexArray(vao_checker_board[0]);
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo_position[0]);
		GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, tiltedQuadVertices.length * 4, verticesBuffer, GLES30.GL_DYNAMIC_DRAW);
		
		//draw using glDrawArrays
		GLES30.glDrawArrays(GLES30.GL_TRIANGLE_FAN, 0, 4);
		GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0); //unbind texture vbo
		
		
		GLES30.glBindVertexArray(0);
		
		GLES30.glUseProgram(0);
		
		//SwapBuffers 
		requestRender();
	}
	
	
	
	void MakeCheckerBoard() {
		
		int c;
		int index=0;
		for (int i = 0;i < checkImageHeight;i++) {
			for (int j = 0;j<checkImageWidth;j++) {
				for(int k=0;k<3;k++){
					int check = 0;
					int cond1=0,cond2=0;
					if(((i & 0x10) == 0) == true){ //0x10 represents 16. jumping by 16
						cond1=1;
					}
					if(((j & 0x10) == 0) == true){
						cond2=1;
					}
					int cond3=cond1 ^ cond2;
					if (cond3 == 1) { 
						check = 1;
					}
					c = check * 255;
					System.out.println("JCG:c is:"+c);
					checkerBoard[index++]=(byte)c;				
				}				
				checkerBoard[index++]=(byte)1;
			}
		}
		
    }
	void GenerateGLTextures() {
		MakeCheckerBoard();
		GLES30.glPixelStorei(GLES30.GL_UNPACK_ALIGNMENT, 1);
		GLES30.glGenTextures(1, texture_checker_board, 0);
		GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, texture_checker_board[0]);
		GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT);
		GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT);
		GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_NEAREST);
		GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST);
		ByteBuffer byteBuffer =  ByteBuffer.allocateDirect(checkerBoard.length); //4 is size of float. This is global mem allocation. All memory location will be initialized to zero
		byteBuffer.order(ByteOrder.nativeOrder()); //Detect native machine endianess and use it
		byteBuffer.put(checkerBoard);
		byteBuffer.position(0);
		GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA, checkImageWidth, checkImageHeight, 0/*border */, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, byteBuffer);

	}

	void uninitialize(){
		//destroy vao
		if(vao_checker_board[0] != 0){
			GLES30.glDeleteVertexArrays(1,vao_checker_board, 0);
			vao_checker_board[0] = 0;			
		}
		
		if(vbo_position[0] != 0){
			GLES30.glDeleteBuffers(1,vbo_position, 0);
			vbo_position[0] = 0;			
		}
		
		if(vbo_texure[0] != 0){
			GLES30.glDeleteBuffers(1,vbo_texure, 0);
			vbo_texure[0] = 0;			
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