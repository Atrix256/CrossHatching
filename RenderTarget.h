#pragma once

#include <d3d11.h>
#include "Utils.h"

class CRenderTarget
{
public:
    bool Create (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height);

    void SetAsRenderTarget (ID3D11DeviceContext* deviceContext);

private:
    CAutoReleasePointer<ID3D11Texture2D>            m_texture;
    CAutoReleasePointer<ID3D11ShaderResourceView>   m_textureView;
    CAutoReleasePointer<ID3D11RenderTargetView>     m_renderTargetView;
};
