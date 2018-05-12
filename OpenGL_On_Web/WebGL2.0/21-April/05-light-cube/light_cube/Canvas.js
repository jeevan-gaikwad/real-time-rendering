//global variables
var canvas=null;
var gl=null; //for webgl context
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros=
{
	JCG_ATTRIBUTE_VERTEX:0,
	JCG_ATTRIBUTE_COLOR:1,
	JCG_ATTRIBUTE_NORMAL:2,
	JCG_ATTRIBUTE_TEXTURE0:3
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;


var vbo_position;
var vbo_normal;
var vao_cube;
var mvpUniform;
var gAngle=0.0;
var modelViewMatrixUniform, projectionMatrixUniform;
var ldUniform, kdUniform, lightPositionUniform, doubleTapUniform ;
var gbOnMouseClickLight1=false;
var perspectiveProjectionMatrix;

//To start animation
var requestAnimationFrame = window.requestAnimationFrame ||
							window.webkitRequestAnimationFrame||
							window.mozRequestAnimationFrame||
							window.oRequestAnimationFrame||
							window.msRequestAnimationFrame;

//To stop animation
var cancelAnimationFrage = 
			window.cancelAnimationFrame||
			window.webkitCancelRequestAnimationFrame||
			window.webkitCancelAnimationFrame||
			window.mozCancelRequestAnimationFrame||
			window.mozCancelAnimationFrame||
			window.oCancelRequestAnimationFrame||
			window.oCancelAnimationFrame||
			window.msCancelRequestAnimationFrame||
			window.msCancelAnimationFrame;
			
//on body load function
function main()
{
	//get canvas elementFromPoint
	canvas = document.getElementById("AMC");
	if(!canvas)
		console.log("Obtaining canvas from main document failed\n");
	else
		console.log("Obtaining canvas from main document succeeded\n");
	//print obtained canvas width and height on console
	console.log("Canvas width:" + canvas.width +" height:" +canvas.height);
	canvas_original_width = canvas.width;
	canvas_original_height = canvas.height;
	
	
	//register keyboard and mouse event with window class
	window.addEventListener("keydown", keydown, false);
	window.addEventListener("click", mouseDown, false);
	window.addEventListener("resize", resize, false);
	
	init();
	resize();
	draw();
}

function init(){
	//Get OpenGL context
	gl=canvas.getContext("webgl2");
	if(gl == null)
		console.log("Obtaining 2D webgl2 failed\n");
	else
		console.log("Obtaining 2D webgl2 succeeded\n");
	
	gl.viewportWidth  = canvas.width;
	gl.viewportHeight  = canvas.height;
	
	//vertex shaderProgramObject
	var vertexShaderSourceCode =
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
	"}" ;
	vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
	gl.compileShader(vertexShaderObject);
	if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS) == false)
	{
		var error=gl.getShaderInfoLog(vertexShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}
	
	//fragmentShader
	var fragmentShaderSource =
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
	"}";
	fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShaderObject,fragmentShaderSource);
	gl.compileShader(fragmentShaderObject);
	if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS) == false)
	{
		var error=gl.getShaderInfoLog(fragmentShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}
	//shader program
	shaderProgramObject=gl.createProgram();
	gl.attachShader(shaderProgramObject, vertexShaderObject);
	gl.attachShader(shaderProgramObject, fragmentShaderObject);
	
	//pre-link binidng of shader program object with vertex shader attributes
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.JCG_ATTRIBUTE_VERTEX, "vPosition");
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.JCG_ATTRIBUTE_NORMAL, "vNormal");
	//linking
	gl.linkProgram(shaderProgramObject);
	if(!gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS))
	{
		var error = gl.getProgramInfoLog(shaderProgramObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}

	modelViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_view_matrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	doubleTapUniform = gl.getUniformLocation(shaderProgramObject, "u_double_tap");
	ldUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");
	
	var cubeVertices = new Float32Array([
										//front face
										-1.0, 1.0, 1.0, //left top
										-1.0, -1.0, 1.0,  //left bottom
										1.0, -1.0, 1.0,  //right bottom
										1.0, 1.0, 1.0, //right top
										//right face
										1.0, 1.0, 1.0,//left top
										1.0, -1.0, 1.0, //left bottom
										1.0, -1.0, -1.0, //right bottom
										1.0, 1.0, -1.0,//right top

										//back face
										1.0, 1.0, -1.0,//left top
										1.0, -1.0, -1.0,//left bottom
										-1.0, -1.0, -1.0, //right bottom
										-1.0, 1.0, -1.0, //right top

										//left face
										- 1.0, 1.0, -1.0,
										-1.0, -1.0, -1.0,
										-1.0, -1.0, 1.0,
										-1.0, 1.0, 1.0,

										//top face
										-1.0, 1.0, -1.0,
										-1.0, 1.0, 1.0,
										1.0, 1.0, 1.0,
										1.0, 1.0, -1.0,

										//bottom face
										-1.0, -1.0, -1.0,
										-1.0, -1.0, 1.0,
										1.0, -1.0, 1.0,
										1.0, -1.0, -1.0
									]);
	vao_cube=gl.createVertexArray();
	gl.bindVertexArray(vao_cube);
	//vbo for positions
	vbo_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
	gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_VERTEX,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_VERTEX);	

	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	var cubeNormals = new Float32Array([	

		0.0,0.0,1.0,
		0.0,0.0,1.0,
		0.0,0.0,1.0,
		0.0,0.0,1.0,
		
		1.0,0.0,0.0,
		1.0,0.0,0.0,
		1.0,0.0,0.0,
		1.0,0.0,0.0,

		0.0,0.0,-1.0,
		0.0,0.0,-1.0,
		0.0,0.0,-1.0,
		0.0,0.0,-1.0,

		-1.0,0.0,0.0,
		-1.0,0.0,0.0,
		-1.0,0.0,0.0,
		-1.0,0.0,0.0,

		0.0,1.0,0.0,
		0.0,1.0,0.0,
		0.0,1.0,0.0,
		0.0,1.0,0.0,

		0.0,-1.0,0.0,
		0.0,-1.0,0.0,
		0.0,-1.0,0.0,
		0.0,-1.0,0.0,

		]);
	vbo_normal =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_normal);
	gl.bufferData(gl.ARRAY_BUFFER, cubeNormals, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_NORMAL,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_NORMAL);	

	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	gl.bindVertexArray(null);//done with vao
	
	gl.clearColor(0.0,0.0,0.0,1.0);
	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	perspectiveProjectionMatrix =  mat4.create();
}

