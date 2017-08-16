#include "RenderTarget.h"

bool CRenderTarget::Create(ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height)
{
    // relevant article on rendering to textures: http://www.rastertek.com/dx11tut22.html
    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    HRESULT hResult;

    // Setup the description of the texture.
    textureDesc.Height = (UINT)height;
    textureDesc.Width = (UINT)width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    // set up the render target view description
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    // Create the render target view.
    hResult = device->CreateRenderTargetView(m_texture.m_ptr, &rtvDesc, &m_renderTargetView.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture.m_ptr, &srvDesc, &m_textureView.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    return true;
}

void CRenderTarget::SetAsRenderTarget(ID3D11DeviceContext* deviceContext)
{
    deviceContext->OMSetRenderTargets(1, &m_renderTargetView.m_ptr, nullptr);
}