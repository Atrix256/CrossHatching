#include "d3d11.h"
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

bool CD3D11::Init (
    size_t screenWidth,
    size_t screenHeight,
    bool vsync,
    HWND hWnd,
    bool fullscreen,
    bool debug)
{
    HRESULT result;
    CAutoReleasePointer<IDXGIFactory> factory;
    CAutoReleasePointer<IDXGIAdapter> adapter;
    CAutoReleasePointer<IDXGIOutput> adapterOutput;
    unsigned int numModes, i, numerator, denominator;
    std::vector<DXGI_MODE_DESC> displayModeList;
    DXGI_ADAPTER_DESC adapterDesc;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    D3D_FEATURE_LEVEL featureLevel;
    CAutoReleasePointer<ID3D11Texture2D> backBufferPtr;
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;

    // Store some settings
    m_vsync_enabled = vsync;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Create a DirectX graphics interface factory.
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Use the factory to create an adapter for the primary graphics interface (video card).
    result = factory.m_ptr->EnumAdapters(0, &adapter.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Enumerate the primary adapter output (monitor).
    result = adapter.m_ptr->EnumOutputs(0, &adapterOutput.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
    result = adapterOutput.m_ptr->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Create a list to hold all the possible display modes for this monitor/video card combination.
    displayModeList.resize(numModes);

    // Now fill the display mode list structures.
    result = adapterOutput.m_ptr->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, &displayModeList[0]);
    if (FAILED(result))
    {
        return false;
    }

    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    for (i = 0; i < numModes; i++)
    {
        if (displayModeList[i].Width == (unsigned int)screenWidth)
        {
            if (displayModeList[i].Height == (unsigned int)screenHeight)
            {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    // Get the adapter (video card) description.
    result = adapter.m_ptr->GetDesc(&adapterDesc);
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the swap chain description.
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // Set to a single back buffer.
    swapChainDesc.BufferCount = 1;

    // Set the width and height of the back buffer.
    swapChainDesc.BufferDesc.Width = (UINT)screenWidth;
    swapChainDesc.BufferDesc.Height = (UINT)screenHeight;

    // Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the refresh rate of the back buffer.
    if (m_vsync_enabled)
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = hWnd;

    // Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // Set to full screen or windowed mode.
    if (fullscreen)
    {
        swapChainDesc.Windowed = false;
    }
    else
    {
        swapChainDesc.Windowed = true;
    }

    // Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Don't set the advanced flags.
    swapChainDesc.Flags = 0;

    // Set the feature level to DirectX 11.
    featureLevel = D3D_FEATURE_LEVEL_11_1;

    // Create the swap chain, Direct3D device, and Direct3D device context.
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, debug ? D3D11_CREATE_DEVICE_DEBUG : 0, &featureLevel, 1,
        D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain.m_ptr, &m_device.m_ptr, NULL, &m_deviceContext.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Get the pointer to the back buffer.
    result = m_swapChain.m_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Create the render target view with the back buffer pointer.
    result = m_device.m_ptr->CreateRenderTargetView(backBufferPtr.m_ptr, NULL, &m_renderTargetView.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Bind the render target view
    m_deviceContext.m_ptr->OMSetRenderTargets(1, &m_renderTargetView.m_ptr, NULL);

    // Setup the raster description which will determine how and what polygons will be drawn.
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = false;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = true;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    result = m_device.m_ptr->CreateRasterizerState(&rasterDesc, &m_rasterState.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Now set the rasterizer state.
    m_deviceContext.m_ptr->RSSetState(m_rasterState.m_ptr);

    // Setup the viewport for rendering.
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // Create the viewport.
    m_deviceContext.m_ptr->RSSetViewports(1, &viewport);

    // Create a texture sampler state description.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = m_device.m_ptr->CreateSamplerState(&samplerDesc, &m_samplerLinearWrap.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = m_device.m_ptr->CreateSamplerState(&samplerDesc, &m_samplerNearestWrap.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = m_device.m_ptr->CreateSamplerState(&samplerDesc, &m_samplerAnisoWrap.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

CD3D11::~CD3D11()
{
    // Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
    if (m_swapChain.m_ptr)
    {
        m_swapChain.m_ptr->SetFullscreenState(false, NULL);
    }
}

void CD3D11::Present ()
{
    // Present the back buffer to the screen since rendering is complete.
    if (m_vsync_enabled)
    {
        // Lock to screen refresh rate.
        m_swapChain.m_ptr->Present(1, 0);
    }
    else
    {
        // Present as fast as possible.
        m_swapChain.m_ptr->Present(0, 0);
    }
}

void CD3D11::SetScissor (size_t x1, size_t y1, size_t x2, size_t y2)
{
    D3D11_RECT scissor;
    scissor.left = (LONG)x1;
    scissor.top = (LONG)y1;
    scissor.right = (LONG)x2;
    scissor.bottom = (LONG)y2;
    Context()->RSSetScissorRects(1, &scissor);
}

void CD3D11::ClearScissor ()
{
    SetScissor(0, 0, m_screenWidth, m_screenHeight);
}

void CD3D11::EnableAlphaBlend (bool enable)
{
    ID3D11BlendState* d3dBlendState;
    D3D11_BLEND_DESC omDesc;
    ZeroMemory(&omDesc, sizeof(D3D11_BLEND_DESC));
    omDesc.RenderTarget[0].BlendEnable = enable;
    omDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    omDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    omDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    omDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    omDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    omDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    omDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if (FAILED(Device()->CreateBlendState(&omDesc, &d3dBlendState)))
        return;

    Context()->OMSetBlendState(d3dBlendState, 0, 0xffffffff);
}