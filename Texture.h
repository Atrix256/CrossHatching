#pragma once

#include <d3d11.h>
#include "Utils.h"

class CTexture
{
public:
    bool LoadTGA (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);

    bool Create (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height, DXGI_FORMAT format);

    bool CreateVolume (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height, size_t depth, CTexture* slices, DXGI_FORMAT format);
    
    ID3D11ShaderResourceView* GetSRV () { return m_textureSRV.m_ptr; }

    ID3D11UnorderedAccessView* GetUAV () { return m_textureUAV.m_ptr; }

    ID3D11Texture2D* GetTexture2D () { return m_texture.m_ptr; }

    ID3D11Texture3D* GetTexture3D () { return m_texture3D.m_ptr; }

private:
    CAutoReleasePointer<ID3D11Texture2D>            m_texture;
    CAutoReleasePointer<ID3D11Texture3D>            m_texture3D;
    CAutoReleasePointer<ID3D11ShaderResourceView>   m_textureSRV;
    CAutoReleasePointer<ID3D11UnorderedAccessView>  m_textureUAV;
};