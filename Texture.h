#pragma once

#include <d3d11.h>
#include "Utils.h"

class CTexture
{
public:
    bool LoadTGA (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);

    bool Create (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height);
    
    ID3D11ShaderResourceView* GetSRV () { return m_textureView.m_ptr; }

    ID3D11UnorderedAccessView* GetUAV () { return m_textureViewCompute.m_ptr; }

private:
    CAutoReleasePointer<ID3D11Texture2D>            m_texture;
    CAutoReleasePointer<ID3D11ShaderResourceView>   m_textureView;
    CAutoReleasePointer<ID3D11UnorderedAccessView>  m_textureViewCompute;
};