function resize()
{
	if(bFullscreen == true)
	{
		canvas.width = window.innerWidth;
		canvas.height = window.innerHeight;		
	}else
	{
		canvas.width = canvas_original_width;
		canvas.height = canvas_original_height;
	}
	
	mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width/canvas.height),0.1, 100.0);		
	
	gl.viewport(0,0,canvas.width,canvas.height);
}
function draw()
{
	gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
	gl.useProgram(shaderProgramObject);
	
	//lighting details
	if(gbOnMouseClickLight1){
			gl.uniform1i(doubleTapUniform, 1);
			gl.uniform3f(ldUniform, 1.0, 1.0, 1.0);
			gl.uniform3f(kdUniform, 0.5, 0.5, 0.5);
			//var lightPosition = new Float32Array([0.0, 0.0, 2.0, 1.0]);
			
			gl.uniform4fv(lightPositionUniform, [0.0, 0.0, 2.0, 1.0]);
	}else{
			gl.uniform1i(doubleTapUniform, 0);
	}
	var modelMatrix = mat4.create();
	var modelViewMatrix = mat4.create();
	
	var angleInRadian = degreeToRadian(gAngle);
	mat4.translate(modelMatrix, modelMatrix, [0.0,0.0,-4.0]);
	mat4.rotateX(modelMatrix, modelMatrix,angleInRadian);
	mat4.rotateY(modelMatrix, modelMatrix,angleInRadian);
	mat4.rotateZ(modelViewMatrix, modelMatrix,angleInRadian);
	//mat4.multiply(modelViewMatrix, modelViewMatrix, modelMatrix);
	gl.uniformMatrix4fv(modelViewMatrixUniform,false,modelViewMatrix);
	
	gl.uniformMatrix4fv(projectionMatrixUniform,false,perspectiveProjectionMatrix);
	
	
	gl.bindVertexArray(vao_cube);
	gl.drawArrays(gl.TRIANGLE_FAN, 0,4);
	gl.drawArrays(gl.TRIANGLE_FAN, 4,4);
	gl.drawArrays(gl.TRIANGLE_FAN, 8,4);
	gl.drawArrays(gl.TRIANGLE_FAN, 12,4);
	gl.drawArrays(gl.TRIANGLE_FAN, 16,4);
	gl.drawArrays(gl.TRIANGLE_FAN, 20,4);
	gl.bindVertexArray(null);
		
	gl.useProgram(null);
	update();
	requestAnimationFrame(draw,canvas);
}
function toggleFullScreen()
{
	//code
	var fullScreen_element = 
		document.fullscreenElement||
		document.webkitFullscreenElement||
		document.mozFullScreenElement||
		document.msFullscreenElement||
		null;
		
	//if not full screen
	if(fullScreen_element == null)
	{
		if(canvas.requestFullscreen)
			canvas.requestFullscreen();
		else if(canvas.mozRequestFullScreen)
			canvas.mozRequestFullScreen();
		else if(canvas.webkitRequestFullscreen)
			canvas.webkitRequestFullscreen();
		else if(canvas.msRequestFullscreen)
			canvas.msRequestFullscreen();
	}
	else //restore from fullscreen
	{
			if(document.exitFullscreen)
				document.exitFullscreen();
			else if(document.mozCancelFullScreen)
				document.mozCancelFullScreen();
			else if(document.webkitExitFullscreen)
				document.webkitExitFullscreen();
			else if(document.msExitFullscreen)
				document.msExitFullscreen();
	
	}
	resize();
}

