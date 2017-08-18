#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Utils.h"

// TODO: how to handle constant buffers? Macro lists that generate something that is a template argument?
// TODO: how to handle textures?
// TODO: same with structured buffers and unordered access stuff
// both for vs/ps as well as cs shaders.

// TODO: use the shadertypes.h somehow?
struct SConstantBuffer
{
    float color[4];
};

class CShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug);

    bool SetConstants (ID3D11DeviceContext* deviceContext, const SConstantBuffer& constantBuffer, ID3D11ShaderResourceView* texture);

    void Draw (ID3D11DeviceContext* deviceContext, size_t indexCount);

private:
    CAutoReleasePointer<ID3D11VertexShader> m_vertexShader;
    CAutoReleasePointer<ID3D11PixelShader> m_pixelShader;
    CAutoReleasePointer<ID3D11InputLayout> m_layout;
    CAutoReleasePointer<ID3D11Buffer> m_constantBuffer;
    CAutoReleasePointer<ID3D11SamplerState> m_sampleState;

    CAutoReleasePointer<ID3D10Blob> m_vertexShaderBuffer;
    CAutoReleasePointer<ID3D10Blob> m_pixelShaderBuffer;

    CAutoReleasePointer<ID3D11ShaderReflection> m_psReflector;
    CAutoReleasePointer<ID3D11ShaderReflection> m_vsReflector;
};

class CComputeShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug);

    bool Dispatch (ID3D11DeviceContext* deviceContext, const SConstantBuffer& constantBuffer, size_t x, size_t y, size_t z);

private:
    CAutoReleasePointer<ID3D11ComputeShader> m_computeShader;

    CAutoReleasePointer<ID3D11Buffer> m_constantBuffer;

    // TODO: doesn't belong here
    CAutoReleasePointer<ID3D11Buffer> m_structuredBuffer;
    CAutoReleasePointer<ID3D11ShaderResourceView> m_structuredBufferSRV;

    CAutoReleasePointer<ID3D10Blob> m_computeShaderBuffer;

    CAutoReleasePointer<ID3D11ShaderReflection> m_reflector;
};