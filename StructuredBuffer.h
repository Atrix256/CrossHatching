#pragma once

#include <array>

template <typename T, size_t NUMELEMENTS>
class CStructuredBuffer
{
public:

    bool Create (ID3D11Device* device)
    {
        // CPU write, GPU read
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.ByteWidth = NUMELEMENTS * sizeof(T);
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = sizeof(T);

        D3D11_SUBRESOURCE_DATA bufferInitData;
        ZeroMemory((&bufferInitData), sizeof(bufferInitData));
        bufferInitData.pSysMem = nullptr;
        HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &m_structuredBuffer.m_ptr);
        if (FAILED(result))
        {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.ElementWidth = NUMELEMENTS;
        result = device->CreateShaderResourceView(m_structuredBuffer.m_ptr, &srvDesc, &m_structuredBufferSRV.m_ptr);
        if (FAILED(result))
        {
            return false;
        }

        return true;
    }

    template <typename LAMBDA>
    bool Write (ID3D11DeviceContext* deviceContext, LAMBDA& lambda)
    {
        // let the caller write to the storage. They can accept it as a reference
        lambda(m_storage);

        // prepare to write to the structred buffer
        HRESULT result;
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        T* dataPtr;
        result = deviceContext->Map(m_structuredBuffer.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(result))
        {
            return false;
        }

        // write the data to the constant buffer
        dataPtr = (T*)mappedResource.pData;
        for (size_t i = 0; i < m_storage.size(); ++i)
            dataPtr[i] = m_storage[i];

        // We are done writing
        deviceContext->Unmap(m_structuredBuffer.m_ptr, 0);
        return true;
    }

    ID3D11ShaderResourceView* GetSRV () { return m_structuredBufferSRV.m_ptr; }

private:

    std::array<T, NUMELEMENTS> m_storage;

    CAutoReleasePointer<ID3D11Buffer> m_structuredBuffer;
    CAutoReleasePointer<ID3D11ShaderResourceView> m_structuredBufferSRV;
};