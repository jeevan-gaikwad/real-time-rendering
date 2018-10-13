#include<iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <XNAMath/xnamath.h>

#pragma warning(disable: 4838)
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")

#define MY_NAME_INITIAL "JCG"
#define WIDTH  800
#define HEIGHT  600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool gbIsFullScreen = false;
HWND ghwnd;
HDC ghdc;

HGLRC ghrc;
bool gbActiveWindow = false;
bool gbEscapeKeyPressed = false;
FILE *gpFile=NULL;
char gszLogFileName[] = "Log.txt";

float gClearColor[4]; //RGBA
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_PosConstantBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_HSConstantBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_PSConstantBuffer = NULL;
//Tessellation shader
ID3D11HullShader *gpID3D11HullShader = NULL;
ID3D11DomainShader *gpID3D11DomainShader = NULL;

struct CBUFFER_Domain_Shader
{
	XMMATRIX WorldViewProjectionMatrix;
};

struct CBUFFER_Hull_Shader
{
	XMVECTOR Hull_constant_function_param;//used to pass noOfLineSegments and noOfStrips
};

struct CBUFFER_Pixel_Shader
{
	XMVECTOR Line_color;
};
XMMATRIX gPerspectiveProjectionMatrix;
unsigned int gNumberOfLineSegments = 1;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HRESULT initialize();
	void uninitialize();
	void display();

	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("D3D11-Triangle-Perspective");
	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can not be created. \n Exiting."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "Log file is successfully opened.\n");
		fclose(gpFile);
	}

	bool bDone = false;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wndclass)) {
		MessageBox(NULL, TEXT("Failed to register wndclass. Exiting"), TEXT("Error"), MB_OK);
		exit(EXIT_FAILURE);
	}
	iScreenWidth = GetSystemMetrics(0);
	iScreenHeight = GetSystemMetrics(1);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName,
		szAppName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		((iScreenWidth / 2) - 400), ((iScreenHeight / 2) - 300),
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	HRESULT hr = initialize();
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "initialize() failed. Exiting \n");
		fclose(gpFile);
	}
	else {
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "initialize() SUCCEEDED \n");
		fclose(gpFile);
	}

	while (bDone == false) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			display();
			if (gbActiveWindow == true) {
				if (gbEscapeKeyPressed == true) {
					bDone = true;
				}
			}
		}
	}
	uninitialize();
	return((int)msg.wParam);
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void uninitialize();
	HRESULT resize(int, int);
	HRESULT hr;
	static HDC hdc;
	DWORD dwStyle;
	static WINDOWPLACEMENT wpPrev;
	BOOL isWp;
	HMONITOR hMonitor;
	MONITORINFO monitorInfo;
	BOOL isMonitorInfo;
	static bool bIsLKeyPressed = false;
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
				fprintf_s(gpFile, "resize() failed. Exiting \n");
				fclose(gpFile);
				uninitialize();
			}
			else
			{
				fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
				fprintf_s(gpFile, "initialize() SUCCEEDED. Exiting \n");
				fclose(gpFile);
			}
		}

		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam)) {
		case VK_UP:
			gNumberOfLineSegments++;
			if (gNumberOfLineSegments >= 50)
				gNumberOfLineSegments = 50;
			break;
		case VK_DOWN:
			gNumberOfLineSegments--;
			if (gNumberOfLineSegments <= 0)
				gNumberOfLineSegments = 1;
			break;
		case VK_ESCAPE:
			gbEscapeKeyPressed = true;
			break;
		
		case 0x46:	//f
			dwStyle = GetWindowLong(hwnd, GWL_STYLE);
			if (gbIsFullScreen == false) {
				if (dwStyle & WS_OVERLAPPEDWINDOW) {
					wpPrev.length = sizeof(WINDOWPLACEMENT);
					isWp = GetWindowPlacement(hwnd, &wpPrev);
					hMonitor = MonitorFromWindow(hwnd, MONITORINFOF_PRIMARY);
					monitorInfo.cbSize = sizeof(MONITORINFO);
					isMonitorInfo = GetMonitorInfo(hMonitor, &monitorInfo);
					if (isWp == TRUE && isMonitorInfo) {
						SetWindowLong(hwnd, GWL_STYLE, dwStyle&~WS_OVERLAPPEDWINDOW);
						SetWindowPos(hwnd, HWND_TOP,
							monitorInfo.rcMonitor.left,
							monitorInfo.rcMonitor.top,
							monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
							monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
							SWP_NOZORDER | SWP_FRAMECHANGED);
						ShowCursor(FALSE);
						gbIsFullScreen = true;
					}
				}
			}
			else { //restore
				SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
				SetWindowPlacement(hwnd, &wpPrev);
				SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
				ShowCursor(TRUE);
				gbIsFullScreen = false;
			}
			break;

		default:
			break;
		}
		break;
	case WM_CLOSE:
		uninitialize();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

