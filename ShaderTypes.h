#pragma once

#include <array>
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Texture.h"
#include "Shader.h"

typedef std::array<float, 2> float2;
typedef std::array<float, 3> float3;
typedef std::array<float, 4> float4;

typedef std::array<unsigned int, 4> uint4;

bool ShaderTypesInit (void);

namespace ShaderTypes
{
#pragma pack(1)
    // define the constant buffer structs
    namespace ConstantBuffers 
    {
        #define CONSTANT_BUFFER_BEGIN(NAME) struct NAME {
        #define CONSTANT_BUFFER_FIELD(NAME, TYPE) TYPE NAME;
        #define CONSTANT_BUFFER_END };
        #include "ShaderTypesList.h"
    };

    // define the structured buffer structs
    namespace StructuredBuffers
    {
        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) struct TYPENAME {
        #define STRUCTURED_BUFFER_FIELD(NAME, TYPE) TYPE NAME;
        #define STRUCTURED_BUFFER_END };
        #include "ShaderTypesList.h"

        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) typedef std::array<ShaderTypes::StructuredBuffers::##TYPENAME, COUNT> T##NAME;
        #include "ShaderTypesList.h"
    };

    // define the vertex format structs
    namespace VertexFormats
    {
        #define VERTEX_FORMAT_BEGIN(NAME) struct NAME {
        #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, CPPTYPE, SHADERTYPE, FORMAT) CPPTYPE NAME;
        #define VERTEX_FORMAT_END };
        #include "ShaderTypesList.h"
    };
#pragma pack()
};

namespace ShaderData
{
    namespace ConstantBuffers
    {
        #define CONSTANT_BUFFER_BEGIN(NAME) extern CConstantBuffer<ShaderTypes::ConstantBuffers::##NAME> NAME;
        #include "ShaderTypesList.h"
    };

    namespace StructuredBuffers
    {
        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) extern CStructuredBuffer<ShaderTypes::StructuredBuffers::##TYPENAME, COUNT> NAME;
        #include "ShaderTypesList.h"
    };

    namespace Textures
    {
        #define TEXTURE_IMAGE(NAME, FILENAME) extern CTexture NAME;
        #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) extern CTexture NAME;
        #define TEXTURE_VOLUME_BEGIN(NAME) extern CTexture NAME;
        #define TEXTURE_ARRAY_BEGIN(NAME) extern CTexture NAME;
        #include "ShaderTypesList.h"
    };

    namespace Shaders
    {
        #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
            enum class EStaticBranches_CS_##NAME { 
        #define SHADER_CS_STATICBRANCH(NAME) NAME,
        #define SHADER_CS_END \
                COUNT \
            };
        #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
            enum class EStaticBranches_VSPS_##NAME { 
        #define SHADER_VSPS_STATICBRANCH(NAME) NAME,
        #define SHADER_VSPS_END \
                COUNT \
            };
        #include "ShaderTypesList.h"

        #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
            extern std::array<CComputeShader, (uint64_t)1 << (uint64_t)EStaticBranches_CS_##NAME::COUNT> NAME;

        #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
            extern std::array<CShader, (uint64_t)1 << (uint64_t)EStaticBranches_VSPS_##NAME::COUNT> NAME;
        #include "ShaderTypesList.h"
    };

    namespace VertexFormats
    {
        #define VERTEX_FORMAT_BEGIN(NAME)  \
            enum class VertexFormatFields_##NAME { 
        #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, CPPTYPE, SHADERTYPE, FORMAT) NAME,
        #define VERTEX_FORMAT_END \
                COUNT \
            };
        #include "ShaderTypesList.h"

        #define VERTEX_FORMAT_BEGIN(NAME)  \
            extern D3D11_INPUT_ELEMENT_DESC NAME[ShaderData::VertexFormats::VertexFormatFields_##NAME::COUNT];
        #include "ShaderTypesList.h"
    }

    // define shader permutation selection functions
    #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
        const CComputeShader& GetShader_##NAME (const std::array<bool, (size_t)ShaderData::Shaders::EStaticBranches_CS_##NAME::COUNT>& params);
    #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
        const CShader& GetShader_##NAME (const std::array<bool, (size_t)ShaderData::Shaders::EStaticBranches_VSPS_##NAME::COUNT>& params);
    #include "ShaderTypesList.h"

    // define shader static branch creation functions
    #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
        bool WriteStaticBranches_CS_##NAME (uint64_t permutation);
    #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
        bool WriteStaticBranches_VSPS_##NAME (uint64_t permutation);
    #include "ShaderTypesList.h"
};

inline float Dot (const float3& a, const float3& b)
{
    return
        a[0] * b[0] +
        a[1] * b[1] +
        a[2] * b[2];
}

