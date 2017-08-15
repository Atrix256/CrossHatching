#include <windows.h>
#include "d3d11.h"
#include "Shader.h"
#include "Model.h"

#define APPLICATION_NAME L"Generalized Cross Hatching"

// TODO: not global!
HWND g_hWnd = nullptr;

// TODO: handle input!
LRESULT CALLBACK MessageHandler (HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
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
            // If a key is pressed send it to the input object so it can record that state.
            //m_Input->KeyDown((unsigned int)wparam);
            return 0;
        }

        // Check if a key has been released on the keyboard.
        case WM_KEYUP:
        {
            // If a key is released then send it to the input object so it can unset the state for that key.
            //m_Input->KeyUp((unsigned int)wparam);
            return 0;
        }

        // Any other messages send to the default message handler as our application won't make use of them.
        default:
        {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}

void InitWindow (size_t screenWidth, size_t screenHeight, bool fullScreen)
{
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;

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
        screenWidth = 800;
        screenHeight = 600;

        // Place the window in the middle of the screen.
        posX = (GetSystemMetrics(SM_CXSCREEN) - (int)screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - (int)screenHeight) / 2;
    }

    // Create the window with the screen settings and get the handle to it.
    g_hWnd = CreateWindowEx(WS_EX_APPWINDOW, APPLICATION_NAME, APPLICATION_NAME,
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
        posX, posY, (int)screenWidth, (int)screenHeight, NULL, NULL, hinstance, NULL);

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
    SetFocus(g_hWnd);

    // Hide the mouse cursor.
    //ShowCursor(false);

    return;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    MSG msg;
    bool done = false;


    // settings
    size_t width = 800;
    size_t height = 600;
    bool fullScreen = false;
    bool vsync = true;

    InitWindow(width, height, fullScreen);

    // TODO: if it fails, report why to a log or something. Or maybe show a message box
    if (!D3D11Init(width, height, vsync, g_hWnd, fullScreen, 100.0f, 0.01f))
        done = true;

    if (!ShaderInit(D3D11GetDevice(), g_hWnd, L"shader.fx"))
        done = true;

    if (!ModelInit(D3D11GetDevice()))
        done = true;

    while (!done)
    {
        // Handle the windows messages.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If windows signals to end the application then exit out.
        if (msg.message == WM_QUIT)
        {
            done = true;
        }
        else
        {
            ShaderSetConstants(D3D11GetContext(), 2.0f, 1.0f, 1.0f, 1.0f);
            D3D11BeginScene(0.4f, 0.0f, 0.4f, 1.0f);
            ModelRender(D3D11GetContext());
            ShaderDraw(D3D11GetContext(), 3);
            D3D11EndScene();

            // TODO: this! make it run a frame
            /*
            // Otherwise do the frame processing.
            result = Frame();
            if (!result)
            {
                done = true;
            }
            */
        }
    }

    ModelShutdown();
    ShaderShutdown();
    D3D11Shutdown();

    return 0;
}