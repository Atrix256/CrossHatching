#pragma once

#include <array>

template <typename T, size_t NUMELEMENTS>
class CStructuredBuffer
{
public:

    bool Create (ID3D11Device* device, bool CPUWrites, const char* debugName)
    {
        // CPU write, GPU read
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.ByteWidth = NUMELEMENTS * sizeof(T);
        if (CPUWrites)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        }
        else
        {
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        }
        
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
        m_structuredBuffer.SetDebugName(debugName);

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
        m_structuredBufferSRV.SetDebugName(debugName);

        if (CPUWrites)
            return true;

        // setup the unordered access view description
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = NUMELEMENTS;
        uavDesc.Buffer.Flags = 0;

        // create the unordered access view for the texture.
        result = device->CreateUnorderedAccessView(m_structuredBuffer.m_ptr, &uavDesc, &m_structuredBufferUAV.m_ptr);
        if (FAILED(result))
        {
            return false;
        }
        m_structuredBufferUAV.SetDebugName(debugName);

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

    const std::array<T, NUMELEMENTS>& Read () const { return m_storage; }

    ID3D11ShaderResourceView* GetSRV () { return m_structuredBufferSRV.m_ptr; }
    ID3D11UnorderedAccessView* GetUAV() { return m_structuredBufferUAV.m_ptr; }

private:

    std::array<T, NUMELEMENTS> m_storage;

    CAutoReleasePointer<ID3D11Buffer> m_structuredBuffer;
    CAutoReleasePointer<ID3D11ShaderResourceView> m_structuredBufferSRV;
    CAutoReleasePointer<ID3D11UnorderedAccessView> m_structuredBufferUAV;
};