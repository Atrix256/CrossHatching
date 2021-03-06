#include "Shader.h"

#include <d3d11.h>
#include <fstream>

static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    std::wofstream fout;

    // Get a pointer to the error message text buffer.
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // Get the length of the message.
    bufferSize = errorMessage->GetBufferSize();

    // Open a file to write the error message to.
    fout.open("shader-error.txt");

    fout << "Compiling " << shaderFilename << ":\n\n";

    // Write out the error message.
    for (i = 0; i<bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // Close the file.
    fout.close();

    return;
}

bool CShader::Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, const char* vsentry, const char* psentry, D3D11_INPUT_ELEMENT_DESC* vertexFormat, size_t vertexFormatElements, bool debug, const char* debugName)
{
    HRESULT result;
    CAutoReleasePointer<ID3D10Blob> errorMessage;

    // Initialize the pointers this function will use to null.
    UINT compileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    if (debug)
        compileFlags |= D3D10_SHADER_DEBUG;

    // Compile the vertex shader code.
    result = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, vsentry, "vs_5_0", compileFlags, 0,
        &m_vertexShaderBuffer.m_ptr, &errorMessage.m_ptr);

    // write shader errors / warnings if there were any (it won't fail if there are just warnings!)
    if (errorMessage.m_ptr)
    {
        OutputShaderErrorMessage(errorMessage.m_ptr, hWnd, fileName);
    }

    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage.m_ptr)
        {
            // Pop a message up on the screen to notify the user to check the text file for compile errors.
            MessageBox(hWnd, L"Error compiling shader.  Check shader-error.txt for message.", fileName, MB_OK);
        }
        else
        {
            MessageBox(hWnd, fileName, L"Missing Shader File", MB_OK);
        }

        return false;
    }
    //m_vertexShaderBuffer.SetDebugName(debugName);

    // Compile the pixel shader code.
    errorMessage.Clear();
    result = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, psentry, "ps_5_0", compileFlags, 0,
        &m_pixelShaderBuffer.m_ptr, &errorMessage.m_ptr);

    // write shader errors / warnings if there were any (it won't fail if there are just warnings!)
    if (errorMessage.m_ptr)
    {
        OutputShaderErrorMessage(errorMessage.m_ptr, hWnd, fileName);
    }

    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage.m_ptr)
        {
            // Pop a message up on the screen to notify the user to check the text file for compile errors.
            MessageBox(hWnd, L"Error compiling shader.  Check shader-error.txt for message.", fileName, MB_OK);
        }
        // If there was nothing in the error message then it simply could not find the file itself.
        else
        {
            MessageBox(hWnd, fileName, L"Missing Shader File", MB_OK);
        }

        return false;
    }
    //m_pixelShaderBuffer.SetDebugName(debugName);

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(m_vertexShaderBuffer.m_ptr->GetBufferPointer(), m_vertexShaderBuffer.m_ptr->GetBufferSize(), NULL, &m_vertexShader.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    m_vertexShader.SetDebugName(debugName);

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(m_pixelShaderBuffer.m_ptr->GetBufferPointer(), m_pixelShaderBuffer.m_ptr->GetBufferSize(), NULL, &m_pixelShader.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    m_pixelShader.SetDebugName(debugName);

    // Create the vertex input layout.  This matches the shader and the model format.
    result = device->CreateInputLayout(vertexFormat, (UINT)vertexFormatElements, m_vertexShaderBuffer.m_ptr->GetBufferPointer(),
        m_vertexShaderBuffer.m_ptr->GetBufferSize(), &m_layout.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    m_layout.SetDebugName(debugName);

    result = D3DReflect(m_pixelShaderBuffer.m_ptr->GetBufferPointer(), m_pixelShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_psReflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    //m_psReflector.SetDebugName(debugName);

    result = D3DReflect(m_vertexShaderBuffer.m_ptr->GetBufferPointer(), m_vertexShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_vsReflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    //m_vsReflector.SetDebugName(debugName);

    return true;
}

void CShader::Set (ID3D11DeviceContext* deviceContext) const
{
    // Set the vertex input layout.
    deviceContext->IASetInputLayout(m_layout.m_ptr);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader.m_ptr, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader.m_ptr, NULL, 0);
}

bool CComputeShader::Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, const char* entry, bool debug, const char* debugName)
{
    HRESULT result;
    CAutoReleasePointer<ID3D10Blob> errorMessage;

    UINT compileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    if (debug)
        compileFlags |= D3D10_SHADER_DEBUG;

    // Compile the compute shader code.
    result = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, "cs_5_0", compileFlags, 0,
        &m_computeShaderBuffer.m_ptr, &errorMessage.m_ptr);

    // write shader errors / warnings if there were any (it won't fail if there are just warnings!)
    if (errorMessage.m_ptr)
    {
        OutputShaderErrorMessage(errorMessage.m_ptr, hWnd, fileName);
    }

    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage.m_ptr)
        {
            // Pop a message up on the screen to notify the user to check the text file for compile errors.
            MessageBox(hWnd, L"Error compiling shader.  Check shader-error.txt for message.", fileName, MB_OK);
        }
        // If there was  nothing in the error message then it simply could not find the shader file itself.
        else
        {
            MessageBox(hWnd, fileName, L"Missing Shader File", MB_OK);
        }

        return false;
    }
    //m_computeShaderBuffer.SetDebugName(debugName);
    
    // Create the compute shader from the buffer.
    result = device->CreateComputeShader(m_computeShaderBuffer.m_ptr->GetBufferPointer(), m_computeShaderBuffer.m_ptr->GetBufferSize(), NULL, &m_computeShader.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    m_computeShader.SetDebugName(debugName);

    result = D3DReflect(m_computeShaderBuffer.m_ptr->GetBufferPointer(), m_computeShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_reflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }
    //m_reflector.SetDebugName(debugName);

    return true;
}

void CComputeShader::Dispatch (ID3D11DeviceContext* deviceContext, size_t x, size_t y, size_t z) const
{
    deviceContext->CSSetShader(m_computeShader.m_ptr, NULL, 0);

    deviceContext->Dispatch((UINT)x, (UINT)y, (UINT)z);
}