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
var materialProperties;
var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var rotationAxis='x'; //default rotation

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
var RADIUS = 500.0;
var light_ambient = [0.0, 0.0, 0.0];	
var light_diffuse = [1.0, 1.0, 1.0];
var light_specular = [1.0, 1.0, 1.0];

var winWidth,winHeight;
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
	     "#version 300 es"+
         "\n"+
         "in vec4 vPosition;"+
         "in vec3 vNormal;"+
		 "uniform vec4 u_light_position;"+
         "uniform mat4 u_model_matrix;"+
         "uniform mat4 u_view_matrix;"+
         "uniform mat4 u_projection_matrix;"+
         "uniform mediump int u_lighting_enabled;"+
		 "out vec3 transformed_normals;" +
		 "out vec3 light_direction;" +
		 "out vec3 viewer_vector;" +		 
         "void main(void)"+
         "{"+
         "if (u_lighting_enabled == 1)"+
         "{"+
         "vec4 eye_coordinates=u_view_matrix * u_model_matrix * vPosition;"+
         "transformed_normals=mat3(u_view_matrix * u_model_matrix) * vNormal;"+
         "light_direction = vec3(u_light_position) - eye_coordinates.xyz;"+
		 "viewer_vector = -eye_coordinates.xyz;"+         
         "}"+
         "gl_Position=u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;"+
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
		"uniform mediump int u_lighting_enabled;"+
		"in vec3 transformed_normals;" +
		"in vec3 light_direction;" +
		"in vec3 viewer_vector;" +
		"uniform vec3 u_La;" +
		"uniform vec3 u_Ld;" +
		"uniform vec3 u_Ls;" +
		"uniform vec3 u_Ka;" +
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_Ks;" +
		"uniform float u_material_shininess;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"vec3 phong_ads_color;"+
		"if(u_lighting_enabled == 1)" +
		"{" +
		"vec3 normalized_transformed_normals = normalize(transformed_normals);" +
		"vec3 normalized_light_direction = normalize(light_direction);" +
		"vec3 normalized_viewer_vector = normalize(viewer_vector);" +
		"float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction), 0.0);" +
		"vec3 ambient = u_La * u_Ka;" +
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" +
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);" +
		"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector),0.0), u_material_shininess);" +
		"phong_ads_color = ambient + diffuse + specular;" +
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
	ldUniform = gl.getUniformLocation(shaderProgramObject, "u_Ld");
	lsUniform = gl.getUniformLocation(shaderProgramObject, "u_Ls");
	
	kaUniform = gl.getUniformLocation(shaderProgramObject, "u_Ka");
	kdUniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	ksUniform = gl.getUniformLocation(shaderProgramObject, "u_Ks");
	
	gLightPositionUniform = gl.getUniformLocation(shaderProgramObject, "u_light_position");
	
	materialShininessUniform = gl.getUniformLocation(shaderProgramObject, "u_material_shininess");
	
	sphere = new Mesh();
	makeSphere(sphere,2.0,30,30);
	materialProperties = initializeMaterialProperties();
	gl.clearColor(0.25,0.25,0.25,1.0);
	gl.clearDepth(1.0);
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
	winWidth = canvas.width;
	winHeight = canvas.height;
	
	mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width/canvas.height),0.1, 100.0);		
	
	gl.viewport(0,0,canvas.width,canvas.height);
}
var lightPosition=[0.0,0.0,0.0,1.0];	
function draw()
{
	gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);
	gl.useProgram(shaderProgramObject);
	
	//lighting details
	
	gl.uniform1i(onLKeyUniform, 1);
	gl.uniform3fv(laUniform, light_ambient);
	gl.uniform3fv(ldUniform, light_diffuse);
	gl.uniform3fv(lsUniform, light_specular);
	
	
	
	switch(rotationAxis){
			case 'x': //x axis
				lightPosition[0] = 0.0;
				lightPosition[1] = RADIUS * Math.cos(gAngle);
				lightPosition[2] = RADIUS * Math.sin(gAngle);
				break;
				
			case 'y': //y axis
				lightPosition[0] = RADIUS * Math.cos(gAngle);
				lightPosition[1] = 0.0;
				lightPosition[2] = RADIUS * Math.sin(gAngle);
				break;
				
			case 'z': //z axis
				lightPosition[0] = RADIUS * Math.cos(gAngle);
				lightPosition[1] = RADIUS * Math.sin(gAngle);
				lightPosition[2] = 0.0;														
				break;
			default:
				break;
	}
		
	gl.uniform4fv(gLightPositionUniform, lightPosition);
	
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
	
	drawSpheres(24,24);
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
		case 120: //x/X
		case 88:
			rotationAxis='x';
			break;
		case 121: //y/Y
		case 89: 
			rotationAxis='y';
			break;
		case 122: //z/Z
		case 90:
			rotationAxis='z';
			break;
		case 76: //l/L
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

