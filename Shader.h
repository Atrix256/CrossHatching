#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Utils.h"


// TODO: how to handle textures?
// TODO: how to handle structured buffers and unordered access stuff?
// TODO: how to handle vertex format? or don't deal with it as you only have full screen quads?

enum class EShaderType
{
    vertex,
    pixel,
    compute
};

class CShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug);

    void SetConstants (ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture);

    void Draw (ID3D11DeviceContext* deviceContext, size_t indexCount);

    ID3D11ShaderReflection* GetVSReflector() { return m_vsReflector.m_ptr; }
    ID3D11ShaderReflection* GetPSReflector() { return m_psReflector.m_ptr; }

private:
    CAutoReleasePointer<ID3D11VertexShader> m_vertexShader;
    CAutoReleasePointer<ID3D11PixelShader> m_pixelShader;
    CAutoReleasePointer<ID3D11InputLayout> m_layout;
    CAutoReleasePointer<ID3D11SamplerState> m_sampleState;

    CAutoReleasePointer<ID3D10Blob> m_vertexShaderBuffer;
    CAutoReleasePointer<ID3D10Blob> m_pixelShaderBuffer;

    CAutoReleasePointer<ID3D11ShaderReflection> m_vsReflector;
    CAutoReleasePointer<ID3D11ShaderReflection> m_psReflector;
};

class CComputeShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug);

    void Dispatch (ID3D11DeviceContext* deviceContext, size_t x, size_t y, size_t z);

    ID3D11ShaderReflection* GetReflector () { return m_reflector.m_ptr; }

private:
    CAutoReleasePointer<ID3D11ComputeShader> m_computeShader;

    // TODO: doesn't belong here
    CAutoReleasePointer<ID3D11Buffer> m_structuredBuffer;
    CAutoReleasePointer<ID3D11ShaderResourceView> m_structuredBufferSRV;

    CAutoReleasePointer<ID3D10Blob> m_computeShaderBuffer;

    CAutoReleasePointer<ID3D11ShaderReflection> m_reflector;
};