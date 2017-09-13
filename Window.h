#pragma once

#include <windows.h>
#include <functional>

typedef std::function<bool(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)> TWndProcCallback;

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen, TWndProcCallback wndProcCallback);

HWND WindowGetHWND ();