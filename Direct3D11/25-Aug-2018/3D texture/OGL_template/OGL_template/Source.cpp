#include<iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <XNAMath/xnamath.h>
#include "WICTextureLoader.h"

#pragma warning(disable: 4838)
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")

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
FILE *gpFile = NULL;
char gszLogFileName[] = "Log.txt";

float gClearColor[4]; //RGBA
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_TriangleVertexBuffer = NULL, *gpID3D11Buffer_QuadVertexBuffer;
ID3D11Buffer *gpID3D11Buffer_TriangleTexcoordsBuffer = NULL, *gpID3D11Buffer_QuadTexccordsBuffer = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11RasterizerState *gpID3DRasterizerState = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

float gAngleRotation;
ID3D11ShaderResourceView *gpID3D11ShaderResourceView_Texture_Pyramid = NULL;
ID3D11SamplerState *gpID3D11SamplerState_Texture_Pyramid = NULL;
ID3D11ShaderResourceView *gpID3D11ShaderResourceView_Texture_Cube = NULL;
ID3D11SamplerState *gpID3D11SamplerState_Texture_Cube = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};

XMMATRIX gPerspectiveProjectionMatrix;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HRESULT initialize();
	void uninitialize();
	void display();

	int iScreenWidth, iScreenHeight;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("D3D11-Texture");
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
		return(hr);
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
HRESULT initialize()
{
	HRESULT resize(int, int);
	HRESULT LoadD3DTexture(const wchar_t *, ID3D11ShaderResourceView **);
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

	numDriverTypes = sizeof(d3dDriverTypes) / (d3dDriverTypes[0]);
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
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
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
		return(hr);
	}
	else {
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() SUCCEEDED \n");
		fprintf_s(gpFile, "The Chosen Driver is of ");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, " Hardware Type.\n");
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
			fprintf_s(gpFile, "Unknown Type.\n");
		}
		fprintf_s(gpFile, "The Supported Highest Feature Level is:");
		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0\n");
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
			fprintf_s(gpFile, "Unknown Type.\n");
		}

		fclose(gpFile);
	}
	//Initialize shaders, input layouts, constant buffers etc.

	//***********VERTEX SHADER **********************

	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldViewProjectionMatrix;"\
		"}" \
		"struct vertex_output" \
		"{" \
		"float4 position:SV_POSITION;" \
		"float2 texcoord : TEXCOORD;" \
		"};" \
		"vertex_output main(float4 pos : POSITION, float2 texcoord : TEXCOORD)" \
		"{"\
		"vertex_output output;" \
		"output.position = mul(worldViewProjectionMatrix, pos);"\
		"output.texcoord = texcoord;" \
		"return(output);"\
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

	//***********PIXEL SHADER **********************
	const char *pixelShaderSourceCode =
		"Texture2D myTexture2D;" \
		"SamplerState mySamplerState;" \
		"float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET" \
		"{" \
		"float4 color = myTexture2D.Sample(mySamplerState, texcoord);" \
		"return(color);" \
		"}";

	ID3DBlob *pID3DBlob_pixelShaderCode = NULL;
	pID3DBlob_Error = NULL;
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
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() Failed for vertex shader\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() succeeded for vertex shader: \n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_pixelShaderCode->Release();
	pID3DBlob_pixelShaderCode = NULL;

	//Triangle vertices
	float pyramidVertices[] =
	{
		// triangle of front side
    0.0f, 1.0f, 0.0f,    // front-top
    -1.0f, -1.0f, 1.0f,  // front-left
    1.0f, -1.0f, 1.0f,   // front-right
    
    // triangle of right side
    0.0f, 1.0f, 0.0f,    // right-top
    1.0f, -1.0f, 1.0f,   // right-left
    1.0f, -1.0f, -1.0f,  // right-right
    
    // triangle of back side
    0.0f, 1.0f, 0.0f,    // back-top
    1.0f, -1.0f, -1.0f,  // back-right
    -1.0f, -1.0f, -1.0f, // back-left
    
    // triangle of left side
    0.0f, 1.0f, 0.0f,    // left-top
    -1.0f, -1.0f, -1.0f, // left-left
    -1.0f, -1.0f, 1.0f,  // left-right

	};

	float pyramidTexcoords[] =
	{
		0.5, 1.0, // front-top
		0.0, 0.0, // front-left
		1.0, 0.0, // front-right

		0.5, 1.0, // right-top
		1.0, 0.0, // right-left
		0.0, 0.0, // right-right

		0.5, 1.0, // back-top
		1.0, 0.0, // back-left
		0.0, 0.0, // back-right

		0.5, 1.0, // left-top
		0.0, 0.0, // left-left
		1.0, 0.0, // left-right
	};
	//Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(pyramidVertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL,
		&gpID3D11Buffer_TriangleVertexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for triangle vertex buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for triangle vertex buffer.\n");
		fclose(gpFile);
	}

	//Copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_TriangleVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, pyramidVertices, sizeof(pyramidVertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_TriangleVertexBuffer, NULL);

	//create buffer for Tex coords
	D3D11_BUFFER_DESC bufferDescTexcoords;
	ZeroMemory(&bufferDescTexcoords, sizeof(D3D11_BUFFER_DESC));
	bufferDescTexcoords.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescTexcoords.ByteWidth = sizeof(float) * ARRAYSIZE(pyramidTexcoords);
	bufferDescTexcoords.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescTexcoords.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpID3D11Device->CreateBuffer(&bufferDescTexcoords, NULL,
		&gpID3D11Buffer_TriangleTexcoordsBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for texture buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for texture buffer.\n");
		fclose(gpFile);
	}
	D3D11_MAPPED_SUBRESOURCE mappedSubresourceTexcoordsTriangle;
	//Copy color info into the color buffer
	ZeroMemory(&mappedSubresourceTexcoordsTriangle, sizeof(D3D11_MAPPED_SUBRESOURCE));

	gpID3D11DeviceContext->Map(gpID3D11Buffer_TriangleTexcoordsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceTexcoordsTriangle);
	memcpy(mappedSubresourceTexcoordsTriangle.pData, pyramidTexcoords, sizeof(pyramidTexcoords));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_TriangleTexcoordsBuffer, 0);

	//Quad drawing
	float cubeVertices[] =
	{
		// SIDE 1 ( TOP )
		// triangle 1
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		// triangle 2
		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,

		// SIDE 2 ( BOTTOM )
		// triangle 1
		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, -1.0f, -1.0f,
		// triangle 2
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,

		// SIDE 3 ( FRONT )
		// triangle 1
		-1.0f, +1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		// triangle 2
		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,

		// SIDE 4 ( BACK )
		// triangle 1
		+1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,
		// triangle 2
		-1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,

		// SIDE 5 ( LEFT )
		// triangle 1
		-1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		// triangle 2
		-1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		// SIDE 6 ( RIGHT )
		// triangle 1
		+1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, -1.0f, +1.0f,
		// triangle 2
		+1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
	};
	float cubeTexcoords[] =
	{
		// SIDE 1 ( TOP )
		// triangle 1
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,

		// SIDE 2 ( BOTTOM )
		// triangle 1
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,

		// SIDE 3 ( FRONT )
		// triangle 1
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,

		// SIDE 4 ( BACK )
		// triangle 1
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,

		// SIDE 5 ( LEFT )
		// triangle 1
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,

		// SIDE 6 ( RIGHT )
		// triangle 1
		+0.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +0.0f,
		// triangle 2
		+1.0f, +0.0f,
		+0.0f, +1.0f,
		+1.0f, +1.0f,
	};

	//Reuse vertex buffer	
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(cubeVertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL,
		&gpID3D11Buffer_QuadVertexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for quad vertex buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for quad vertex buffer.\n");
		fclose(gpFile);
	}
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_QuadVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, cubeVertices, sizeof(cubeVertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_QuadVertexBuffer, NULL);

	//Colors for QUAD

	//create buffer for COLORS
	ZeroMemory(&bufferDescTexcoords, sizeof(D3D11_BUFFER_DESC));
	bufferDescTexcoords.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescTexcoords.ByteWidth = sizeof(float) * ARRAYSIZE(cubeTexcoords);
	bufferDescTexcoords.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescTexcoords.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpID3D11Device->CreateBuffer(&bufferDescTexcoords, NULL,
		&gpID3D11Buffer_QuadTexccordsBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for quad color buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for quad color buffer.\n");
		fclose(gpFile);
	}
	//Copy color info into the color buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresourceColorsQuad;
	ZeroMemory(&mappedSubresourceColorsQuad, sizeof(D3D11_MAPPED_SUBRESOURCE));

	gpID3D11DeviceContext->Map(gpID3D11Buffer_QuadTexccordsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceColorsQuad);
	memcpy(mappedSubresourceColorsQuad.pData, cubeTexcoords, sizeof(cubeTexcoords));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_QuadTexccordsBuffer, 0);

	//create and set input layout
	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	inputElementDesc[1].SemanticName = "TEXCOORD";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(inputElementDesc, _ARRAYSIZE(inputElementDesc), pID3DBlob_vertexShaderCode->GetBufferPointer(),
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

	//Define and set constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for constant buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for constant buffer.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);


	//*** TURN OFF Backface culling ***
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	gpID3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3DRasterizerState);

	gpID3D11DeviceContext->RSSetState(gpID3DRasterizerState);
	//****Backface culling DONE *********
	//Load Pyramid texture 
	hr = LoadD3DTexture(L"Stone.bmp", &gpID3D11ShaderResourceView_Texture_Pyramid);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "LoadD3DTexture() failed. Exiting \n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "LoadD3DTexture() Succeeded. \n");
		fclose(gpFile);
	}
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Pyramid);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "gpID3D11Device->CreateSamplerState() failed for Pyramid texture. Exiting \n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "gpID3D11Device->CreateSamplerState() Succeeded for Pyramid texture. \n");
		fclose(gpFile);
	}
	//Cube texture
	hr = LoadD3DTexture(L"Kundali.bmp", &gpID3D11ShaderResourceView_Texture_Cube);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "LoadD3DTexture() failed. Exiting \n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "LoadD3DTexture() Succeeded. \n");
		fclose(gpFile);
	}

	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Cube);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "gpID3D11Device->CreateSamplerState() failed for cube texuture. Exiting \n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "gpID3D11Device->CreateSamplerState() Succeeded for cube texuture. \n");
		fclose(gpFile);
	}
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
		return(hr);
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
	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}
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
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "D3D11Device::CreateRenderTargetView() Succeeded. \n");
		fclose(gpFile);
	}
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	//Create Depth stencil view(buffer)
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer;
	hr=gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	//Create  depth stencil view from aove depth stencil buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		DWORD errCode = GetLastError();
		TCHAR *err;
		int msgSize;
		if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			(LPTSTR)&err,
			0,
			NULL))
			return hr;
		//static char buffer[1024];
		//_snprintf(buffer, sizeof(buffer), "ERROR: %s: %s\n", "Error occurred:", err);
		//fprintf_s(gpFile, wcrtomb_s(sizeof(e);
		fprintf_s(gpFile, "gpID3D11Device->CreateDepthStencilView() failed. Exiting \n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "gpID3D11Device->CreateDepthStencilView() Succeeded. \n");
		fclose(gpFile);
	}
	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

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
	void update();
	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//D3D drawing goes here
	// select which vertex buffer to display
	UINT stridePosition = sizeof(float) * 3;
	UINT strideTexccords = sizeof(float) * 2;
	UINT offset = 0;
	//Triangle drawing
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_TriangleVertexBuffer, &stridePosition, &offset);
	//Pass color data
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_TriangleTexcoordsBuffer, &strideTexccords, &offset);

	// Bind Pyramid texture to Pixel shader
	gpID3D11DeviceContext->PSSetShaderResources(0, 1,
		&gpID3D11ShaderResourceView_Texture_Pyramid);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Pyramid);
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Model translation and viewing
	XMMATRIX worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(-1.5f, 0.0, 6.0f);  //Shift triangle to slightly left
	worldMatrix = XMMatrixRotationY(XMConvertToRadians(gAngleRotation)) * worldMatrix;
	XMMATRIX viewMatrix = XMMatrixIdentity();

	//Final world view projection mat
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	// load the data into constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	gpID3D11DeviceContext->Draw(12, 0);

	//******Quad drawing using TRIANGLE LIST **********************
	//Pass vertices data
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_QuadVertexBuffer, &stridePosition, &offset);
	//Pass color data
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_QuadTexccordsBuffer, &strideTexccords, &offset);
	// Bind Pyramid texture to Pixel shader
	gpID3D11DeviceContext->PSSetShaderResources(0, 1,
		&gpID3D11ShaderResourceView_Texture_Cube);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Cube);
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Model translation and viewing
	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(1.5f, 0.0, 6.0f); ////Shift triangle to slightly right
	worldMatrix = XMMatrixScaling(0.75f, 0.75f, 0.75f) * worldMatrix;
	worldMatrix = XMMatrixRotationX(XMConvertToRadians(gAngleRotation)) * worldMatrix;
	worldMatrix = XMMatrixRotationY(XMConvertToRadians(gAngleRotation)) * worldMatrix;
	worldMatrix = XMMatrixRotationZ(XMConvertToRadians(gAngleRotation)) * worldMatrix;
	viewMatrix = XMMatrixIdentity();

	//Final world view projection mat
	wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	// load the data into constant buffer
	ZeroMemory(&constantBuffer, sizeof(CBUFFER));
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	gpID3D11DeviceContext->Draw(6, 0);
	gpID3D11DeviceContext->Draw(6, 6);
	gpID3D11DeviceContext->Draw(6, 12);
	gpID3D11DeviceContext->Draw(6, 18);
	gpID3D11DeviceContext->Draw(6, 24);
	gpID3D11DeviceContext->Draw(6, 30);
	//Swap front and back buffers
	gpIDXGISwapChain->Present(0, //Do we want synchronize buffer with Vertical refresh rate, NO(0)
		0 //How many frames to show, per buffer. We say use default(0)
	);
	update();// Update changing resources
}
void update()
{
	if (gAngleRotation > 360.0f)
	{
		gAngleRotation = 0.0f;
	}
	else
		gAngleRotation += 0.01f;
}
HRESULT LoadD3DTexture(const wchar_t* textureFileName, ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
	HRESULT hr;
	hr = DirectX::CreateWICTextureFromFile(gpID3D11Device, gpID3D11DeviceContext, textureFileName, NULL,
		ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "DirectX::CreateWICTextureFromFile() failed. Exiting \n");
		fclose(gpFile);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+"); //append mode
		fprintf_s(gpFile, "DirectX::CreateWICTextureFromFile() Succeeded. \n");
		fclose(gpFile);
	}
	return hr;
}
void uninitialize()
{
	if (gpID3D11SamplerState_Texture_Cube)
	{
		gpID3D11SamplerState_Texture_Cube->Release();
		gpID3D11SamplerState_Texture_Cube = NULL;
	}
	if (gpID3D11ShaderResourceView_Texture_Cube)
	{
		gpID3D11ShaderResourceView_Texture_Cube->Release();
		gpID3D11ShaderResourceView_Texture_Cube = NULL;
	}
	if (gpID3D11SamplerState_Texture_Pyramid)
	{
		gpID3D11SamplerState_Texture_Pyramid->Release();
		gpID3D11SamplerState_Texture_Pyramid = NULL;
	}
	if (gpID3D11ShaderResourceView_Texture_Pyramid)
	{
		gpID3D11ShaderResourceView_Texture_Pyramid->Release();
		gpID3D11ShaderResourceView_Texture_Pyramid = NULL;
	}
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}
	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}
	if (gpID3D11Buffer_TriangleVertexBuffer)
	{
		gpID3D11Buffer_TriangleVertexBuffer->Release();
		gpID3D11Buffer_TriangleVertexBuffer = NULL;
	}
	if (gpID3D11Buffer_TriangleTexcoordsBuffer)
	{
		gpID3D11Buffer_TriangleTexcoordsBuffer->Release();
		gpID3D11Buffer_TriangleTexcoordsBuffer = NULL;
	}
	if (gpID3D11Buffer_QuadVertexBuffer)
	{
		gpID3D11Buffer_QuadVertexBuffer->Release();
		gpID3D11Buffer_QuadVertexBuffer = NULL;
	}
	if (gpID3D11Buffer_QuadTexccordsBuffer)
	{
		gpID3D11Buffer_QuadTexccordsBuffer->Release();
		gpID3D11Buffer_QuadTexccordsBuffer = NULL;
	}
	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}
	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
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
