#pragma once

template <size_t NUMELEMENTS>
class CVertexFormat
{
public:
    bool Create (const char* Semantics[NUMELEMENTS], UINT indices[NUMELEMENTS])
    {
        return true;
    }

private:
    D3D11_INPUT_ELEMENT_DESC polygonLayout[NUMELEMENTS];
    CAutoReleasePointer<ID3D11InputLayout> m_layout;
};