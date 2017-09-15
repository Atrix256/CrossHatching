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
        bool debug);

    void Present ();

    void SetScissor (size_t x1, size_t y1, size_t x2, size_t y2);
    void ClearScissor ();

    void EnableAlphaBlend (bool enable);

    void DrawIndexed (size_t indexCount, size_t startIndex = 0, size_t startVertex = 0) const;

    ID3D11Device* Device() { return m_device.m_ptr; }
    ID3D11DeviceContext* Context () { return m_deviceContext.m_ptr; }

    ID3D11SamplerState* SamplerLinearWrap () { return m_samplerLinearWrap.m_ptr; }
    ID3D11SamplerState* SamplerNearestWrap () { return m_samplerNearestWrap.m_ptr; }
    ID3D11SamplerState* SamplerAnisoWrap () { return m_samplerAnisoWrap.m_ptr; }

    ~CD3D11 ();

private:
    bool m_vsync_enabled;
    CAutoReleasePointer<IDXGISwapChain> m_swapChain;
    CAutoReleasePointer<ID3D11Device> m_device;
    CAutoReleasePointer<ID3D11DeviceContext> m_deviceContext;
    CAutoReleasePointer<ID3D11RenderTargetView> m_renderTargetView;
    CAutoReleasePointer<ID3D11RasterizerState> m_rasterState;

    CAutoReleasePointer<ID3D11SamplerState> m_samplerLinearWrap;
    CAutoReleasePointer<ID3D11SamplerState> m_samplerNearestWrap;
    CAutoReleasePointer<ID3D11SamplerState> m_samplerAnisoWrap;

    size_t m_screenWidth;
    size_t m_screenHeight;
};

extern CD3D11 g_d3d;