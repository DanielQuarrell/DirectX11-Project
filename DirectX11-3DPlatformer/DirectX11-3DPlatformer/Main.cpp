#include <windows.h>

LPCTSTR WndClassName = L"firstwindow";	//Window Name
HWND hwnd = NULL;						//Window Handle

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

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

	messageloop();

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
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);	//Background colour of window
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
		L"Window Title",				//Name in title of window
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
			//Run Game
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
					L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
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


