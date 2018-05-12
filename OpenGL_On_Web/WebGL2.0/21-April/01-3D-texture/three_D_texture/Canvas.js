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

var vao_pyramid;
var vbo_position;
var vbo_texture;
var vao_cube;
var mvpUniform;
var gAngle=0.0;

var perspectiveProjectionMatrix;

var pyramid_texture = 0;
var cube_texture = 0;
var uniform_texture0_sampler;

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
	"in vec2 vTexture0_Coord;" +
	"out vec2 out_texture0_coord;" +
	"uniform mat4 u_mvp_matrix;"+
	"void main(void)"+
	"{"+
	"gl_Position = u_mvp_matrix * vPosition;"+
	"/* Just pass texture coords as it is to fragment shader*/" +
	"out_texture0_coord = vTexture0_Coord;" +
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
	"in vec2 out_texture0_coord;" +
	"uniform highp sampler2D u_texture0_sampler;"+
	"out vec4 FragColor;"+
	"void main(void)"+
	"{"+
	"FragColor = texture(u_texture0_sampler, out_texture0_coord);"+
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
	gl.bindAttribLocation(shaderProgramObject, WebGLMacros.JCG_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");
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
	
	//Load pyramid texture
	pyramid_texture = gl.createTexture();
	pyramid_texture.image = new Image();
	pyramid_texture.image.src = "stone.png";
	pyramid_texture.image.onload = function ()
	{
		gl.bindTexture(gl.TEXTURE_2D, pyramid_texture);
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0 , gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, pyramid_texture.image);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
	
	//Load cube texture
	cube_texture = gl.createTexture();
	cube_texture.image = new Image();
	cube_texture.image.src = "kundali.png";
	cube_texture.image.onload = function ()
	{
		gl.bindTexture(gl.TEXTURE_2D, cube_texture);
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0 , gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, cube_texture.image);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
	//get MVP uniform
	mvpUniform = gl.getUniformLocation(shaderProgramObject, "u_mvp_matrix");
	uniform_texture0_sampler =  gl.getUniformLocation(shaderProgramObject, "u_texture0_sampler");
	//Preparation to draw a pyramid
	
	var pyramidVertices = new Float32Array([
										0.0, 1.0, 0.0, //apex of the triangle
										-1.0, -1.0, 1.0, //left-bottom
										1.0, -1.0, 1.0, //right-bottom
										//right face
										0.0, 1.0, 0.0, //apex
										1.0, -1.0, 1.0,//left bottom
										1.0, -1.0, -1.0, //right bottom
										//back face
										0.0, 1.0, 0.0, //apex
										1.0, -1.0, -1.0,
										-1.0, -1.0, -1.0,
										//left face
										0.0, 1.0, 0.0, //apex
										-1.0, -1.0, -1.0, //left bottom
										-1.0, -1.0, 1.0 //right bottom																		
											]);
											
	var pyramidTexcoords = new Float32Array([
										0.5, 1.0, //front top
										0.0, 0.0, //front left
										1.0, 0.0, //front right

										0.5, 1.0, //right top
										1.0, 0.0, //right left
										0.0, 0.0, //right right

										0.5, 1.0, //back top
										1.0, 0.0, //back left
										0.0, 0.0, //back right

										0.5, 1.0, //left top
										0.0, 0.0, //left left
										1.0, 0.0 //left right
											]);
	vao_pyramid=gl.createVertexArray();
	gl.bindVertexArray(vao_pyramid);
	//vbo for positions
	vbo_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_VERTEX,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	
	vbo_texture =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidTexcoords, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_TEXTURE0,
							2,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_TEXTURE0);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindVertexArray(null);//done with pyramid vao
	
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
									
	var cubeTexcoords = new Float32Array([
									0.0, 0.0,
									1.0, 0.0,
									1.0, 1.0,
									0.0, 1.0,

									0.0, 0.0,
									1.0, 0.0,
									1.0, 1.0,
									0.0, 1.0,

									0.0, 0.0,
									1.0, 0.0,
									1.0, 1.0,
									0.0, 1.0,

									0.0, 0.0,
									1.0, 0.0,
									1.0, 1.0,
									0.0, 1.0,

									0.0, 0.0,
									1.0, 0.0,
									1.0, 1.0,
									0.0, 1.0,

									0.0, 0.0,
									1.0, 0.0,
									1.0, 1.0,
									0.0, 1.0
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

	
	vbo_texture =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture);
	gl.bufferData(gl.ARRAY_BUFFER, cubeTexcoords, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_TEXTURE0,
							2,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_TEXTURE0);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindVertexArray(null);//done with pyramid vao
	
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
	//draw pyramid
	var modelViewMatrix = mat4.create();
	var modelViewProjectionMatrix =  mat4.create();
	var angleInRadian = degreeToRadian(gAngle);
	mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5,0.0,-4.0]);
	mat4.rotateY(modelViewMatrix, modelViewMatrix,angleInRadian);
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);
	gl.bindTexture(gl.TEXTURE_2D, pyramid_texture);
	gl.uniform1i(uniform_texture0_sampler, 0);
	
	gl.bindVertexArray(vao_pyramid);
	gl.drawArrays(gl.TRIANGLES, 0,12);
	gl.bindVertexArray(null);
	
	//draw cube
	modelViewMatrix =  mat4.identity(modelViewMatrix);
	modelViewProjectionMatrix =  mat4.identity(modelViewProjectionMatrix);
	mat4.translate(modelViewMatrix, modelViewMatrix, [1.5,0.0,-4.0]);
	mat4.scale(modelViewMatrix, modelViewMatrix, [0.75, 0.75, 0.75]);
	mat4.rotateX(modelViewMatrix, modelViewMatrix,angleInRadian);
	
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	gl.bindTexture(gl.TEXTURE_2D, cube_texture);
	gl.uniform1i(uniform_texture0_sampler, 0);
	gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);
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
	alert("Mouse is clicked");
}

function uninitialize()
{
	if(pyramid_texture)
	{
		gl.deleteTexture(pyramid_texture);
		pyramid_texture = 0;
	}
	if(cube_texture)
	{
		gl.deleteTexture(cube_texture);
		cube_texture = 0;
	}
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
	
	if(vbo_texture)
	{
		gl.deleteBuffer(vbo_texture);
		vbo_texture=null;
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