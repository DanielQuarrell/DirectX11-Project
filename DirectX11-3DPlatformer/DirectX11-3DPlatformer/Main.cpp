#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <DXErr.h>
#include <DirectXMath.h>

//Include Direct3D Library files

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "DXErr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib") //For error checking only

//Global Variables

IDXGISwapChain* swapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;

ID3D11RenderTargetView* renderTargetView;
ID3D11Buffer* pIndexBuffer;	//Buffer index to define triangles
ID3D11Buffer* pVertBuffer;	//Buffer that will hold vertex data
ID3D11DepthStencilView* pDepthStencilView;
ID3D11DepthStencilState* pDepthStecilState;
ID3D11Texture2D* pDepthStencilBuffer;

//Object specific
ID3D11Buffer* pConstantBuffer;		//Constant buffer interface
ID3D11Buffer* pConstantBuffer2;		//Constant buffer interface

ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;
ID3D10Blob* VS_Buffer;			//Information about the vertex shader
ID3D10Blob* PS_Buffer;			//Information about the pixel shader
ID3D11InputLayout* vertexLayout;

LPCTSTR WndClassName = L"firstwindow";	//Window Name
HWND hwnd = NULL;						//Window Handle
HRESULT hResult;						//Used for error checking

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

//Function Prototypes
bool InitializeDirect3d11(HINSTANCE hInstance);
void ReleaseObjects();
bool InitScene();
void UpdateScene();
void RenderScene();

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width,
	int height,
	bool windowed);

int messageloop();

//Initialise Windows Callback Procedure
LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

struct Vertex
{
	D3DXVECTOR3 position;

	Vertex() {}
	Vertex(D3DXVECTOR3 _position)
		: position(_position) {}
};

//The input-layout description
D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
};
UINT numElements = ARRAYSIZE(inputElementDesc);

//Main windows function - application start
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	if (!InitializeWindow(hInstance, nShowCmd, WINDOW_WIDTH, WINDOW_HEIGHT, true))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitializeDirect3d11(hInstance))
	{
		MessageBox(0, L"Direct3D Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitScene())
	{
		MessageBox(0, L"Scene Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	messageloop();

	ReleaseObjects();

	return 0;
}

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width,
	int height,
	bool windowed)
{
	//Extended Windows Class
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);					//Set to the size of the windows class
	wc.style = CS_HREDRAW | CS_VREDRAW;				//Redrawn when window moves or changes size
	wc.lpfnWndProc = WndProc;						//Default windows procedure function
	wc.cbClsExtra = NULL;							//Bytes after WNDCLASSEX
	wc.cbWndExtra = NULL;							//Bytes after Windows Instance
	wc.hInstance = hInstance;						//Instance of the windows application 
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);		//Loads an icon for the window
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);		//Loads cursor type
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);	//Background colour of window
	wc.lpszMenuName = NULL;							//Name attached to the window
	wc.lpszClassName = WndClassName;				//Name of the window class
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);	//Loads an icon for the taskbar

	//Register the windows class
	if (!RegisterClassEx(&wc))
	{
		//If registration fails, display error
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	//Create the window
	hwnd = CreateWindowEx(
		NULL,							//Extended Style
		WndClassName,					//Name of the window class used
		L"DirectX11-3DPlatformer",		//Name in title of window
		WS_OVERLAPPEDWINDOW,			//Style of the window
		CW_USEDEFAULT, CW_USEDEFAULT,	//Starting X and Y position of window
		width, height,					//Width and Height of window
		NULL,							//Handle to the Parent of the window
		NULL,							//Handle to the Menu attached to the window
		hInstance,						//Current Program instance
		NULL							//Used for an MDI client window
	);

	//Check if the window was created
	if (!hwnd)
	{
		//If registration fails, display error
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

bool InitializeDirect3d11(HINSTANCE hInstance)
{
	//Buffer Description - struct
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC)); //Cleared the data structure and set it to null

	bufferDesc.Width = WINDOW_WIDTH;
	bufferDesc.Height = WINDOW_HEIGHT;
	bufferDesc.RefreshRate.Numerator = 60;								//Refresh rate in hertz - 60/1 - 60hz
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//Use 32-bit colour
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //Scanline order is unspecified.
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					//Unspecified image scaling 

	//Swap Chain Description - struct

	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC)); //Cleared the data structure and set it to null

	swapChainDesc.BufferDesc = bufferDesc;							//Buffer Description
	swapChainDesc.SampleDesc.Count = 1;								//Number of multisamples
	swapChainDesc.SampleDesc.Quality = 0;							//The image quality level. The higher the quality, the lower the performance. The valid range is between zero and one less than the level returned by ID3D11Device::CheckMultisampleQualityLevels
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//How the swap chain is going to be used
	swapChainDesc.BufferCount = 1;									//Number of back buffers in the chain
	swapChainDesc.OutputWindow = hwnd;								//Handle to app window
	swapChainDesc.Windowed = TRUE;									//Windowed or full screen
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			//Lets the display driver handle the front buffer before swapping it out with the back buffer


	//Create the Swapchain
	hResult = D3D11CreateDeviceAndSwapChain(
		NULL,									//Pointer to the Graphics Card / Video adapter to use
		D3D_DRIVER_TYPE_HARDWARE,				//Parameter to determine if Hardware or Software should be used for rendering
		NULL,									//HMODULE handle to a DLL that's used to impliment software rasterizing
		NULL,									//Flags for different modes such as debugging
		NULL,									//Pointer for pFeatureLevels (video card features), NULL for highest
		NULL,									//Number of elements in the pFeatureLevels array
		D3D11_SDK_VERSION,						//SDK version
		&swapChainDesc,
		&swapChain,
		&d3d11Device,
		NULL,									//Pointer to D3D_FEATURE_LEVEL (Used for backwards compatibility)
		&d3d11DevCon);

	if (FAILED(hResult))
	{
		MessageBox(NULL, DXGetErrorDescription(hResult),
			TEXT("D3D11CreateDeviceAndSwapChain"), MB_OK);
		return 0;
	}

	//Create the back buffer
	ID3D11Texture2D* backBuffer;
	hResult = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

	if (FAILED(hResult))
	{
		MessageBox(NULL, DXGetErrorDescription(hResult),
			TEXT("swapChain->GetBuffer"), MB_OK);
		return 0;
	}

	//Create render target
	hResult = d3d11Device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
	backBuffer->Release();

	if (FAILED(hResult))
	{
		MessageBox(NULL, DXGetErrorDescription(hResult),
			TEXT("d3d11Device->CreateRenderTargetView"), MB_OK);
		return 0;
	}

	//Create the depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthStecilDesc;
	ZeroMemory(&depthStecilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthStecilDesc.DepthEnable = TRUE;
	depthStecilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStecilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	hResult = d3d11Device->CreateDepthStencilState(&depthStecilDesc, &pDepthStecilState);
	d3d11DevCon->OMSetDepthStencilState(pDepthStecilState, 1);

	if (FAILED(hResult))
	{
		MessageBox(NULL, DXGetErrorDescription(hResult),
			TEXT("CreateDepthStencilState"), MB_OK);
		return 0;
	}

	//Describe the Depth/Stencil Buffer - Depth map
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

	depthBufferDesc.Width = WINDOW_WIDTH;
	depthBufferDesc.Height = WINDOW_HEIGHT;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferDesc.SampleDesc.Count = 1;					//anti-aliasing 
	depthBufferDesc.SampleDesc.Quality = 0;					//anti-aliasing 
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hResult = d3d11Device->CreateTexture2D(&depthBufferDesc, nullptr, &pDepthStencilBuffer);

	if (FAILED(hResult))
	{
		MessageBox(NULL, DXGetErrorDescription(hResult),
			TEXT("d3d11Device->CreateTexture2D"), MB_OK);
		return 0;
	}

	//Create the Depth/Stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	ZeroMemory(&depthViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice = 0;

	hResult = d3d11Device->CreateDepthStencilView(pDepthStencilBuffer, &depthViewDesc, &pDepthStencilView);

	if (FAILED(hResult))
	{
		MessageBox(NULL, DXGetErrorDescription(hResult),
			TEXT("d3d11Device->CreateDepthStencilView"), MB_OK);
		return 0;
	}

	//Set render target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, pDepthStencilView);

	return true;
}