HRESULT CreateConstantBuffer(int sizeOfBuffer, ID3D11Buffer **constantBufferOut,char *bufferName)
{
	HRESULT hr;
	//Define and set constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeOfBuffer;
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, constantBufferOut);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		//fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for %s constant buffer.\n", bufferName);
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		//fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for %s constant buffer.\n", bufferName);
		fclose(gpFile);
	}
}

HRESULT initialize()
{
	HRESULT resize(int, int);

	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE
	};
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;
	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;

	numDriverTypes = sizeof(d3dDriverTypes)/(d3dDriverTypes[0]);
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.Windowed = TRUE;
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			d3dDriverType,
			NULL,
			createDeviceFlags,
			&d3dFeatureLevel_required,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&dxgiSwapChainDesc,
			&gpIDXGISwapChain,
			&gpID3D11Device,
			&d3dFeatureLevel_acquired,
			&gpID3D11DeviceContext
		);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() failed. Exiting \n");
		fclose(gpFile);
	}
	else {
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() SUCCEEDED \n");
		fprintf_s(gpFile,"The Chosen Driver is of ");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile," Hardware Type.\n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, " WARP Type.\n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, " Reference Type.\n");
		}
		else
		{
			fprintf_s(gpFile,"Unknown Type.\n");
		}
		fprintf_s(gpFile, "The Supported Highest Feature Level is:");
		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile,"11.0\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10.1\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10.\n");
		}
		else
		{
			fprintf_s(gpFile,"Unknown Type.\n");
		}

		fclose(gpFile);
	}
	//Initialize shaders, input layouts, constant buffers etc.

	//***********VERTEX SHADER **********************

	const char *vertexShaderSourceCode =
		"struct vertex_output" \
		"{" \
		"float4 position:POSITION;"\
		"};" \
		"vertex_output main(float2 pos: POSITION) : SV_POSITION"\
		"{"\
		"  vertex_output output;" \
		"  output.position = float4(pos,0.0,0.0);"\
		"  return(output);"\
		"}";
		
	ID3DBlob *pID3DBlob_vertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;
	hr = D3DCompile(vertexShaderSourceCode, lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_vertexShaderCode,
		&pID3DBlob_Error);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for vertex shader: %s \n",
				(char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeeded for vertex shader: \n");
		fclose(gpFile);
	}
	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_vertexShaderCode->GetBufferPointer(),
		pID3DBlob_vertexShaderCode->GetBufferSize(), NULL, &gpID3D11VertexShader);
	if (FAILED(hr))
	{
		
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() Failed for vertex shader\n");
		fclose(gpFile);
		return(hr);		
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() succeeded for vertex shader: \n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);

	//TESSELLATION 
	//***********HULL SHADER **********************

	const char *hullShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4 hull_constant_function_param;"\
		"};" \
		"struct  vertex_output"\
		"{" \
		"  float4 position:POSITION;"\
		"};"\
		"struct Hull_Constant_Output"\
		"{"\
		"   float edges[2]:SV_TESSFACTOR;"\
		"};"\
		"Hull_Constant_Output hull_constant_output_function(void)"\
		"{"\
		"   Hull_Constant_Output output;"\
		"   float noOfStrips=hull_constant_function_param[0];"\
		"   float noOfSegments=hull_constant_function_param[1];"\
		"   output.edges[0]=noOfStrips;"\
		"   output.edges[1]=noOfSegments;"\
		"   return(output);"\
		"};"\
		"struct Hull_Output" \
		"{" \
		"   float4 position:POSITION;" \
		"};" \
		"[domain(\"isoline\")]"\
		"[partitioning(\"integer\")]"\
		"[outputtopology(\"line\")]"\
		"[outputcontrolpoints(4)]"\
		"[patchconstantfunc(\"hull_constant_output_function\")]"\
		"Hull_Output main(InputPatch<vertex_output,4> input_patch,uint i:SV_OutputControlPointID)"\
		"{"\
		"    Hull_Output output;"\
		"    output.position = input_patch[i].position;"\
		"    return output;"\
		"}";

	ID3DBlob *pID3DBlob_hullShaderCode = NULL;
	if (pID3DBlob_Error != NULL) {
		pID3DBlob_Error->Release();
		pID3DBlob_Error = NULL;
	}
	hr = D3DCompile(hullShaderSourceCode, lstrlenA(hullShaderSourceCode) + 1,
		"HS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"hs_5_0",
		0,
		0,
		&pID3DBlob_hullShaderCode,
		&pID3DBlob_Error);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for Hull shader: %s \n",
				(char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeeded for Hull shader: \n");
		fclose(gpFile);
	}
	hr = gpID3D11Device->CreateHullShader(pID3DBlob_hullShaderCode->GetBufferPointer(),
		pID3DBlob_hullShaderCode->GetBufferSize(), NULL, &gpID3D11HullShader);
	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateHullShader() Failed\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateHullShader() succeeded.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->HSSetShader(gpID3D11HullShader, 0, 0);

	//***********DOMAIN SHADER **********************

	const char *domainShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldViewProjectionMatrix;"\
		"};" \
		"struct Hull_Output" \
		"{" \
		"   float4 position:POSITION;" \
		"};" \
		"struct Domain_Output"\
		"{"\
		"   float4 position:SV_POSITION;"\
		"};"\
		"struct Hull_Constant_Output"\
		"{"\
		"   float edges[2]:SV_TESSFACTOR;"\
		"};"\
		"[domain(\"isoline\")]"\
		"Domain_Output main(Hull_Constant_Output input,OutputPatch<Hull_Output,4> output_patch,"\
		"float2 tessCoord:SV_DOMAINLOCATION)"\
		"{"\
		"    Domain_Output output;"\
		"    float u=tessCoord.x;"\
		"    float3 p0=output_patch[0].position.xyz;"\
		"    float3 p1=output_patch[1].position.xyz;"\
		"    float3 p2=output_patch[2].position.xyz;"\
		"    float3 p3=output_patch[3].position.xyz;"\
		"	 float u1 = (1.0 - u);" \
		"	 float u2 = u * u;" \
		"	 float b3 = u2 * u;" \
		"	 float b2 = 3.0 * u2* u1;" \
		"	 float b1 = 3.0 * u * u1 * u1; " \
		"	 float b0 = u1 * u1 * u1; " \
		"	 float3 p = p0 * b0 * + p1*b1 + p2*b2 + p3*b3;" \
		"    output.position=mul(worldViewProjectionMatrix,float4(p,1.0));"\
		"    return output;"\
		"}";

	ID3DBlob *pID3DBlob_domainShaderCode = NULL;
	if (pID3DBlob_Error != NULL) {
		pID3DBlob_Error->Release();
		pID3DBlob_Error = NULL;
	}
	hr = D3DCompile(domainShaderSourceCode, lstrlenA(domainShaderSourceCode) + 1,
		"DS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ds_5_0",
		0,
		0,
		&pID3DBlob_domainShaderCode,
		&pID3DBlob_Error);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for Domain shader: %s \n",
				(char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeeded for Domain shader: \n");
		fclose(gpFile);
	}
	hr = gpID3D11Device->CreateDomainShader(pID3DBlob_domainShaderCode->GetBufferPointer(),
		pID3DBlob_domainShaderCode->GetBufferSize(), NULL, &gpID3D11DomainShader);
	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDomainShader() Failed\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDomainShader() succeeded.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->DSSetShader(gpID3D11DomainShader, 0, 0);

	//***********PIXEL SHADER **********************
	const char *pixelShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"	float4 lineColor:COLOR;"\
		"};" \
		"float4 main(float4 pos:SV_POSITION) : SV_TARGET" \
		"{"\
		"	return(lineColor);"\
		"}";

	ID3DBlob *pID3DBlob_pixelShaderCode = NULL;
	if (pID3DBlob_Error != NULL) {
		pID3DBlob_Error->Release();
		pID3DBlob_Error = NULL;
	}
	hr = D3DCompile(pixelShaderSourceCode, lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_pixelShaderCode,
		&pID3DBlob_Error);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for pixel shader: %s \n",
				(char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeeded for Pixel shader: \n");
		fclose(gpFile);
	}
	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_pixelShaderCode->GetBufferPointer(),
		pID3DBlob_pixelShaderCode->GetBufferSize(), NULL, &gpID3D11PixelShader);
	if (FAILED(hr))
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Failed for pixel shader\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() succeeded for pixel shader: \n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_pixelShaderCode->Release();
	pID3DBlob_pixelShaderCode = NULL;

	float vertices[] = 
	{
		-2.0f, -2.0f, -1.0f, 2.0f, 1.0f, -2.0f, 2.0f, 2.0f
	};
	//Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL,
		&gpID3D11Buffer_VertexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for vertex buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for vertex buffer.\n");
		fclose(gpFile);
	}

	//Copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, vertices, sizeof(vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer, NULL);

	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc;
	inputElementDesc.SemanticName = "POSITION";
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc.InputSlot = 0;
	inputElementDesc.AlignedByteOffset = 0;
	inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc.InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(&inputElementDesc, 1, pID3DBlob_vertexShaderCode->GetBufferPointer(),
		pID3DBlob_vertexShaderCode->GetBufferSize(), &gpID3D11InputLayout);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
	pID3DBlob_vertexShaderCode->Release();
	pID3DBlob_vertexShaderCode = NULL;

	CreateConstantBuffer(sizeof(CBUFFER_Domain_Shader), &gpID3D11Buffer_PosConstantBuffer, "DS");
	CreateConstantBuffer(sizeof(CBUFFER_Hull_Shader), &gpID3D11Buffer_HSConstantBuffer,"HS");
	CreateConstantBuffer(sizeof(CBUFFER_Pixel_Shader), &gpID3D11Buffer_PSConstantBuffer, "PS");

	//gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_PosConstantBuffer);
	gpID3D11DeviceContext->DSSetConstantBuffers(0, 1, &gpID3D11Buffer_PosConstantBuffer);

	gpID3D11DeviceContext->HSSetConstantBuffers(0, 1, &gpID3D11Buffer_HSConstantBuffer);
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_PSConstantBuffer);

	gClearColor[0] = 0.0f; //R
	gClearColor[1] = 0.0f; //G
	gClearColor[2] = 0.0f; //B
	gClearColor[3] = 1.0f; //A
	gPerspectiveProjectionMatrix = XMMatrixIdentity();
	//call resize for first time. Mandotory part of initialization code
	hr = resize(WIDTH, HEIGHT);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "resize() failed. Exiting \n");
		fclose(gpFile);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "resize() Succeeded. \n");
		fclose(gpFile);
	}
	return(S_OK);
}
HRESULT resize(int width, int height)
{
	//width = 800;
	//height = 600;
	HRESULT hr = S_OK;
	//free any previosly created, size dependant resources
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}
	//Now resize swap chain buffers according to new window size
	gpIDXGISwapChain->ResizeBuffers(1, //only one buffer
		width, height, //size of the new buffer
		DXGI_FORMAT_R8G8B8A8_UNORM,  // RGB of 8 bit. Unsigned normal
		0 //Use best suitable method to resize the buffer
		);
	//Get resized buffer from the swap chain
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;
	gpIDXGISwapChain->GetBuffer(0, //We want 0th index back buffer
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pID3D11Texture2D_BackBuffer
	);
	//Again get the render target view from d3d11 device using above buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL,
		&gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "D3D11Device::CreateRenderTargetView() failed. Exiting \n");
		fclose(gpFile);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "D3D11Device::CreateRenderTargetView() Succeeded. \n");
		fclose(gpFile);
	}
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

	//Like glViewPort, set D3D view port
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;

	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	
	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), ((float)width / (float)height), 0.1f, 100.0f);
	
	return(hr);
}
void display()
{
	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	//D3D drawing goes here
	// select which vertex buffer to display
	UINT stride = sizeof(float) * 2;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);
	
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	//Model translation and viewing
	XMMATRIX worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(-1.5f, -0.5f, +4.0f);
	XMMATRIX viewMatrix = XMMatrixIdentity();

	//Final world view projection mat
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	// load the data into constant buffer
	CBUFFER_Domain_Shader dsConstantBuffer;
	dsConstantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_PosConstantBuffer, 0, NULL, &dsConstantBuffer, 0, 0);

	CBUFFER_Hull_Shader hsConstantBuffer;
	hsConstantBuffer.Hull_constant_function_param = XMVectorSet(1, gNumberOfLineSegments, 0, 0);
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_HSConstantBuffer, 0, NULL, &hsConstantBuffer, 0, 0);

	CBUFFER_Pixel_Shader psConstantBuffer;
	psConstantBuffer.Line_color = XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f);
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_PSConstantBuffer, 0, NULL, &psConstantBuffer, 0, 0);

	gpID3D11DeviceContext->Draw(4, 0);
	//Swap front and back buffers
	gpIDXGISwapChain->Present(0, //Do we want synchronize buffer with Vertical refresh rate, NO(0)
		0 //How many frames to show, per buffer. We say use default(0)
	);
	TCHAR str[255];
	wsprintf(str, TEXT("OpenGL PP Window: Segments: %d"), gNumberOfLineSegments);
	SetWindowText(ghwnd, str);
}

