#pragma once

#include <windows.h>
#include <functional>

typedef std::function<void(unsigned char key, bool pressed)> TKeyPressCallback;

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen, TKeyPressCallback keyPressCallback);

HWND WindowGetHWND ();