#pragma once

#include <windows.h>

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen);

HWND WindowGetHWND ();