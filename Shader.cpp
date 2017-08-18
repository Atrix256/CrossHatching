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

bool CShader::Load (ID3D11Device* device, HWND hWnd, wchar_t* fileName, bool debug)
{
    HRESULT result;
    ID3D10Blob* errorMessage;

    D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
    unsigned int numElements;
    D3D11_BUFFER_DESC constantBufferDesc;
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

    // Create the vertex input layout description.
    // This setup needs to match the VertexType stucture in the ModelClass and in the shader.
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "COLOR";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "TEXCOORD";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    // Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, m_vertexShaderBuffer.m_ptr->GetBufferPointer(),
        m_vertexShaderBuffer.m_ptr->GetBufferSize(), &m_layout.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(SConstantBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
    constantBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer.m_ptr);
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

    //https://msdn.microsoft.com/en-us/library/windows/desktop/ff476625(v=vs.85).aspx
    // TODO: GetResourceBindingDescByName().  To see what constant buffers etc it needs!
    D3D11_SHADER_INPUT_BIND_DESC desc;
    result = m_vsReflector.m_ptr->GetResourceBindingDescByName("Constants", &desc);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool CShader::SetConstants (ID3D11DeviceContext* deviceContext, const SConstantBuffer& constantBuffer, ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    SConstantBuffer* dataPtr;
    unsigned int bufferNumber;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_constantBuffer.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (SConstantBuffer*)mappedResource.pData;
    dataPtr[0] = constantBuffer;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_constantBuffer.m_ptr, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Finanly set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer.m_ptr);
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer.m_ptr);

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);

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

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    D3D11_BUFFER_DESC constantBufferDesc;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(SConstantBuffer);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
    constantBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // TODO: this doesn't belong in here, should be it's own class probably.

    struct SBufferItem
    {
        float c[4];
    };
    SBufferItem initialData[1];
    initialData[0].c[0] = 0.4f;
    initialData[0].c[1] = 0.6f;
    initialData[0].c[2] = 0.8f;
    initialData[0].c[3] = 1.0f;
    SBufferItem* pInitialData = &initialData[0];
    UINT iNumElements = 1;

    // CPU write, GPU read
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = iNumElements * sizeof(SBufferItem);
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(SBufferItem);

    D3D11_SUBRESOURCE_DATA bufferInitData;
    ZeroMemory((&bufferInitData), sizeof(bufferInitData));
    bufferInitData.pSysMem = pInitialData;
    result = device->CreateBuffer((&bufferDesc), (pInitialData) ? (&bufferInitData) : NULL, &m_structuredBuffer.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = iNumElements;
    result = device->CreateShaderResourceView(m_structuredBuffer.m_ptr, &srvDesc, &m_structuredBufferSRV.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    // TODO: temp!
    result = D3DReflect(m_computeShaderBuffer.m_ptr->GetBufferPointer(), m_computeShaderBuffer.m_ptr->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_reflector.m_ptr);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool CComputeShader::Dispatch (ID3D11DeviceContext* deviceContext, const SConstantBuffer& constantBuffer, size_t x, size_t y, size_t z)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    SConstantBuffer* dataPtr;
    unsigned int bufferNumber;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_constantBuffer.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (SConstantBuffer*)mappedResource.pData;
    dataPtr[0] = constantBuffer;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_constantBuffer.m_ptr, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Finanly set the constant buffer in the vertex shader with the updated values.
    deviceContext->CSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer.m_ptr);


    deviceContext->CSSetShader(m_computeShader.m_ptr, NULL, 0);

    deviceContext->CSSetShaderResources(0, 1, &m_structuredBufferSRV.m_ptr);

    deviceContext->Dispatch((UINT)x, (UINT)y, (UINT)z);

    return true;
}