//Closes and Cleans up Direct3D by releasing all the COM objects
void ReleaseObjects()
{
	swapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	pVertBuffer->Release();
	pIndexBuffer->Release();
	vertexShader->Release();
	pixelShader->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertexLayout->Release();
	pDepthStecilState->Release();
	pDepthStencilView->Release();
	pDepthStencilBuffer->Release();
	pConstantBuffer->Release();
}

//Initialize the game scene
bool InitScene()
{
	//Compile Shaders from the shader file
	hResult = D3DX11CompileFromFile(L"Shaders.fx", 0, 0, "VS", "vs_5_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hResult = D3DX11CompileFromFile(L"Shaders.fx", 0, 0, "PS", "ps_5_0", 0, 0, 0, &PS_Buffer, 0, 0);

	//Create the Shader Objects
	hResult = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &vertexShader);
	hResult = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &pixelShader);

	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(vertexShader, 0, 0);
	d3d11DevCon->PSSetShader(pixelShader, 0, 0);

	//Create the vertex buffer
	Vertex verticies[] =
	{
		Vertex{D3DXVECTOR3(-0.5f, -0.5f, -0.5f)},
		Vertex{D3DXVECTOR3(0.5f, -0.5f, -0.5f)},
		Vertex{D3DXVECTOR3(-0.5f,  0.5f, -0.5f)},
		Vertex{D3DXVECTOR3(0.5f,  0.5f, -0.5f)},
		Vertex{D3DXVECTOR3(-0.5f, -0.5f,  0.5f)},
		Vertex{D3DXVECTOR3(0.5f, -0.5f,  0.5f)},
		Vertex{D3DXVECTOR3(-0.5f,  0.5f,  0.5f)},
		Vertex{D3DXVECTOR3(0.5f,  0.5f,  0.5f)}
	};

	DWORD indices[] = {
		0, 2, 1,	2, 3, 1,
		1, 3, 5,	3, 7, 5,
		2, 6, 3,	3, 6, 7,
		4, 5, 7,	4, 7, 6,
		0, 4, 2,	2, 4, 6,
		0, 1, 4,	1, 5, 4
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;								//Reads and writes to the GPU
	indexBufferDesc.ByteWidth = sizeof(indices);				//Byte size = DWORD type * 2 triangles * 3 verticies for each triangle
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;						//Use buffer as a index buffer
	indexBufferDesc.CPUAccessFlags = 0;											//Defines how a CPU can access a resource
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;

	d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &pIndexBuffer);	//Create the buffer
	d3d11DevCon->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);		//Bind it to the IA stage of the graphics pipeline

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;								//Reads and writes to the GPU 
	vertexBufferDesc.ByteWidth = sizeof(verticies);	//Byte Size = Vertex struct * 3 since there is 3 elements in the array
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;						//Use buffer as a vertex buffer
	vertexBufferDesc.CPUAccessFlags = 0;										//Defines how a CPU can access a resource
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));										//Clear the memory in the vertex buffer
	vertexBufferData.pSysMem = verticies;													//The data to place into the buffer				
	hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pVertBuffer);	//Create the buffer

	//Create the Input Layout
	hResult = d3d11Device->CreateInputLayout(inputElementDesc, numElements, VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(), &vertexLayout);

	//Set the Input Layout
	d3d11DevCon->IASetInputLayout(vertexLayout);

	//Select which vertex buffer to display
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &pVertBuffer, &stride, &offset);

	//Select which primtive type we are using
	d3d11DevCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WINDOW_WIDTH;
	viewport.Height = WINDOW_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	d3d11DevCon->RSSetViewports(1, &viewport);

	// Create constant buffer for transformation matrix
	struct ConstantBufferTransform
	{
		DirectX::XMMATRIX transformation;
	};
	const ConstantBufferTransform cb
	{
		{
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixRotationZ(1) *
				DirectX::XMMatrixRotationX(1) *
				DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) *
				DirectX::XMMatrixTranslation(0, 0, 4.0f) *
				DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f) // 3/4 to match the aspect ratio
			)
		}
	};

	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;				//CPU write, GPU read
	constantBufferDesc.ByteWidth = sizeof(cb);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//Bind to the vertex shader file
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPU write as its going to be updated every frame
	constantBufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;

	hResult = d3d11Device->CreateBuffer(&constantBufferDesc, &csd, &pConstantBuffer);

	struct ConstantBuffer2
	{
		D3DXCOLOR face_colors[6];
	};
	const ConstantBuffer2 cb2 =
	{
		{
			{D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f)},
			{D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
			{D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
			{D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)},
			{D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)},
			{D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f)},
		}
	};

	D3D11_BUFFER_DESC constantBuffer2Desc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constantBuffer2Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer2Desc.Usage = D3D11_USAGE_DEFAULT;				//GPU read / write
	constantBuffer2Desc.ByteWidth = sizeof(cb);
	constantBuffer2Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//Bind to the vertex shader file
	constantBuffer2Desc.CPUAccessFlags = 0;
	constantBuffer2Desc.MiscFlags = 0;
	constantBuffer2Desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA csd2 = {};
	csd2.pSysMem = &cb2;

	hResult = d3d11Device->CreateBuffer(&constantBuffer2Desc, &csd2, &pConstantBuffer2);

	return true;
}

