#pragma once

#include <windows.h>
#include <functional>

enum class EKeyEvent
{
    press,
    release,
    input
};

typedef std::function<void(char key, EKeyEvent event)> TKeyPressCallback;

void WindowInit (size_t screenWidth, size_t screenHeight, bool fullScreen, TKeyPressCallback keyPressCallback);

HWND WindowGetHWND ();