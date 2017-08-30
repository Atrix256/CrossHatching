#pragma once

#include <array>
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Texture.h"

typedef std::array<float, 2> float2;
typedef std::array<float, 3> float3;
typedef std::array<float, 4> float4;

namespace ShaderTypes
{
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
        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) struct TYPENAME {
        #define STRUCTURED_BUFFER_FIELD(NAME, TYPE) TYPE NAME;
        #define STRUCTURED_BUFFER_END };
        #include "ShaderTypesList.h"
    };

    // define the vertex format structs
    namespace VertexFormats
    {
        #define VERTEX_FORMAT_BEGIN(NAME) struct NAME {
        #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, TYPE, FORMAT) TYPE NAME;
        #define VERTEX_FORMAT_END };
        #include "ShaderTypesList.h"
    };
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
        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) extern CStructuredBuffer<ShaderTypes::StructuredBuffers::##TYPENAME, COUNT> NAME;
        #include "ShaderTypesList.h"
    };

    namespace Textures
    {
        #define TEXTURE_IMAGE(NAME, FILENAME) extern CTexture NAME;
        #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) extern CTexture NAME;
        #include "ShaderTypesList.h"
    }
};

inline float Dot(const float3& a, const float3& b)
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

inline void MakeTriangle (
    ShaderTypes::StructuredBuffers::TrianglePrim& triangle,
    float3 a,
    float3 b,
    float3 c,
    float albedo,
    float emissive
)
{
    triangle.positionA_Albedo[0] = a[0];
    triangle.positionA_Albedo[1] = a[1];
    triangle.positionA_Albedo[2] = a[2];
    triangle.positionA_Albedo[3] = albedo;

    triangle.positionB_Emissive[0] = b[0];
    triangle.positionB_Emissive[1] = b[1];
    triangle.positionB_Emissive[2] = b[2];
    triangle.positionB_Emissive[3] = emissive;

    triangle.positionC_w[0] = c[0];
    triangle.positionC_w[1] = c[1];
    triangle.positionC_w[2] = c[2];
    triangle.positionC_w[3] = 0.0f;

    // calculate normal
    float3 AB = b - a;
    float3 AC = c - a;

    float3 norm = Cross(AB, AC);
    Normalize(norm);
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
    float albedo,
    float emissive
)
{
    quad.positionA_Albedo[0] = a[0];
    quad.positionA_Albedo[1] = a[1];
    quad.positionA_Albedo[2] = a[2];
    quad.positionA_Albedo[3] = albedo;

    quad.positionB_Emissive[0] = b[0];
    quad.positionB_Emissive[1] = b[1];
    quad.positionB_Emissive[2] = b[2];
    quad.positionB_Emissive[3] = emissive;

    quad.positionC_w[0] = c[0];
    quad.positionC_w[1] = c[1];
    quad.positionC_w[2] = c[2];
    quad.positionC_w[3] = 0.0f;

    quad.positionD_w[0] = d[0];
    quad.positionD_w[1] = d[1];
    quad.positionD_w[2] = d[2];
    quad.positionD_w[3] = 0.0f;

    // calculate normal
    float3 AB = b - a;
    float3 AC = c - a;

    float3 norm = Cross(AB, AC);
    Normalize(norm);
    quad.normal_w[0] = norm[0];
    quad.normal_w[1] = norm[1];
    quad.normal_w[2] = norm[2];
    quad.normal_w[3] = 0.0f;
}

inline void MakeSphere(
    ShaderTypes::StructuredBuffers::SpherePrim& sphere,
    float3 position,
    float radius,
    float albedo,
    float emissive)
{
    sphere.position_Radius[0] = position[0];
    sphere.position_Radius[1] = position[1];
    sphere.position_Radius[2] = position[2];
    sphere.position_Radius[3] = radius;

    sphere.albedo_Emissive_zw[0] = albedo;
    sphere.albedo_Emissive_zw[1] = emissive;
    sphere.albedo_Emissive_zw[2] = 0.0f;
    sphere.albedo_Emissive_zw[3] = 0.0f;
}

inline void MakeOBB (
    ShaderTypes::StructuredBuffers::OBBPrim& obb,
    float3 position,
    float3 radius,
    float3 rotAxis,
    float rotAngle,
    float albedo,
    float emissive
)
{
    obb.position_Albedo[0] = position[0];
    obb.position_Albedo[1] = position[1];
    obb.position_Albedo[2] = position[2];
    obb.position_Albedo[3] = albedo;

    obb.radius_Emissive[0] = radius[0];
    obb.radius_Emissive[1] = radius[1];
    obb.radius_Emissive[2] = radius[2];
    obb.radius_Emissive[3] = emissive;

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