function setMaterialProperties(materialNumber)
{
		var materialIndex= materialNumber*4; //bcz we've 4 properties/material
		var material_ambient=materialProperties[materialIndex];
        var material_diffuse = materialProperties[materialIndex + 1];
        var material_specular = materialProperties[materialIndex + 2];
        var material_shininess = materialProperties[materialIndex + 3][0];//extract shininess
		//set material properties
		gl.uniform3fv(kaUniform,material_ambient.slice(0,3));
		gl.uniform3fv(kdUniform, material_diffuse.slice(0,3));
		gl.uniform3fv(ksUniform, material_specular.slice(0,3));
		gl.uniform1f(materialShininessUniform, material_shininess);
}

function drawSpheres(no_of_spheres, no_of_materials) {

	var windowCenterX = winWidth / 2;
	var windowCenterY = winHeight / 2;
	var viewPortSizeX = winWidth/4;
	var viewPortSizeY = winHeight/6;
	var newAspectRatio = (parseFloat(viewPortSizeX))/(parseFloat(viewPortSizeY));
	var viewPortSizeAspectRatio = parseFloat(viewPortSizeX / viewPortSizeY);
	
	//update perspective according to new aspect ratio
	mat4.perspective(perspectiveProjectionMatrix, 45.0, newAspectRatio, 0.1, 100.0);		
	
	gl.viewport(windowCenterX, windowCenterY, viewPortSizeX, viewPortSizeY);
	
	var y_trans = viewPortSizeY;
	var x_trans = viewPortSizeX;
	var   distanceBetSpheres = 130;
	var currentViewPortX = windowCenterX - x_trans*3; //we have 4 columns, 2-2 from center
	var currentViewPortY = windowCenterY + y_trans*2;  //we have 8 rows but 4 up, 4 down from center
	
	gl.viewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
	
	var i = 0, j = 0;
	for (i = 1, j = 0;i <= no_of_spheres;i++, j++) {

		var local_y_trans = 0;
		if (((i - 1) % 4) == 0 && (i - 1 != 0)) {
			currentViewPortY -= y_trans;
			currentViewPortX = windowCenterX - x_trans*3; //reset X
			gl.viewport( currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
		}
		currentViewPortX += x_trans;
		gl.viewport(currentViewPortX, currentViewPortY, viewPortSizeX, viewPortSizeY);
		
		//set material properties
		if (j <no_of_materials)
			setMaterialProperties(j); //j is material number

		//draws sphere at current model view matrix position
		sphere.draw();
	}
	
}
function initializeMaterialProperties(){
			var materialProperties= [
				//0 Emerald
					[ 0.0215, 0.1745, 0.0215, 1.0 ],//ambient;
					[ 0.07568, 0.61424, 0.07568, 1.0], //diffuse
					[ 0.633, 0.727811, 0.633, 1.0],//specular
					[0.6 * 128],//shininess,
				//1 Jade
					[0.135, 0.2225, 0.1575, 1.0],//ambient;
					[0.54, 0.89, 0.63, 1.0], //diffuse
					[0.316228, 0.316228, 0.316228, 1.0],//specular
					[0.1 * 128],//shininess		
				//2 Obsidian
					[ 0.05375, 0.5, 0.06625, 1.0 ],//ambient;
					[ 0.18275, 0.17, 0.22525, 1.0 ], //diffuse
					[ 0.332741, 0.328634, 0.346435, 1.0 ],//specular
					[0.3 * 128],//shininess						
				//3 Pearl
				[ 0.25, 0.20725, 0.20725, 1.0 ],//ambient;
				[ 1.0, 0.829, 0.829, 1.0 ], //diffuse
				[ 0.296648, 0.296648, 0.296648, 1.0 ],//specular
				[0.88 * 128],//shininess
				//4 Ruby
				[ 0.1745, 0.01175, 0.01175, 1.0 ],//ambient;
				[ 0.61424, 0.04136, 0.04136, 1.0 ], //diffuse
				[ 0.727811, 0.626959, 0.626959, 1.0 ],//specular
				[0.6 * 128],//shininess
				//5 Turquoise
				[ 0.1, 0.18725, 0.1745, 1.0 ],//ambient;
				[ 0.396, 0.74151, 0.69102, 1.0 ], //diffuse
				[ 0.297254, 0.30829, 0.306678, 1.0 ],//specular
				[0.1 * 128],//shininess
				//6 Brass
				[ 0.329412, 0.223529, 0.27451, 1.0 ],//ambient;
				[ 0.78392, 0.568627, 0.113725, 1.0 ], //diffuse
				[ 0.992157, 0.941176, 0.807843, 1.0 ],//specular
				[0.21794872 * 128],//shininess
				//7 Bronze
				[ 0.2125, 0.1275, 0.054, 1.0 ],//ambient;
				[ 0.714, 0.4284, 0.18144, 1.0 ], //diffuse
				[ 0.393548, 0.271906, 0.166721, 1.0 ],//specular
				[0.2 * 128],//shininess
				//8 Chrome
				[ 0.25, 0.25, 0.25, 1.0 ],//ambient;
				[ 0.4, 0.4, 0.4, 1.0 ], //diffuse
				[ 0.774597, 0.774597, 0.774597, 1.0 ],//specular
				[0.6 * 128],//shininess
				//9 Copper
				[ 0.19125, 0.0735, 0.0225, 1.0 ],//ambient;
				[ 0.7038, 0.27048, 0.0828, 1.0 ], //diffuse
				[ 0.25677, 0.137622, 0.086014, 1.0 ],//specular
				[0.1 * 128],//shininess
				//10 Gold
				[ 0.24725, 0.1995, 0.0745, 1.0 ],//ambient;
				[ 0.75164, 0.60648, 0.22648, 1.0 ], //diffuse
				[ 0.628281, 0.555802, 0.366065, 1.0 ],//specular
				[0.4 * 128],//shininess
				//11 Silver
				[ 0.19225, 0.19225, 0.19225, 1.0 ],//ambient;
				[ 0.50745, 0.50745, 0.50745, 1.0 ], //diffuse
				[ 0.508273, 0.508273, 0.508273, 1.0 ],//specular
				[0.4 * 128],//shininess
				//12 Black
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.0, 0.0, 0.0, 1.0 ], //diffuse
				[ 0.50, 0.50, 0.50, 1.0 ],//specular
				[0.25 * 128],//shininess
				//13 Cyan
				[ 0.0, 0.1, 0.06, 1.0 ],//ambient;
				[ 0.0, 0.50980392, 0.50980392, 1.0 ], //diffuse
				[ 0.50196078, 0.50196078, 0.50196078, 1.0 ],//specular
				[0.25 * 128],//shininess
				//14 Green
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.1, 0.35, 0.1, 1.0 ], //diffuse
				[ 0.45, 0.55, 0.45, 1.0 ],//specular
				[0.25 * 128],//shininess
				//15 Red
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.0, 0.0, 1.0 ], //diffuse
				[ 0.7, 0.6, 0.6, 1.0 ],//specular
				[0.25 * 128],//shininess
				//16 White
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.55, 0.55, 0.55, 1.0 ], //diffuse
				[ 0.70, 0.70, 0.70, 1.0 ],//specular
				[0.25 * 128],//shininess
				//17 Yellow Plastic
				[ 0.0, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.5, 0.0, 1.0 ], //diffuse
				[ 0.60, 0.60, 0.50, 1.0 ],//specular
				[0.25 * 128],//shininess
				//18 Black
				[ 0.02, 0.02, 0.02, 1.0 ],//ambient;
				[ 0.01, 0.01, 0.01, 1.0 ], //diffuse
				[ 0.04, 0.04, 0.04, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//19 Cyan
				[ 0.0, 0.05, 0.05, 1.0 ],//ambient;
				[ 0.4, 0.5, 0.5, 1.0 ], //diffuse
				[ 0.04, 0.7, 0.7, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//20 Green
				[ 0.0, 0.05, 0.00, 1.0 ],//ambient;
				[ 0.4, 0.5, 0.4, 1.0 ], //diffuse
				[ 0.04, 0.7, 0.04, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//21 Red
				[ 0.05, 0.0, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.4, 0.4, 1.0 ], //diffuse
				[ 0.7, 0.04, 0.04, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//22 White
				[ 0.05, 0.05, 0.05, 1.0 ],//ambient;
				[ 0.5, 0.5, 0.5, 1.0 ], //diffuse
				[ 0.7, 0.7, 0.7, 1.0 ],//specular
				[0.078125 * 128],//shininess
				//23 Yellow Rubber
				[ 0.05, 0.05, 0.0, 1.0 ],//ambient;
				[ 0.5, 0.5, 0.4, 1.0 ], //diffuse
				[ 0.7, 0.7, 0.04, 1.0 ],//specular
				[0.078125 * 128]//shininess
					
				
			];
			return materialProperties;
	}