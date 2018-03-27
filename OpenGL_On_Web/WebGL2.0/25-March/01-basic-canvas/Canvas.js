//on body load function
function main()
{
	//get canvas elementFromPoint
	var canvas = document.getElementById("AMC");
	if(!canvas)
		console.log("Obtaining canvas from main document failed\n");
	else
		console.log("Obtaining canvas from main document succeeded\n");
	//print obtained canvas width and height on console
	console.log("Canvas width:" + canvas.width +" height:" +canvas.height);
	
	//Get normal 2D contextual
	var context=canvas.getContext("2d");
	if(!context)
		console.log("Obtaining 2D context failed\n");
	else
		console.log("Obtaining 2D context succeeded\n");
	
	//fill canvas with black color
	context.fillStyle = "black"; //we can also provide values in hex "#000000"
	context.fillRect(0,0,canvas.width, canvas.height);
	//clear the text
	context.textAlign="center";
	context.textBaseline = "middle";
	
	//text
	var str="Hello world!";
	
	//text font
	context.font="48px sans-serif";
	
	//text color
	context.fillStyle="white";
	
	//display text in center
	context.fillText(str,canvas.width/2, canvas.height/2);
}