void uninitialize()
{
	if (gpID3D11Buffer_PosConstantBuffer)
	{
		gpID3D11Buffer_PosConstantBuffer->Release();
		gpID3D11Buffer_PosConstantBuffer = NULL;
	}
	if (gpID3D11Buffer_HSConstantBuffer)
	{
		gpID3D11Buffer_HSConstantBuffer->Release();
		gpID3D11Buffer_HSConstantBuffer = NULL;
	}
	if (gpID3D11Buffer_PSConstantBuffer)
	{
		gpID3D11Buffer_PSConstantBuffer->Release();
		gpID3D11Buffer_PSConstantBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}
	if (gpID3D11Buffer_VertexBuffer)
	{
		gpID3D11Buffer_VertexBuffer->Release();
		gpID3D11Buffer_VertexBuffer = NULL;
	}
	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11DomainShader)
	{
		gpID3D11DomainShader->Release();
		gpID3D11DomainShader = NULL;
	}
	if (gpID3D11HullShader)
	{
		gpID3D11HullShader->Release();
		gpID3D11HullShader = NULL;
	}
	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}
	//Code
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}
	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}
	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}
	if (gpFile)
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "uninitialize() SUCCEEDED. \n");
		fprintf_s(gpFile, "Log file is successfully closed. \n");
		fclose(gpFile);
	}
}
