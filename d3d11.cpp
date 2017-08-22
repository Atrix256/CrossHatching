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
    float screenDepth,
    float screenNear,
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
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;

    // Store the vsync setting (only available in full screen mode)
    vsync = vsync && fullscreen;
    m_vsync_enabled = vsync;

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

    // Initialize the description of the depth buffer.
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = (UINT)screenWidth;
    depthBufferDesc.Height = (UINT)screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    result = m_device.m_ptr->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the description of the stencil state.
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the depth stencil state.
    result = m_device.m_ptr->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Set the depth stencil state.
    m_deviceContext.m_ptr->OMSetDepthStencilState(m_depthStencilState.m_ptr, 1);

    // Initialize the depth stencil view.
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    result = m_device.m_ptr->CreateDepthStencilView(m_depthStencilBuffer.m_ptr, &depthStencilViewDesc, &m_depthStencilView.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Bind the render target view and depth stencil buffer to the output render pipeline.
    m_deviceContext.m_ptr->OMSetRenderTargets(1, &m_renderTargetView.m_ptr, m_depthStencilView.m_ptr);

    // Setup the raster description which will determine how and what polygons will be drawn.
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
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

void CD3D11::BeginScene (float red, float green, float blue, float alpha)
{
    float color[4];

    // Setup the color to clear the buffer to.
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    // Clear the back buffer.
    m_deviceContext.m_ptr->ClearRenderTargetView(m_renderTargetView.m_ptr, color);

    // Clear the depth buffer.
    m_deviceContext.m_ptr->ClearDepthStencilView(m_depthStencilView.m_ptr, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CD3D11::EndScene ()
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