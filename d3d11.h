#pragma once

#include <d3d11.h>

bool D3D11Init (
    size_t screenWidth,
    size_t screenHeight,
    bool vsync,
    HWND hWnd,
    bool fullscreen,
    float screenDepth,
    float screenNear,
    bool debug
);

void D3D11Shutdown();

void D3D11BeginScene (float red, float green, float blue, float alpha);
void D3D11EndScene ();

ID3D11Device* D3D11GetDevice ();
ID3D11DeviceContext* D3D11GetContext ();