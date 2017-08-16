#pragma once

#include <d3d11.h>
#include "Utils.h"

class CTexture
{
public:
    bool LoadTGA (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);
    
    ID3D11ShaderResourceView* GetTexture () { return m_textureView.m_ptr; }

private:
    CAutoReleasePointer<ID3D11Texture2D>            m_texture;
    CAutoReleasePointer<ID3D11ShaderResourceView>   m_textureView;
};