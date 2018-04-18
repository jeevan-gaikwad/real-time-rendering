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

var vao_triangle;
var vbo_position;
var vbo_color;
var vao_square;
var mvpUniform;

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
	"#version 300 es"+
	"\n" +
	"in vec4 vPosition;" +
	"uniform mat4 u_mvp_matrix;"+
	"void main(void)"+
	"{"+
	"gl_Position = u_mvp_matrix * vPosition;"+
	"}";
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
	"out vec4 FragColor;"+
	"void main(void)"+
	"{"+
	"FragColor = vec4(0.0,0.0,0.0,0.0);"+
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
	//get MVP uniform
	mvpUniform = gl.getUniformLocation(shaderProgramObject, "u_mvp_matrix");
	
	//Preparation to draw a triangle
	
	var triangleVertices = new Float32Array([
											0.0, 1.0, 0.0,
											-1.0,-1.0,0.0,
											1.0, -1.0, 0.0																		
											]);
	vao_triangle=gl.createVertexArray();
	gl.bindVertexArray(vao_triangle);
	//vbo for positions
	vbo_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
	gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_VERTEX,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindVertexArray(null);
	
	var squareVertices = new Float32Array([
											-1.0, 1.0, 0.0, //left top
											-1.0, -1.0, 0.0,  //left bottom
											1.0, -1.0, 0.0,  //right bottom
											1.0, 1.0, 0.0 //right top
											]);
	vao_square=gl.createVertexArray();
	gl.bindVertexArray(vao_square);
	//vbo for positions
	vbo_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
	gl.bufferData(gl.ARRAY_BUFFER, squareVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_VERTEX,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);

	gl.clearColor(0.0,0.0,0.0,1.0);

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
	gl.clear(gl.COLOR_BUFFER_BIT);
	gl.useProgram(shaderProgramObject);
	var modelViewMatrix = mat4.create();
	var modelViewProjectionMatrix =  mat4.create();
	mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5,0.0,-4.0]);
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);
	
	gl.bindVertexArray(vao_triangle);
	gl.drawArrays(gl.TRIANGLES, 0,3);
	gl.bindVertexArray(null);
	
	//draw square
	modelViewMatrix =  mat4.identity(modelViewMatrix);
	modelViewProjectionMatrix =  mat4.identity(modelViewProjectionMatrix);
	mat4.translate(modelViewMatrix, modelViewMatrix, [1.5,0.0,-4.0]);
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);
	gl.bindVertexArray(vao_square);
	gl.drawArrays(gl.TRIANGLE_FAN, 0,4);
	gl.bindVertexArray(null);
		
	gl.useProgram(null);
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
			toggleFullScreen();			
			break;
	}
}

function mouseDown()
{
	alert("Mouse is clicked");
}

function uninitialize()
{
	if(vao_triangle)
	{
		gl.deleteVertexArray(vao_triangle);
		vao_triangle = null;
	}
	if(vao_square)
	{
		gl.deleteVertexArray(vao_square);
		vao_square = null;
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