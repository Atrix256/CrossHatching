#pragma once

bool D3D11Init (
    size_t screenWidth,
    size_t screenHeight,
    bool vsync,
    void* hwnd,
    bool fullscreen,
    float screenDepth,
    float screenNear
);

void D3D11Shutdown();

void D3D11BeginScene(float red, float green, float blue, float alpha);
void D3D11EndScene();
