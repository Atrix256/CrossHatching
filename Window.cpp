#include "window.h"
#include "IMGUIWrap.h"

#define APPLICATION_NAME L"Generalized Cross Hatching From Path Tracing"

static HWND s_hWnd = nullptr;

bool IMGUI_EventHandler (HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        io.MouseDown[0] = true;
        return io.WantCaptureMouse;
    case WM_LBUTTONUP:
        io.MouseDown[0] = false;
        return io.WantCaptureMouse;
    case WM_RBUTTONDOWN:
        io.MouseDown[1] = true;
        return io.WantCaptureMouse;
    case WM_RBUTTONUP:
        io.MouseDown[1] = false;
        return io.WantCaptureMouse;
    case WM_MBUTTONDOWN:
        io.MouseDown[2] = true;
        return io.WantCaptureMouse;
    case WM_MBUTTONUP:
        io.MouseDown[2] = false;
        return io.WantCaptureMouse;
    case WM_MOUSEWHEEL:
        io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
        return io.WantCaptureMouse;
    case WM_MOUSEMOVE:
        POINT P;
        GetCursorPos(&P);
        ScreenToClient(WindowGetHWND(), &P);
        io.MousePos.x = (float)P.x;
        io.MousePos.y = (float)P.y;
        return io.WantCaptureMouse;
    case WM_KEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return io.WantCaptureKeyboard;
    case WM_KEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return io.WantCaptureKeyboard;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter((unsigned short)wParam);
        return io.WantTextInput;
    }
    return false;
}

LRESULT CALLBACK MessageHandler (HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    ImGuiIO& io = ImGui::GetIO();

    if (IMGUI_EventHandler(hwnd, umsg, wparam, lparam))
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

        // Check if a key has been released on the keyboard.
        case WM_KEYUP:
        {
            if ((char)wparam == 'H')
                EnableIMGUI(!GetIMGUIEnabled());
            return 0;
        }

        // Any other messages send to the default message handler as our application won't make use of them.
        default:
        {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen)
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

        // Create the window with the screen settings and get the handle to it.
        s_hWnd = CreateWindowEx(WS_EX_APPWINDOW, APPLICATION_NAME, APPLICATION_NAME,
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
            posX, posY, (int)screenWidth, (int)screenHeight, NULL, NULL, hinstance, NULL);
    }
    else
    {
        RECT windowSize;
        windowSize.left = 0;
        windowSize.top = 0;
        windowSize.right = (int)screenWidth;
        windowSize.bottom = (int)screenHeight;
        AdjustWindowRect(&windowSize, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUPWINDOW | WS_CAPTION, false);

        windowSize.right -= windowSize.left;
        windowSize.left = 0;
        windowSize.bottom -= windowSize.top;
        windowSize.top = 0;

        // Place the window in the middle of the screen.
        posX = (GetSystemMetrics(SM_CXSCREEN) - windowSize.right) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - windowSize.bottom) / 2;

        // Create the window with the screen settings and get the handle to it.
        s_hWnd = CreateWindowEx(WS_EX_APPWINDOW, APPLICATION_NAME, APPLICATION_NAME,
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUPWINDOW | WS_CAPTION,
            posX, posY, windowSize.right, windowSize.bottom, NULL, NULL, hinstance, NULL);
    }

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(s_hWnd, SW_SHOW);
    SetForegroundWindow(s_hWnd);
    SetFocus(s_hWnd);

    if (fullScreen)
        ShowCursor(false);
}

HWND WindowGetHWND()
{
    return s_hWnd;
}