//Update the game scene
void UpdateScene()
{

}

void RenderScene()
{
	//Clear our backbuffer to the updated color
	D3DXCOLOR backgroundColour(0.0f, 0.2f, 0.4f, 1.0f);
	d3d11DevCon->ClearRenderTargetView(renderTargetView, backgroundColour);
	//Clear the depth buffer
	d3d11DevCon->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Bind buffer to the vertex and pixel shaders
	d3d11DevCon->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	d3d11DevCon->PSSetConstantBuffers(0, 1, &pConstantBuffer2);

	//Number of indices that need to be drawn, offset from the begining of the index array, offset from the begining of the vertices array
	d3d11DevCon->DrawIndexed(36, 0, 0);

	//Present the back buffer to the screen
	swapChain->Present(0, 0);
}

int messageloop()
{
	MSG msg;

	ZeroMemory(&msg, sizeof(MSG)); //Cleared the data structure and set it to null

	//While a message is displayed
	while (true)
	{
		//Check for messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//Close program on quit message
			if (msg.message == WM_QUIT)
			{
				break;
			}
			//Translates message
			TranslateMessage(&msg);
			//Send to windows procedure (WndProc)
			DispatchMessage(&msg);
		}
		else
		{
			//Run Game code

			UpdateScene();
			RenderScene();
		}
	}

	return msg.wParam; //Return the message
}

//Default windows procedure for processing
LRESULT CALLBACK WndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{

				if (MessageBox(0, L"Are you sure you want to exit?",
					L"Exit message", MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					DestroyWindow(hwnd);
				}
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(
		hwnd,
		msg,
		wParam,
		lParam);
}


