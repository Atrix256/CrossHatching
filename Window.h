#pragma once

#include <windows.h>
#include <functional>

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen);

HWND WindowGetHWND ();