#pragma once

#include <array>

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