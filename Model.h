#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Utils.h"

// TODO: how to handle vertex format?
// TODO: how do we handle model loading?

class CModel
{
public:
    bool Load (ID3D11Device* device);

    void Render (ID3D11DeviceContext* deviceContext);

    size_t GetIndexCount() const { return m_indexCount; }

private:
    CAutoReleasePointer<ID3D11Buffer> m_vertexBuffer;
    CAutoReleasePointer<ID3D11Buffer> m_indexBuffer;
    size_t m_vertexCount = 0;
    size_t m_indexCount = 0;
};
