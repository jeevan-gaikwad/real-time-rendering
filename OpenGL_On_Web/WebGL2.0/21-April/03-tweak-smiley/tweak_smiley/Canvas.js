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

var vao_rectangle;
var vbo_position;
var vbo_texture;
var mvpUniform;
var gAngle=0.0;
var keycount =0;

var perspectiveProjectionMatrix;

var smiley_texture = 0;

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
//white color texture
var checkImageHeight = 64;
var checkImageWidth = 64;
var coloredBoardTexture=new  Uint8Array(64*64*4);
var colored_board_texture = 0;
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
	
	
	//Load smiley texture
	smiley_texture = gl.createTexture();
	smiley_texture.image = new Image();
	smiley_texture.image.src = "smiley.png";
	smiley_texture.image.onload = function ()
	{
		gl.bindTexture(gl.TEXTURE_2D, smiley_texture);
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0 , gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, smiley_texture.image);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
	GenerateGLTextures();//Generate procedural texture
	//get MVP uniform
	mvpUniform = gl.getUniformLocation(shaderProgramObject, "u_mvp_matrix");
	uniform_texture0_sampler =  gl.getUniformLocation(shaderProgramObject, "u_texture0_sampler");
	//Preparation to draw a pyramid
	
	var rectangleVertices = new Float32Array([
										-1.0, 1.0, 0.0,
										-1.0, -1.0, 0.0,
										1.0, -1.0, 0.0,
										1.0, 1.0, 0.0																
											]);
											
	
	vao_rectangle=gl.createVertexArray();
	gl.bindVertexArray(vao_rectangle);
	//vbo for positions
	vbo_position =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
	gl.bufferData(gl.ARRAY_BUFFER, rectangleVertices, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_VERTEX,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	
	
	vbo_texture =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture);
	gl.bufferData(gl.ARRAY_BUFFER, 8*4, gl.DYNAMIC_DRAW);
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
function GenerateGLTextures() {

	MakeColoredBoard();
/*
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &gColoredBoardTexture);
	glBindTexture(GL_TEXTURE_2D, gColoredBoardTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, coloredBoardTexture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
*/
	gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
	colored_board_texture = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, colored_board_texture);
	
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texImage2D(gl.TEXTURE_2D, 0 ,gl.RGB, checkImageWidth, checkImageHeight, 0,  gl.RGB, gl.UNSIGNED_BYTE, coloredBoardTexture,0);

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
	var rectangleTexcoords = new Float32Array(8);
	
		if (keycount == 1) { //show 0.5
		rectangleTexcoords[0] = 0.0;
		rectangleTexcoords[1] = 0.5;
		rectangleTexcoords[2] = 0.0;
		rectangleTexcoords[3] = 0.0;
		rectangleTexcoords[4] = 0.5;
		rectangleTexcoords[5] = 0.0;
		rectangleTexcoords[6] = 0.5;
		rectangleTexcoords[7] = 0.5;
	}else if (keycount == 2) { //show 0.5
		rectangleTexcoords[0] = 0.0;
		rectangleTexcoords[1] = 1.0;
		rectangleTexcoords[2] = 0.0;
		rectangleTexcoords[3] = 0.0;
		rectangleTexcoords[4] = 1.0;
		rectangleTexcoords[5] = 0.0;
		rectangleTexcoords[6] = 1.0;
		rectangleTexcoords[7] = 1.0;
	}
	else if (keycount == 3) { //double
		rectangleTexcoords[0] = 0.0;
		rectangleTexcoords[1] = 2.0;
		rectangleTexcoords[2] = 0.0;
		rectangleTexcoords[3] = 0.0;
		rectangleTexcoords[4] = 2.0;
		rectangleTexcoords[5] = 0.0;
		rectangleTexcoords[6] = 2.0;
		rectangleTexcoords[7] = 2.0;
	} 
	else if (keycount == 4) { //double
		rectangleTexcoords[0] = 0.5;
		rectangleTexcoords[1] = 0.5;
		rectangleTexcoords[2] = 0.5;
		rectangleTexcoords[3] = 0.5;
		rectangleTexcoords[4] = 0.5;
		rectangleTexcoords[5] = 0.5;
		rectangleTexcoords[6] = 0.5;
		rectangleTexcoords[7] = 0.5;
	}											
	gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
	gl.useProgram(shaderProgramObject);
	//draw pyramid
	var modelViewMatrix = mat4.create();
	var modelViewProjectionMatrix =  mat4.create();
	mat4.translate(modelViewMatrix, modelViewMatrix, [0.0,0.0,-4.0]);

	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
	
	gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);
	if(keycount == 0)
	{
		gl.bindTexture(gl.TEXTURE_2D, colored_board_texture);
	}else
		gl.bindTexture(gl.TEXTURE_2D, smiley_texture);
	gl.uniform1i(uniform_texture0_sampler, 0);
	
	gl.bindVertexArray(vao_rectangle);
	gl.bindBuffer(gl.ARRAY_BUFFER,vbo_texture);
	//gl.bufferData(gl.ARRAY_BUFFER, rectangleTexcoords, gl.DYNAMIC_DRAW,0, rectangleTexcoords.length); 
	gl.bufferData(gl.ARRAY_BUFFER, rectangleTexcoords, gl.DYNAMIC_DRAW); 
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
		case 49: //1
			keycount = 1;
			break;
		case 50: //2
			keycount = 2;
			break;
		case 51: //3
			keycount = 3;
			break;
		case 52: //4
			keycount = 4;
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

function mouseDown()
{
	alert("Mouse is clicked");
}
function MakeColoredBoard() {
	var i, j;
	for (i = 0;i < checkImageHeight*checkImageWidth*4;i=i+4) {
		
			coloredBoardTexture[i] = 128; //R value
			coloredBoardTexture[i+1] = 128; //R value
			coloredBoardTexture[i+2] = 50; //R value
			coloredBoardTexture[i+3] = 1; //R value
		
		
	}
}
function uninitialize()
{
	if(pyramid_texture)
	{
		gl.deleteTexture(pyramid_texture);
		pyramid_texture = 0;
	}
	if(smiley_texture)
	{
		gl.deleteTexture(smiley_texture);
		smiley_texture = 0;
	}
	if(vao_rectangle)
	{
		gl.deleteVertexArray(vao_rectangle);
		vao_rectangle = null;
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