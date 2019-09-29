#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <DXErr.h>

//Include Direct3D Library files

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "DXErr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

//Global Variables

IDXGISwapChain*			swapChain;
ID3D11Device*			d3d11Device;
ID3D11DeviceContext*	d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

LPCTSTR WndClassName = L"firstwindow";	//Window Name
HWND hwnd = NULL;						//Window Handle

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
	//Used for error checking
	HRESULT hResult;

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
	swapChainDesc.SampleDesc.Count = 4;								//Number of multisamples
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

	//Set render target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);

	/*
	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = WINDOW_WIDTH;
	viewport.Height = WINDOW_HEIGHT;

	d3d11DevCon->RSSetViewports(1, &viewport);
	*/

	return true;
}

//Closes and Cleans up Direct3D by releasing all the COM objects
void ReleaseObjects()
{
	swapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
}

//Initialize the game scene
bool InitScene()
{
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


