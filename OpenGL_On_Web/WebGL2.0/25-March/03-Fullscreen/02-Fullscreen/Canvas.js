//global variables
var canvas=null;
var context=null;

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
	
	//Get normal 2D contextual
	context=canvas.getContext("2d");
	if(!context)
		console.log("Obtaining 2D context failed\n");
	else
		console.log("Obtaining 2D context succeeded\n");
	
	//fill canvas with black color
	context.fillStyle = "black"; //we can also provide values in hex "#000000"
	context.fillRect(0,0,canvas.width, canvas.height);
	
	drawText("Hello world!!");
	//register keyboard and mouse event with window class
	window.addEventListener("keydown", keydown, false);
	window.addEventListener("click", mouseDown, false);
}

function drawText(text)
{
	//code to draw the text in center
	//clear the text
	context.textAlign="center";
	context.textBaseline = "middle";
		
	//text font
	context.font="48px sans-serif";
	
	//text color
	context.fillStyle="white";
	
	//display text in center
	context.fillText(text,canvas.width/2, canvas.height/2);	
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
		case 70: //for 'F' or 'f'
			toggleFullScreen();
			
			//repaint
			drawText("Hello world!!!");
			break;
	}
}

function mouseDown()
{
	alert("Mouse is clicked");
}