inline float3 Cross (const float3& a, const float3& b)
{
    return
    {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

inline float3 XYZ (const float4& v)
{
    return { v[0], v[1], v[2] };
}

template <size_t N>
std::array<float, N> operator- (const std::array<float, N>& A, const std::array<float, N>& B)
{
    std::array<float, N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = A[i] - B[i];
    return result;
}

template <size_t N>
inline float Length (std::array<float, N>& v)
{
    float len = 0.0f;
    for (float f : v)
        len += f*f;
    return std::sqrtf(len);
}

template <size_t N>
inline void Normalize (std::array<float, N>& v)
{
    float len = Length(v);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

inline float3 Normal (const std::array<float, 3>& a, const std::array<float, 3>& b, const std::array<float, 3>& c)
{
    float3 ab = b - a;
    float3 ac = c - a;

    float3 norm = Cross(ab, ac);
    Normalize(norm);
    return norm;
}

inline float3 ChangeBasis (const std::array<float, 3>& v, const std::array<float, 3>& xAxis, const std::array<float, 3>& yAxis, const std::array<float, 3>& zAxis)
{
    return
    {
        Dot(v, {xAxis[0], yAxis[0], zAxis[0]}),
        Dot(v, {xAxis[1], yAxis[1], zAxis[1]}),
        Dot(v, {xAxis[2], yAxis[2], zAxis[2]})
    };
}

inline void MakeTriangle (
    ShaderTypes::StructuredBuffers::TrianglePrim& triangle,
    float3 a,
    float3 b,
    float3 c,
    float3 albedo,
    float3 emissive
)
{
    triangle.positionA_w[0] = a[0];
    triangle.positionA_w[1] = a[1];
    triangle.positionA_w[2] = a[2];
    triangle.positionA_w[3] = 0.0f;

    triangle.positionB_w[0] = b[0];
    triangle.positionB_w[1] = b[1];
    triangle.positionB_w[2] = b[2];
    triangle.positionB_w[3] = 0.0f;

    triangle.positionC_w[0] = c[0];
    triangle.positionC_w[1] = c[1];
    triangle.positionC_w[2] = c[2];
    triangle.positionC_w[3] = 0.0f;

    triangle.albedo_w[0] = albedo[0];
    triangle.albedo_w[1] = albedo[1];
    triangle.albedo_w[2] = albedo[2];
    triangle.albedo_w[3] = 0.0f;

    triangle.emissive_w[0] = emissive[0];
    triangle.emissive_w[1] = emissive[1];
    triangle.emissive_w[2] = emissive[2];
    triangle.emissive_w[3] = 0.0f;

    // calculate normal
    float3 norm = Normal(a, b, c);
    triangle.normal_w[0] = norm[0];
    triangle.normal_w[1] = norm[1];
    triangle.normal_w[2] = norm[2];
    triangle.normal_w[3] = 0.0f;
}

inline void MakeQuad(
    ShaderTypes::StructuredBuffers::QuadPrim& quad,
    float3 a,
    float3 b,
    float3 c,
    float3 d,
    float3 albedo,
    float3 emissive
)
{
    quad.positionA_w[0] = a[0];
    quad.positionA_w[1] = a[1];
    quad.positionA_w[2] = a[2];
    quad.positionA_w[3] = 0.0f;

    quad.positionB_w[0] = b[0];
    quad.positionB_w[1] = b[1];
    quad.positionB_w[2] = b[2];
    quad.positionB_w[3] = 0.0f;

    quad.positionC_w[0] = c[0];
    quad.positionC_w[1] = c[1];
    quad.positionC_w[2] = c[2];
    quad.positionC_w[3] = 0.0f;

    quad.positionD_w[0] = d[0];
    quad.positionD_w[1] = d[1];
    quad.positionD_w[2] = d[2];
    quad.positionD_w[3] = 0.0f;

    quad.albedo_w[0] = albedo[0];
    quad.albedo_w[1] = albedo[1];
    quad.albedo_w[2] = albedo[2];
    quad.albedo_w[3] = 0.0f;

    quad.emissive_w[0] = emissive[0];
    quad.emissive_w[1] = emissive[1];
    quad.emissive_w[2] = emissive[2];
    quad.emissive_w[3] = 0.0f;

    // calculate normal
    float3 norm = Normal(a, b, c);
    quad.normal_w[0] = norm[0];
    quad.normal_w[1] = norm[1];
    quad.normal_w[2] = norm[2];
    quad.normal_w[3] = 0.0f;
}

inline void MakeSphere(
    ShaderTypes::StructuredBuffers::SpherePrim& sphere,
    float3 position,
    float radius,
    float3 albedo,
    float3 emissive)
{
    sphere.position_Radius[0] = position[0];
    sphere.position_Radius[1] = position[1];
    sphere.position_Radius[2] = position[2];
    sphere.position_Radius[3] = radius;

    sphere.albedo_w[0] = albedo[0];
    sphere.albedo_w[1] = albedo[1];
    sphere.albedo_w[2] = albedo[2];
    sphere.albedo_w[3] = 0.0f;

    sphere.emissive_w[0] = emissive[0];
    sphere.emissive_w[1] = emissive[1];
    sphere.emissive_w[2] = emissive[2];
    sphere.emissive_w[3] = 0.0f;
}

inline void MakeOBB (
    ShaderTypes::StructuredBuffers::OBBPrim& obb,
    float3 position,
    float3 radius,
    float3 rotAxis,
    float rotAngle,
    float3 albedo,
    float3 emissive
)
{
    obb.position_w[0] = position[0];
    obb.position_w[1] = position[1];
    obb.position_w[2] = position[2];
    obb.position_w[3] = 0.0f;

    obb.radius_w[0] = radius[0];
    obb.radius_w[1] = radius[1];
    obb.radius_w[2] = radius[2];
    obb.radius_w[3] = 0.0f;

    obb.albedo_w[0] = albedo[0];
    obb.albedo_w[1] = albedo[1];
    obb.albedo_w[2] = albedo[2];
    obb.albedo_w[3] = 0.0f;

    obb.emissive_w[0] = emissive[0];
    obb.emissive_w[1] = emissive[1];
    obb.emissive_w[2] = emissive[2];
    obb.emissive_w[3] = 0.0f;

    // make sure the axis we get is normalized
    Normalize(rotAxis);

    // calculate the x,y,z axis of the OBB
    float cosTheta = cos(rotAngle);
    float sinTheta = sin(rotAngle);

    obb.XAxis_w =
    {
        cosTheta + rotAxis[0] * rotAxis[0] * (1.0f - cosTheta),
        rotAxis[0] * rotAxis[1] * (1.0f - cosTheta) - rotAxis[2] * sinTheta,
        rotAxis[0] * rotAxis[2] * (1.0f - cosTheta) + rotAxis[1] * sinTheta,
        0.0f
    };

    obb.YAxis_w =
    {
        rotAxis[1] * rotAxis[0] * (1.0f - cosTheta) + rotAxis[2] * sinTheta,
        cosTheta + rotAxis[1] * rotAxis[1] * (1.0f - cosTheta),
        rotAxis[1] * rotAxis[2] * (1.0f - cosTheta) - rotAxis[0] * sinTheta,
        0.0f
    };

    obb.ZAxis_w =
    {
        rotAxis[2] * rotAxis[0] * (1.0f - cosTheta) - rotAxis[1] * sinTheta,
        rotAxis[2] * rotAxis[1] * (1.0f - cosTheta) + rotAxis[0] * sinTheta,
        cosTheta + rotAxis[2] * rotAxis[2] * (1.0f - cosTheta),
        0.0f
    };
}

template <EShaderType SHADER_TYPE>
void UnbindShaderTextures (ID3D11DeviceContext* deviceContext, ID3D11ShaderReflection* reflector)
{
    D3D11_SHADER_INPUT_BIND_DESC desc;
    HRESULT result;

    // reflect textures
    #define TEXTURE_IMAGE(NAME, FILENAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = nullptr; \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } \
        result = reflector->GetResourceBindingDescByName(#NAME "_rw", &desc); \
        if (!FAILED(result)) { \
            UINT count = -1; \
            ID3D11UnorderedAccessView* uav = nullptr; \
            if (SHADER_TYPE == EShaderType::compute) \
                deviceContext->CSSetUnorderedAccessViews(desc.BindPoint, 1, &uav, &count); \
        }

    #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = nullptr; \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } \
        result = reflector->GetResourceBindingDescByName(#NAME "_rw", &desc); \
        if (!FAILED(result)) { \
            UINT count = -1; \
            ID3D11UnorderedAccessView* uav = nullptr; \
            if (SHADER_TYPE == EShaderType::compute) \
                deviceContext->CSSetUnorderedAccessViews(desc.BindPoint, 1, &uav, &count); \
        }

    #define TEXTURE_VOLUME_BEGIN(NAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = nullptr; \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } 

    #define TEXTURE_ARRAY_BEGIN(NAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = nullptr; \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } 

    // reflect structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = nullptr; \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } \
        result = reflector->GetResourceBindingDescByName(#NAME "_rw", &desc); \
        if (!FAILED(result)) { \
            UINT count = -1; \
            ID3D11UnorderedAccessView* uav = nullptr; \
            if (SHADER_TYPE == EShaderType::compute) \
                deviceContext->CSSetUnorderedAccessViews(desc.BindPoint, 1, &uav, &count); \
        }

    #include "ShaderTypesList.h"
}

template <EShaderType SHADER_TYPE>
void FillShaderParams (ID3D11DeviceContext* deviceContext, ID3D11ShaderReflection* reflector)
{
    D3D11_SHADER_INPUT_BIND_DESC desc;
    HRESULT result;

    // reflect constant buffers
    #define CONSTANT_BUFFER_BEGIN(NAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11Buffer* buffer = ShaderData::ConstantBuffers::##NAME.Get(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetConstantBuffers(desc.BindPoint, 1, &buffer); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetConstantBuffers(desc.BindPoint, 1, &buffer); \
            else \
                deviceContext->CSSetConstantBuffers(desc.BindPoint, 1, &buffer); \
        }
    
    // reflect structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = ShaderData::StructuredBuffers::##NAME.GetSRV(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } \
        result = reflector->GetResourceBindingDescByName(#NAME "_rw", &desc); \
        if (!FAILED(result)) { \
            UINT count = -1; \
            ID3D11UnorderedAccessView* uav = ShaderData::StructuredBuffers::##NAME.GetUAV(); \
            if (SHADER_TYPE == EShaderType::compute) \
                deviceContext->CSSetUnorderedAccessViews(desc.BindPoint, 1, &uav, &count); \
        }

    // reflect textures
    #define TEXTURE_IMAGE(NAME, FILENAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = ShaderData::Textures::##NAME.GetSRV(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } \
        result = reflector->GetResourceBindingDescByName(#NAME "_rw", &desc); \
        if (!FAILED(result)) { \
            UINT count = -1; \
            ID3D11UnorderedAccessView* uav = ShaderData::Textures::##NAME.GetUAV(); \
            if (SHADER_TYPE == EShaderType::compute) \
                deviceContext->CSSetUnorderedAccessViews(desc.BindPoint, 1, &uav, &count); \
        }

    #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = ShaderData::Textures::##NAME.GetSRV(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        } \
        result = reflector->GetResourceBindingDescByName(#NAME "_rw", &desc); \
        if (!FAILED(result)) { \
            UINT count = -1; \
            ID3D11UnorderedAccessView* uav = ShaderData::Textures::##NAME.GetUAV(); \
            if (SHADER_TYPE == EShaderType::compute) \
                deviceContext->CSSetUnorderedAccessViews(desc.BindPoint, 1, &uav, &count); \
        }

    #define TEXTURE_VOLUME_BEGIN(NAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = ShaderData::Textures::##NAME.GetSRV(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        }

    #define TEXTURE_ARRAY_BEGIN(NAME) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = ShaderData::Textures::##NAME.GetSRV(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        }

    #include "ShaderTypesList.h"

    // hard coded samplers
    result = reflector->GetResourceBindingDescByName("SamplerLinearWrap", &desc);
    if (!FAILED(result))
    {
        ID3D11SamplerState* sampler = g_d3d.SamplerLinearWrap();
        if (SHADER_TYPE == EShaderType::vertex)
            deviceContext->VSSetSamplers(desc.BindPoint, 1, &sampler);
        else if (SHADER_TYPE == EShaderType::pixel)
            deviceContext->PSSetSamplers(desc.BindPoint, 1, &sampler);
        else
            deviceContext->CSSetSamplers(desc.BindPoint, 1, &sampler);
    }
    result = reflector->GetResourceBindingDescByName("SamplerNearestWrap", &desc);
    if (!FAILED(result))
    {
        ID3D11SamplerState* sampler = g_d3d.SamplerNearestWrap();
        if (SHADER_TYPE == EShaderType::vertex)
            deviceContext->VSSetSamplers(desc.BindPoint, 1, &sampler);
        else if (SHADER_TYPE == EShaderType::pixel)
            deviceContext->PSSetSamplers(desc.BindPoint, 1, &sampler);
        else
            deviceContext->CSSetSamplers(desc.BindPoint, 1, &sampler);
    }
    result = reflector->GetResourceBindingDescByName("SamplerAnisoWrap", &desc);
    if (!FAILED(result))
    {
        ID3D11SamplerState* sampler = g_d3d.SamplerAnisoWrap();
        if (SHADER_TYPE == EShaderType::vertex)
            deviceContext->VSSetSamplers(desc.BindPoint, 1, &sampler);
        else if (SHADER_TYPE == EShaderType::pixel)
            deviceContext->PSSetSamplers(desc.BindPoint, 1, &sampler);
        else
            deviceContext->CSSetSamplers(desc.BindPoint, 1, &sampler);
    }
}