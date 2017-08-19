#pragma once

#include <d3d11.h>
#include "Utils.h"

class CD3D11
{
public:
    bool Init (
        size_t screenWidth,
        size_t screenHeight,
        bool vsync,
        HWND hWnd,
        bool fullscreen,
        float screenDepth,
        float screenNear,
        bool debug);

    void BeginScene (float red, float green, float blue, float alpha);
    void EndScene ();

    ID3D11Device* Device() { return m_device.m_ptr; }
    ID3D11DeviceContext* Context () { return m_deviceContext.m_ptr; }

    ~CD3D11 ();

private:
    bool m_vsync_enabled;
    CAutoReleasePointer<IDXGISwapChain> m_swapChain;
    CAutoReleasePointer<ID3D11Device> m_device;
    CAutoReleasePointer<ID3D11DeviceContext> m_deviceContext;
    CAutoReleasePointer<ID3D11RenderTargetView> m_renderTargetView;
    CAutoReleasePointer<ID3D11Texture2D> m_depthStencilBuffer;
    CAutoReleasePointer<ID3D11DepthStencilState> m_depthStencilState;
    CAutoReleasePointer<ID3D11DepthStencilView> m_depthStencilView;
    CAutoReleasePointer<ID3D11RasterizerState> m_rasterState;
};