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
var modelMatrixUniform, viewMatrixUniform, projectionMatrixUniform;
var onLKeyUniform, gLight0PositionUniform, gLight1PositionUniform, gLight2PositionUniform ;
var laUniform, L0d_uniform, L0s_uniform, L1d_uniform, L1s_uniform, L2d_uniform, L2s_uniform;
var kaUniform, kdUniform, ksUniform, materialShininessUniform;
var gbOnLKeyLight1=false;
var perspectiveProjectionMatrix;
var RADIUS = 500;
var light_ambient = [0.0, 0.0, 0.0];
	
var light0_diffuse = [1.0, 0.0, 0.0];
var light0_specular = [1.0, 0.0, 0.0];

var light1_diffuse = [0.0, 1.0, 0.0];
var light1_specular = [0.0, 1.0, 0.0];

var light2_diffuse = [0.0, 0.0, 1.0];
var light2_specular = [0.0, 0.0, 1.0];

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
		"uniform vec4 u_light0_position;" +
		"uniform vec4 u_light1_position;" +
		"uniform vec4 u_light2_position;" +
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
		"int u_lighting_enabled = 1;"+
		"if(u_lighting_enabled == 1)" +
		"{" +
		"        /*light0 calculations  */       " +
		"vec4 eye_coordinates_light0 = u_view_matrix * u_model_matrix * vPosition;" +
		"transformed_normals_light0 = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
		"light_direction_light0 = vec3(u_light0_position) - eye_coordinates_light0.xyz;" +
		"viewer_vector_light0 = -eye_coordinates_light0.xyz;" +		
		"        /*light1 calculations  */       " +
		"vec4 eye_coordinates_light1 = u_view_matrix * u_model_matrix * vPosition;" +
		"transformed_normals_light1 = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
		"light_direction_light1 = vec3(u_light1_position) - eye_coordinates_light1.xyz;" +
		"viewer_vector_light1 = -eye_coordinates_light1.xyz;" +
		"        /*light2 calculations  */       " +
		"vec4 eye_coordinates_light2 = u_view_matrix * u_model_matrix * vPosition;" +
		"transformed_normals_light2 = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
		"light_direction_light2 = vec3(u_light2_position) - eye_coordinates_light2.xyz;" +
		"viewer_vector_light2 = -eye_coordinates_light2.xyz;" +
		"}" +
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
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
	"out vec4 FragColor;" +
	"void main(void)" +
	"{" +
	"vec3 phong_ads_color;"+
	"int u_lighting_enabled = 1;"+
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
	laUniform = gl.getUniformLocation(shaderProgramObject, "u_La");		
	l0dUniform = gl.getUniformLocation(shaderProgramObject, "u_L0d");
	l0sUniform = gl.getUniformLocation(shaderProgramObject, "u_L0s");
	
	l1dUniform = gl.getUniformLocation(shaderProgramObject, "u_L1d");
	l1sUniform = gl.getUniformLocation(shaderProgramObject, "u_L1s");
	
	l2dUniform = gl.getUniformLocation(shaderProgramObject, "u_L2d");
	l2sUniform = gl.getUniformLocation(shaderProgramObject, "u_L2s");
	
	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
	
	gLight0PositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light0_position");
	gLight1PositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light1_position");
	gLight2PositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light2_position");
	
	materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");
	
	sphere = new Mesh();
	makeSphere(sphere,2.0,30,30);
	
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
	
	gl.uniform1i(onLKeyUniform, 1);
	gl.uniform3fv(laUniform, light_ambient);
	
	gl.uniform3fv(l0dUniform, light0_diffuse);
	gl.uniform3fv(l0sUniform, light0_specular);
	
	gl.uniform3fv(l1dUniform, light1_diffuse);
	gl.uniform3fv(l1sUniform, light1_specular);
	
	gl.uniform3fv(l2dUniform, light2_diffuse);
	gl.uniform3fv(l2sUniform, light2_specular);
	
	var angleInRadian = degreeToRadian(gAngle);
	var lightPosition=new Float32Array([0.0,0.0,0.0,1.0]);
	lightPosition[0] = RADIUS * Math.cos(gAngle);
	lightPosition[2] = RADIUS * Math.sin(gAngle);
	gl.uniform4fv(gLight0PositionUniform, lightPosition);
	lightPosition[0] = 0.0;
	lightPosition[2] = 0.0;
	//light1
	lightPosition[1] = RADIUS * Math.cos(gAngle);
	lightPosition[2] = RADIUS * Math.sin(gAngle);
	gl.uniform4fv(gLight1PositionUniform, lightPosition);
	lightPosition[1] = 0.0;
	lightPosition[2] = 0.0;
	//light2
	lightPosition[0] = RADIUS * Math.cos(gAngle);
	lightPosition[1] = RADIUS * Math.sin(gAngle);
	gl.uniform4fv(gLight2PositionUniform, lightPosition);
	lightPosition[0] = 0.0;
	lightPosition[1] = 0.0;
	//set material properties
	gl.uniform3fv(kaUniform, material_ambient);
	gl.uniform3fv(kdUniform, material_diffuse);
	gl.uniform3fv(ksUniform, material_specular);
	gl.uniform1f(materialShininessUniform,material_shininess);
	
	var modelMatrix = mat4.create();
	var viewMatrix = mat4.create();
	
	
	mat4.translate(modelMatrix, modelMatrix, [0.0,0.0,-6.0]);
	//mat4.multiply(modelViewMatrix, modelViewMatrix, modelMatrix);
	gl.uniformMatrix4fv(modelMatrixUniform,false,modelMatrix);
	gl.uniformMatrix4fv(viewMatrixUniform,false,viewMatrix);
	gl.uniformMatrix4fv(projectionMatrixUniform,false,perspectiveProjectionMatrix);
	
	sphere.draw();
	
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
			gAngle = gAngle + 0.05;
}
function degreeToRadian(angleInDegree)
{
	return (angleInDegree *  Math.PI/ 180);
}
function mouseDown()
{
	//alert("Mouse is clicked");
	if(gbOnLKeyLight1)
		gbOnLKeyLight1 = false;
	else
		gbOnLKeyLight1 = true;
}

function uninitialize()
{
	if(sphere)
	{
		sphere.deallocate();
		sphere=null;
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