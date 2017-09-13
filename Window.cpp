#include "window.h"

#define APPLICATION_NAME L"Generalized Cross Hatching"

static HWND s_hWnd = nullptr;

static TWndProcCallback s_wndProcCallback = nullptr;

LRESULT CALLBACK MessageHandler (HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    if (s_wndProcCallback(hwnd, umsg, wparam, lparam))
        return 1;

    switch (umsg)
    {
	    // Check if the window is being destroyed.
	    case WM_DESTROY:
	    {
		    PostQuitMessage(0);
		    return 0;
	    }

	    // Check if the window is being closed.
	    case WM_CLOSE:
	    {
		    PostQuitMessage(0);
		    return 0;
	    }

        // Check if a key has been pressed on the keyboard.
        case WM_KEYDOWN:
        {
            if ((unsigned int)wparam == 27)
            {
                PostQuitMessage(0);
            }
            return 0;
        }

        // Any other messages send to the default message handler as our application won't make use of them.
        default:
        {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen, TWndProcCallback wndProcCallback)
{
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;

    // store off the callback
    s_wndProcCallback = wndProcCallback;

    // Get the instance of this application.
    HMODULE hinstance = GetModuleHandle(NULL);

    // Setup the windows class with default settings.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = MessageHandler;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = APPLICATION_NAME;
    wc.cbSize = sizeof(WNDCLASSEX);

    // Register the window class.
    RegisterClassEx(&wc);

    // Setup the screen settings depending on whether it is running in full screen or in windowed mode.
    if (fullScreen)
    {
        // Determine the resolution of the clients desktop screen.
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Change the display settings to full screen.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // Set the position of the window to the top left corner.
        posX = posY = 0;
    }
    else
    {
        // If windowed then set it to 800x600 resolution.
        //screenWidth = 800;
        //screenHeight = 600;

        // Place the window in the middle of the screen.
        posX = (GetSystemMetrics(SM_CXSCREEN) - (int)screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - (int)screenHeight) / 2;
    }

    // Create the window with the screen settings and get the handle to it.
    s_hWnd = CreateWindowEx(WS_EX_APPWINDOW, APPLICATION_NAME, APPLICATION_NAME,
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
        posX, posY, (int)screenWidth, (int)screenHeight, NULL, NULL, hinstance, NULL);

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(s_hWnd, SW_SHOW);
    SetForegroundWindow(s_hWnd);
    SetFocus(s_hWnd);
}

HWND WindowGetHWND()
{
    return s_hWnd;
}