function keydown(event)
{
	switch(event.keyCode)
	{
		case 27://Esc
			uninitialize();
			window.close();
			break;
		case 70: //for 'F' or 'f'
			if(bFullscreen == true)
				bFullscreen = false;
			else
				bFullscreen = true;			
		toggleFullScreen();			
			break;
	}
}
function update()
{
		if( gAngle >= 360.0)
			gAngle = 0.0;
		else
			gAngle = gAngle + 1.0;
}
function degreeToRadian(angleInDegree)
{
	return (angleInDegree *  Math.PI/ 180);
}
function mouseDown()
{
	//alert("Mouse is clicked");
	if(gbOnMouseClickLight1)
		gbOnMouseClickLight1 = false;
	else
		gbOnMouseClickLight1 = true;
}

function uninitialize()
{
	if(vao_pyramid)
	{
		gl.deleteVertexArray(vao_pyramid);
		vao_pyramid = null;
	}
	if(vao_cube)
	{
		gl.deleteVertexArray(vao_cube);
		vao_cube = null;
	}
	if(vbo_position)
	{
		gl.deleteBuffer(vbo_position);
		vbo_position=null;
	}
	
	if(vbo_color)
	{
		gl.deleteBuffer(vbo_color);
		vbo_color=null;
	}
	
	if(shaderProgramObject)
	{
		if(fragmentShaderObject)
		{
			gl.detachShader(shaderProgramObject, fragmentShaderObject);
			fragmentShaderObject = null;
		}
		
		if(vertexShaderObject)
		{
			gl.detachShader(shaderProgramObject, vertexShaderObject);
			vertexShaderObject = null;
		}
		gl.deleteProgram(shaderProgramObject);
		shaderProgramObject = null;
	}
}