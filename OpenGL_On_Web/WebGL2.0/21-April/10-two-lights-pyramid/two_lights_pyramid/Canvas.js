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
var vbo_normal;

var gAngle=0.0;
var modelMatrixUniform, viewMatrixUniform, projectionMatrixUniform;
var light0PositionUniform, light1PositionUniform, onLKeyUniform;
var l0aUniform, l0dUniform, l0sUniform;
var l1aUniform, l1dUniform, l1sUniform;
var kaUniform, kdUniform, ksUniform, materialShininessUniform;
var gbOnLKeyLight1=false;
var perspectiveProjectionMatrix;

var light0_ambient = [0.0, 0.0, 0.0];
var light0_diffuse = [1.0, 0.0, 0.0];
var light0_specular = [1.0, 0.0, 0.0];
var light0_position = [2.0, 2.0, 0.0, 1.0];
	
var light1_ambient = [0.0, 0.0, 0.0];
var light1_diffuse = [0.0, 0.0, 1.0];
var light1_specular = [0.0, 0.0, 1.0];
var light1_position = [-2.0, 2.0, 0.0, 1.0];

var material_ambient = [0.0, 0.0, 0.0];
var material_diffuse = [1.0, 1.0, 1.0];
var material_specular = [1.0, 1.0, 1.0];
	
var material_shininess = 50.0;
var sphere=null;
//To start animation
var requestAnimationFrame = window.requestAnimationFrame ||
							window.webkitRequestAnimationFrame||
							window.mozRequestAnimationFrame||
							window.oRequestAnimationFrame||
							window.msRequestAnimationFrame;

//To stop animation
var cancelAnimationFrame = 
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
	"in vec3 phong_ads_color;" +
	"out vec4 FragColor;" +
	"void main(void)" +
	"{" +
	"FragColor = vec4(phong_ads_color, 1.0);" +
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

	
	modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_model_matrix");
	viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_view_matrix");
	projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_projection_matrix");
	onLKeyUniform = gl.getUniformLocation(shaderProgramObject, "u_lighting_enabled");
	l0aUniform = gl.getUniformLocation(shaderProgramObject, "u_L0a");		
	l0dUniform = gl.getUniformLocation(shaderProgramObject, "u_L0d");
	l0sUniform = gl.getUniformLocation(shaderProgramObject, "u_L0s");
	
	l1aUniform = gl.getUniformLocation(shaderProgramObject, "u_L1a");		
	l1dUniform = gl.getUniformLocation(shaderProgramObject, "u_L1d");
	l1sUniform = gl.getUniformLocation(shaderProgramObject, "u_L1s");
	
	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
	
	light0PositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light0_position");
	light1PositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light1_position");
	materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");
	
//Preparation to draw a pyramid
	
	var pyramidVertices = new Float32Array([
										//front face
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
	
//Preparation to for pyramid normals
	
	var pyramidNormals = new Float32Array([
								0.0, 0.447214, 0.894427, //front face normals
								0.0, 0.447214, 0.894427, //front face normals
								0.0, 0.447214, 0.894427, //front face normals

								0.894427, 0.447214, 0.0, //right face
								0.894427, 0.447214, 0.0, //right face
								0.894427, 0.447214, 0.0, //right face

								0.0, 0.447214, -0.894427, //back face
								0.0, 0.447214, -0.894427, //back face
								0.0, 0.447214, -0.894427, //back face

								-0.894427, 0.447214, 0.0, //left face
								-0.894427, 0.447214, 0.0, //left face
								- 0.894427, 0.447214, 0.0 //left face
								]);
	
	vbo_normal =  gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vbo_normal);
	gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.JCG_ATTRIBUTE_NORMAL,
							3,
							gl.FLOAT,
							false, 0, 0);
	gl.enableVertexAttribArray(WebGLMacros.JCG_ATTRIBUTE_NORMAL);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
		
	gl.clearColor(0.0,0.0,0.0,1.0);
	//gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	gl.enable(gl.CULL_FACE);
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
	if(gbOnLKeyLight1){
			gl.uniform1i(onLKeyUniform, 1);
			gl.uniform3fv(l0aUniform, light0_ambient);
			gl.uniform3fv(l0dUniform, light0_diffuse);
			gl.uniform3fv(l0sUniform, light0_specular);
			
			gl.uniform4fv(light0PositionUniform, light0_position);
			
			gl.uniform3fv(l1aUniform, light1_ambient);
			gl.uniform3fv(l1dUniform, light1_diffuse);
			gl.uniform3fv(l1sUniform, light1_specular);
			
			gl.uniform4fv(light1PositionUniform, light1_position);
			
			//set material properties
			gl.uniform3fv(kaUniform, material_ambient);
			gl.uniform3fv(kdUniform, material_diffuse);
			gl.uniform3fv(ksUniform, material_specular);
			gl.uniform1f(materialShininessUniform,material_shininess);
			
	}else{
			gl.uniform1i(onLKeyUniform, 0);
	}
	var modelMatrix = mat4.create();
	var viewMatrix = mat4.create();
	
	var angleInRadian = degreeToRadian(gAngle);
	mat4.translate(modelMatrix, modelMatrix, [0.0,0.0,-4.0]);
	mat4.rotateY(modelMatrix, modelMatrix,angleInRadian);
	//mat4.multiply(modelViewMatrix, modelViewMatrix, modelMatrix);
	gl.uniformMatrix4fv(modelMatrixUniform,false,modelMatrix);
	gl.uniformMatrix4fv(viewMatrixUniform,false,viewMatrix);
	gl.uniformMatrix4fv(projectionMatrixUniform,false,perspectiveProjectionMatrix);
	
	//draw pyramid
	gl.bindVertexArray(vao_pyramid);
	gl.drawArrays(gl.TRIANGLES, 0,12);
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
		case 76:
		case 108:
				if(gbOnLKeyLight1)
					gbOnLKeyLight1 = false;
				else
					gbOnLKeyLight1 = true;
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
	if(vao_pyramid)
	{
		gl.deleteVertexArray(vao_pyramid);
		vao_pyramid = null;
	}
	if(vbo_position)
	{
		gl.deleteBuffer(vbo_position);
		vbo_position=null;
	}
	if(vbo_normal)
	{
		gl.deleteBuffer(vbo_normal);
		vbo_normal=null;
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