#pragma once

#include <d3d11.h>
#include "Utils.h"

class CTexture
{
public:
    bool LoadTGA (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);

    bool LoadFromPixels(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned char* pixels, int width, int height, const char* debugName);

    bool Create (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height, DXGI_FORMAT format, const char* debugName);

    bool CreateVolume (ID3D11Device* device, ID3D11DeviceContext* deviceContext, CTexture** slices, size_t numSlices, const char* debugName);

    bool CreateArray (ID3D11Device* device, ID3D11DeviceContext* deviceContext, CTexture** slices, size_t numSlices, const char* debugName);
    
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