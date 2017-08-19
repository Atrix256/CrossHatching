#include "Shader.h"

#include <d3d11.h>
#include <directxmath.h>
#include <fstream>

static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    std::ofstream fout;

    // Get a pointer to the error message text buffer.
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // Get the length of the message.
    bufferSize = errorMessage->GetBufferSize();

    // Open a file to write the error message to.
    fout.open("shader-error.txt");

    // Write out the error message.
    for (i = 0; i<bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // Close the file.
    fout.close();

    // Release the error message.
    errorMessage->Release();
    errorMessage = 0;

    // Pop a message up on the screen to notify the user to check the text file for compile errors.
    MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

    return;
}

bool CShader::Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, D3D11_INPUT_ELEMENT_DESC* vertexFormat, size_t vertexFormatElements, bool debug)
{
    HRESULT result;
    ID3D10Blob* errorMessage;

    D3D11_SAMPLER_DESC samplerDesc;

    // Initialize the pointers this function will use to null.
    errorMessage = 0;
    UINT compileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    if (debug)
        compileFlags |= D3D10_SHADER_DEBUG;

    // Compile the vertex shader code.
    result = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_5_0", compileFlags, 0,
        &m_vertexShaderBuffer.m_ptr, &errorMessage);
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hWnd, fileName);
        }
        // If there was  nothing in the error message then it simply could not find the shader file itself.
        else
        {
            MessageBox(hWnd, fileName, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // Compile the pixel shader code.
    result = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main", "ps_5_0", compileFlags, 0,
        &m_pixelShaderBuffer.m_ptr, &errorMessage);
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hWnd, fileName);
        }
        // If there was nothing in the error message then it simply could not find the file itself.
        else
        {
            MessageBox(hWnd, fileName, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(m_vertexShaderBuffer.m_ptr->GetBufferPointer(), m_vertexShaderBuffer.m_ptr->GetBufferSize(), NULL, &m_vertexShader.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(m_pixelShaderBuffer.m_ptr->GetBufferPointer(), m_pixelShaderBuffer.m_ptr->GetBufferSize(), NULL, &m_pixelShader.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Create the vertex input layout.  This matches the shader and the model format.
    result = device->CreateInputLayout(vertexFormat, (UINT)vertexFormatElements, m_vertexShaderBuffer.m_ptr->GetBufferPointer(),
        m_vertexShaderBuffer.m_ptr->GetBufferSize(), &m_layout.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // TODO: temp!
    result = D3DReflect(m_pixelShaderBuffer.m_ptr->GetBufferPointer(), m_pixelShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_psReflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    result = D3DReflect(m_vertexShaderBuffer.m_ptr->GetBufferPointer(), m_vertexShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_vsReflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void CShader::Draw (ID3D11DeviceContext* deviceContext, size_t indexCount)
{
    // Set the vertex input layout.
    deviceContext->IASetInputLayout(m_layout.m_ptr);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader.m_ptr, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader.m_ptr, NULL, 0);

    // Set the sampler state in the pixel shader.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState.m_ptr);

    // Render the triangle.
    deviceContext->DrawIndexed((UINT)indexCount, 0, 0);
}

bool CComputeShader::Load(ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug)
{
    HRESULT result;
    ID3D10Blob* errorMessage;

    // Initialize the pointers this function will use to null.
    errorMessage = 0;

    UINT compileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    if (debug)
        compileFlags |= D3D10_SHADER_DEBUG;

    // Compile the compute shader code.
    result = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "cs_main", "cs_5_0", compileFlags, 0,
        &m_computeShaderBuffer.m_ptr, &errorMessage);
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hWnd, fileName);
        }
        // If there was  nothing in the error message then it simply could not find the shader file itself.
        else
        {
            MessageBox(hWnd, fileName, L"Missing Shader File", MB_OK);
        }

        return false;
    }
    
    // Create the compute shader from the buffer.
    result = device->CreateComputeShader(m_computeShaderBuffer.m_ptr->GetBufferPointer(), m_computeShaderBuffer.m_ptr->GetBufferSize(), NULL, &m_computeShader.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    result = D3DReflect(m_computeShaderBuffer.m_ptr->GetBufferPointer(), m_computeShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_reflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void CComputeShader::Dispatch (ID3D11DeviceContext* deviceContext, size_t x, size_t y, size_t z)
{
    deviceContext->CSSetShader(m_computeShader.m_ptr, NULL, 0);

    deviceContext->Dispatch((UINT)x, (UINT)y, (UINT)z);
}