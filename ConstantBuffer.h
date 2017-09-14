#pragma once

#include <d3d11.h>
#include "Utils.h"

template <typename T>
class CConstantBuffer
{
public:
    bool Create (ID3D11Device* device)
    {
        // set up the description
        D3D11_BUFFER_DESC constantBufferDesc;
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.ByteWidth = sizeof(T);
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantBufferDesc.MiscFlags = 0;
        constantBufferDesc.StructureByteStride = 0;

        // Create the buffer
        HRESULT result = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer.m_ptr);
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

        // prepare to write to the constant buffer
        HRESULT result;
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        T* dataPtr;
        result = deviceContext->Map(m_constantBuffer.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(result))
        {
            return false;
        }

        // write the data to the constant buffer
        dataPtr = (T*)mappedResource.pData;
        dataPtr[0] = m_storage;

        // We are done writing
        deviceContext->Unmap(m_constantBuffer.m_ptr, 0);
        return true;
    }

    const T& Read () const
    {
        return m_storage;
    }

    ID3D11Buffer* Get () { return m_constantBuffer.m_ptr; }

private:
    CAutoReleasePointer<ID3D11Buffer> m_constantBuffer;

    T m_storage;
};