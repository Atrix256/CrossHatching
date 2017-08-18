#pragma once

#include <array>

typedef std::array<float, 4> float4;

namespace ShaderTypes
{
    // define the structs
    #define CONSTANT_BUFFER_BEGIN(NAME) struct NAME {
    #define BUFFER_FIELD(NAME, TYPE) TYPE NAME;
    #define CONSTANT_BUFFER_END };
    #include "ShaderTypesList.h"

};