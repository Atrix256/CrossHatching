#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Utils.h"

enum class EShaderType
{
    vertex,
    pixel,
    compute
};

class CShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, const char* vsentry, const char* psentry, D3D11_INPUT_ELEMENT_DESC* vertexFormat, size_t vertexFormatElements, bool debug);

    void Set (ID3D11DeviceContext* deviceContext) const;

    ID3D11ShaderReflection* GetVSReflector () const { return m_vsReflector.m_ptr; }
    ID3D11ShaderReflection* GetPSReflector () const { return m_psReflector.m_ptr; }

private:
    CAutoReleasePointer<ID3D11VertexShader> m_vertexShader;
    CAutoReleasePointer<ID3D11PixelShader> m_pixelShader;
    CAutoReleasePointer<ID3D11InputLayout> m_layout;

    CAutoReleasePointer<ID3D10Blob> m_vertexShaderBuffer;
    CAutoReleasePointer<ID3D10Blob> m_pixelShaderBuffer;

    CAutoReleasePointer<ID3D11ShaderReflection> m_vsReflector;
    CAutoReleasePointer<ID3D11ShaderReflection> m_psReflector;
};

class CComputeShader
{
public:
    bool Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, const char* entry, bool debug);

    void Dispatch (ID3D11DeviceContext* deviceContext, size_t x, size_t y, size_t z) const;

    ID3D11ShaderReflection* GetReflector () const { return m_reflector.m_ptr; }

private:
    CAutoReleasePointer<ID3D11ComputeShader> m_computeShader;

    CAutoReleasePointer<ID3D10Blob> m_computeShaderBuffer;

    CAutoReleasePointer<ID3D11ShaderReflection> m_reflector;
};