#pragma once

#include <d3d11.h>
#include "Utils.h"

// TODO: how to handle constant buffers? Macro lists that generate something that is a template argument?
// TODO: how to handle textures?

struct SConstantBuffer
{
    float color[4];
};

class CShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug);

    bool SetConstants (ID3D11DeviceContext* deviceContext, const SConstantBuffer& constantBuffer, ID3D11ShaderResourceView* texture);

    void Draw (ID3D11DeviceContext* deviceContext, int indexCount);

private:
    CAutoReleasePointer<ID3D11VertexShader> m_vertexShader;
    CAutoReleasePointer<ID3D11PixelShader> m_pixelShader;
    CAutoReleasePointer<ID3D11InputLayout> m_layout;
    CAutoReleasePointer<ID3D11Buffer> m_constantBuffer;
    CAutoReleasePointer<ID3D11SamplerState> m_sampleState;
};