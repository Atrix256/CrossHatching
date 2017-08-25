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
        #define TEXTURE(NAME, FILENAME) extern CTexture NAME;
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

inline void CalculateTriangleNormal (ShaderTypes::StructuredBuffers::TrianglePrim& triangle)
{
    float3 AB = XYZ(triangle.positionB_Emissive) - XYZ(triangle.positionA_Albedo);
    float3 AC = XYZ(triangle.positionC_w) - XYZ(triangle.positionA_Albedo);

    float3 norm = Cross(AB, AC);
    Normalize(norm);
    triangle.normal_w[0] = norm[0];
    triangle.normal_w[1] = norm[1];
    triangle.normal_w[2] = norm[2];
}

inline void CalculateQuadNormal (ShaderTypes::StructuredBuffers::QuadPrim& quad)
{
    float3 AB = XYZ(quad.positionB_Emissive) - XYZ(quad.positionA_Albedo);
    float3 AC = XYZ(quad.positionC_w) - XYZ(quad.positionA_Albedo);

    float3 norm = Cross(AB, AC);
    Normalize(norm);
    quad.normal_w[0] = norm[0];
    quad.normal_w[1] = norm[1];
    quad.normal_w[2] = norm[2];
}

inline void CalculateOBBNormals (ShaderTypes::StructuredBuffers::OBBPrim& obb)
{
    // make sure the axis we get is normalized
    float3 axis = XYZ(obb.rotationAxis_rotationAngle);
    Normalize(axis);
    float rotation = obb.rotationAxis_rotationAngle[3];

    // calculate the x,y,z axis of the OBB
    float cosTheta = cos(rotation);
    float sinTheta = sin(rotation);

    obb.XAxis_w =
    {
        cosTheta + axis[0] * axis[0] * (1.0f - cosTheta),
        axis[0] * axis[1] * (1.0f - cosTheta) - axis[2] * sinTheta,
        axis[0] * axis[2] * (1.0f - cosTheta) + axis[1] * sinTheta,
        0.0f
    };

    obb.YAxis_w =
    {
        axis[1] * axis[0] * (1.0f - cosTheta) + axis[2] * sinTheta,
        cosTheta + axis[1] * axis[1] * (1.0f - cosTheta),
        axis[1] * axis[2] * (1.0f - cosTheta) - axis[0] * sinTheta,
        0.0f
    };

    obb.ZAxis_w =
    {
        axis[2] * axis[0] * (1.0f - cosTheta) - axis[1] * sinTheta,
        axis[2] * axis[1] * (1.0f - cosTheta) + axis[0] * sinTheta,
        cosTheta + axis[2] * axis[2] * (1.0f - cosTheta),
        0.0f
    };
}