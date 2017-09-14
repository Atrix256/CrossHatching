#pragma once

#include <d3d11.h>
#include "Utils.h"
#include <vector>

template <typename VertexType>
class CModel
{
public:
    template <typename LAMBDA>
    bool Create (ID3D11Device* device, LAMBDA& lambda)
    {
        D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
        D3D11_SUBRESOURCE_DATA vertexData, indexData;
        HRESULT result;

        // have the caller fill out the vertex and index data. It can take them as references.
        lambda(m_vertices, m_indices);

        // Set up the description of the static vertex buffer.
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexType) * m_vertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        // Give the subresource structure a pointer to the vertex data.
        vertexData.pSysMem = &m_vertices[0];
        vertexData.SysMemPitch = 0;
        vertexData.SysMemSlicePitch = 0;

        // Now create the vertex buffer.
        result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer.m_ptr);
        if (FAILED(result))
        {
            return false;
        }

        // Set up the description of the static index buffer.
        indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        indexBufferDesc.ByteWidth = (UINT)(sizeof(unsigned long) * m_indices.size());
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        indexBufferDesc.MiscFlags = 0;
        indexBufferDesc.StructureByteStride = 0;

        // Give the subresource structure a pointer to the index data.
        indexData.pSysMem = &m_indices[0];
        indexData.SysMemPitch = 0;
        indexData.SysMemSlicePitch = 0;

        // Create the index buffer.
        result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer.m_ptr);
        if (FAILED(result))
        {
            return false;
        }
        return true;
    }

    template <typename LAMBDA>
    bool Write (ID3D11DeviceContext* deviceContext, LAMBDA& lambda)
    {
        // let the caller write to the storage. They can accept the argument as a reference
        lambda(m_vertices, m_indices);

        // write the vertex data
        HRESULT result;
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        {
            result = deviceContext->Map(m_vertexBuffer.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (FAILED(result))
            {
                return false;
            }
            memcpy(mappedResource.pData, &m_vertices[0], sizeof(m_vertices[0]) * m_vertices.size());
            deviceContext->Unmap(m_vertexBuffer.m_ptr, 0);
        }

        // write the index data
        {
            result = deviceContext->Map(m_indexBuffer.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (FAILED(result))
            {
                return false;
            }
            memcpy(mappedResource.pData, &m_indices[0], sizeof(m_indices[0]) * m_indices.size());
            deviceContext->Unmap(m_indexBuffer.m_ptr, 0);
        }

        return true;
    }

    void Render (ID3D11DeviceContext* deviceContext)
    {
        unsigned int stride;
        unsigned int offset;

        // Set vertex buffer stride and offset.
        stride = sizeof(VertexType);
        offset = 0;

        // Set the vertex buffer to active in the input assembler so it can be rendered.
        deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer.m_ptr, &stride, &offset);

        // Set the index buffer to active in the input assembler so it can be rendered.
        deviceContext->IASetIndexBuffer(m_indexBuffer.m_ptr, DXGI_FORMAT_R32_UINT, 0);

        // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    size_t GetIndexCount() const { return m_indices.size(); }

private:
    std::vector<VertexType> m_vertices;
    std::vector<unsigned long> m_indices;
    CAutoReleasePointer<ID3D11Buffer> m_vertexBuffer;
    CAutoReleasePointer<ID3D11Buffer> m_indexBuffer;
};
