#define _CRT_SECURE_NO_WARNINGS

#include "ShaderTypes.h"
#include "d3d11.h"
#include "Window.h"

// automatically reflected things for shaders
namespace ShaderData
{
    namespace ConstantBuffers
    {
        #define CONSTANT_BUFFER_BEGIN(NAME) CConstantBuffer<ShaderTypes::ConstantBuffers::##NAME> NAME;
        #include "ShaderTypesList.h"
    };

    namespace StructuredBuffers
    {
        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) CStructuredBuffer<ShaderTypes::StructuredBuffers::##TYPENAME, COUNT> NAME;
        #include "ShaderTypesList.h"
    };

    namespace Textures
    {
        #define TEXTURE_IMAGE(NAME, FILENAME) CTexture NAME;
        #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) CTexture NAME;
        #define TEXTURE_VOLUME_BEGIN(NAME) CTexture NAME;
        #define TEXTURE_ARRAY_BEGIN(NAME) CTexture NAME;
        #include "ShaderTypesList.h"
    }

    namespace Shaders
    {
        #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) std::array<CComputeShader, (uint64_t)1 << (uint64_t)EStaticBranches_CS_##NAME::COUNT> NAME;
        #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) std::array<CShader, (uint64_t)1 << (uint64_t)EStaticBranches_VSPS_##NAME::COUNT> NAME;
        #include "ShaderTypesList.h"
    };

    namespace VertexFormats
    {
        #define VERTEX_FORMAT_BEGIN(NAME) D3D11_INPUT_ELEMENT_DESC NAME [ShaderData::VertexFormats::VertexFormatFields_##NAME::COUNT] = {
        #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, CPPTYPE, SHADERTYPE, FORMAT) \
            {#SEMANTIC, INDEX, FORMAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        #define VERTEX_FORMAT_END };
        #include "ShaderTypesList.h"
    }

    // make shader permutation selection functions
    #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
        const CComputeShader& GetShader_##NAME (const std::array<bool, (size_t)ShaderData::Shaders::EStaticBranches_CS_##NAME::COUNT>& params) \
        { \
            auto& permutations = ShaderData::Shaders::NAME; \
            size_t permutationIndex = 0; \
            size_t branchIndex = 0;
    #define SHADER_CS_STATICBRANCH(NAME) if (params[branchIndex]) { permutationIndex |= ((size_t)1 << branchIndex); } ++branchIndex;
    #define SHADER_CS_END return permutations[permutationIndex]; }
    #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
        const CShader& GetShader_##NAME (const std::array<bool, (size_t)ShaderData::Shaders::EStaticBranches_VSPS_##NAME::COUNT>& params) \
        { \
            auto& permutations = ShaderData::Shaders::NAME; \
            size_t permutationIndex = 0; \
            size_t branchIndex = 0;
    #define SHADER_VSPS_STATICBRANCH(NAME) if (params[branchIndex]) { permutationIndex |= ((size_t)1 << branchIndex); } ++branchIndex;
    #define SHADER_VSPS_END return permutations[permutationIndex]; }
    #include "ShaderTypesList.h"

    // make shader static branch creation functions
    #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
        bool WriteStaticBranches_CS_##NAME (uint64_t permutation) \
        { \
            FILE *file = fopen("Shaders/StaticBranches.h", "w+t"); \
            if (!file) \
                return false; \
            fprintf(file, "//This file is autogenerated for each shader, using ShaderTypesList.h as source data\n\n"); \
            uint64_t mask = 1;            
    #define SHADER_CS_STATICBRANCH(NAME) \
            fprintf(file, "#define " #NAME " %i\n", (permutation & mask) ? 1 : 0); \
            mask *= 2;
    #define SHADER_CS_END \
            fclose(file); \
            return true; \
        }
    #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
        bool WriteStaticBranches_VSPS_##NAME (uint64_t permutation) \
        { \
            FILE *file = fopen("Shaders/StaticBranches.h", "w+t"); \
            if (!file) \
                return false; \
            fprintf(file, "//This file is autogenerated for each shader, using ShaderTypesList.h as source data\n\n"); \
            uint64_t mask = 1;            
    #define SHADER_VSPS_STATICBRANCH(NAME) \
            fprintf(file, "#define " #NAME " %i\n", (permutation & mask) ? 1 : 0); \
            mask *= 2;
    #define SHADER_VSPS_END \
            fclose(file); \
            return true; \
        }
    #include "ShaderTypesList.h"
};

// generate shader code ShaderTypes.h
bool WriteShaderTypesHLSL (void)
{
    FILE *file = fopen("Shaders/ShaderTypes.h", "w+t");
    if (!file)
        return false;

    fprintf(file, "//This file is autogenerated by WriteShaderTypesHLSL(), using ShaderTypesList.h as source data\n\n");

    // static branches
    fprintf(file, "#include \"StaticBranches.h\"\n\n");

    // hard coded sampler states
    fprintf(file, "//----------------------------------------------------------------------------\n//Samplers\n//----------------------------------------------------------------------------\n");
    fprintf(file, "SamplerState SamplerLinearWrap;\nSamplerState SamplerNearestWrap;\nSamplerState SamplerAnisoWrap;\n\n");

    // write the texture declarations
    fprintf(file, "//----------------------------------------------------------------------------\n//Textures\n//----------------------------------------------------------------------------\n");
    #define TEXTURE_IMAGE(NAME, FILENAME) fprintf(file, "Texture2D " #NAME ";\nRWTexture2D<float4> " #NAME "_rw;\n\n");
    #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) fprintf(file, "Texture2D " #NAME ";\nRWTexture2D<" #SHADERTYPE "> " #NAME "_rw;\n\n");
    #define TEXTURE_VOLUME_BEGIN(NAME) fprintf(file, "Texture3D " #NAME ";\n\n");
    #define TEXTURE_ARRAY_BEGIN(NAME) fprintf(file, "Texture2DArray " #NAME ";\n\n");
    #include "ShaderTypesList.h"

    // write the cbuffer declarations
    fprintf(file, "//----------------------------------------------------------------------------\n//Constant Buffers\n//----------------------------------------------------------------------------\n");
    #define CONSTANT_BUFFER_BEGIN(NAME) fprintf(file, "cbuffer " #NAME "\n{\n");
    #define CONSTANT_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define CONSTANT_BUFFER_END fprintf(file, "};\n\n");
    #include "ShaderTypesList.h"

    // write the vertex formats
    fprintf(file, "//----------------------------------------------------------------------------\n//Vertex Formats\n//----------------------------------------------------------------------------\n");
    #define VERTEX_FORMAT_BEGIN(NAME) fprintf(file, "struct " #NAME "\n{\n");
    #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, CPPTYPE, SHADERTYPE, FORMAT) fprintf(file, "  " #SHADERTYPE " " #NAME " : " #SEMANTIC #INDEX ";\n");
    #define VERTEX_FORMAT_END fprintf(file, "};\n\n");
    #include "ShaderTypesList.h"

    // write the struct declarations for structured buffers
    fprintf(file, "//----------------------------------------------------------------------------\n//Structured Buffer Types\n//----------------------------------------------------------------------------\n");
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) fprintf(file, "struct " #TYPENAME "\n{\n");
    #define STRUCTURED_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define STRUCTURED_BUFFER_END fprintf(file, "};\n\n");
    #include "ShaderTypesList.h"

    // write the structured buffer declarations
    fprintf(file, "//----------------------------------------------------------------------------\n//Structured Buffers\n//----------------------------------------------------------------------------\n");
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) fprintf(file, "StructuredBuffer<" #TYPENAME "> " #NAME ";\nRWStructuredBuffer<" #TYPENAME "> " #NAME "_rw;\n\n");
    #include "ShaderTypesList.h"

    fclose(file);
    return true;
}

bool ShaderTypesInit (void)
{
    // create constant buffers
    #define CONSTANT_BUFFER_BEGIN(NAME) if (!ShaderData::ConstantBuffers::NAME.Create(g_d3d.Device(), #NAME)) { ReportError("Could not create constant buffer: " #NAME "\n"); return false; }
    #include "ShaderTypesList.h"

    // create structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES) if (!ShaderData::StructuredBuffers::NAME.Create(g_d3d.Device(), CPUWRITES, #NAME)) { ReportError("Could not create structured buffer: " #NAME "\n"); return false; }
    #include "ShaderTypesList.h"

    // create textures
    #define TEXTURE_IMAGE(NAME, FILENAME) \
        if(!ShaderData::Textures::NAME.LoadTGA(g_d3d.Device(), g_d3d.Context(), FILENAME)) { ReportError("Could not load texture: " #NAME "\n"); return false; }
    #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) \
        if(!ShaderData::Textures::NAME.Create(g_d3d.Device(), g_d3d.Context(), c_width, c_height, FORMAT, #NAME)) { ReportError("Could not create texture buffer: " #NAME "\n"); return false; }
    #include "ShaderTypesList.h"

    // create volume textures and texture arrays
    #define TEXTURE_VOLUME_BEGIN(NAME) CTexture* slices##NAME [] = {
    #define TEXTURE_VOLUME_SLICE(TEXTURE) &ShaderData::Textures::##TEXTURE,
    #define TEXTURE_VOLUME_END };
    #define TEXTURE_ARRAY_BEGIN(NAME) CTexture* slices##NAME [] = {
    #define TEXTURE_ARRAY_SLICE(TEXTURE) &ShaderData::Textures::##TEXTURE,
    #define TEXTURE_ARRAY_END };
    #include "ShaderTypesList.h"

    #define TEXTURE_VOLUME_BEGIN(NAME) \
        size_t numSlices##NAME = sizeof(slices##NAME) / sizeof(slices##NAME[0]);\
        if (!ShaderData::Textures::NAME.CreateVolume(g_d3d.Device(), g_d3d.Context(), slices##NAME, numSlices##NAME)) { ReportError("Could not create volume texture: " #NAME "\n"); return false; }
    #define TEXTURE_ARRAY_BEGIN(NAME) \
        size_t numSlices##NAME = sizeof(slices##NAME) / sizeof(slices##NAME[0]);\
        if (!ShaderData::Textures::NAME.CreateArray(g_d3d.Device(), g_d3d.Context(), slices##NAME, numSlices##NAME, #NAME)) { ReportError("Could not create texture array: " #NAME "\n"); return false; }
    #include "ShaderTypesList.h"

    // write hlsl shadertypes.h
    if (!WriteShaderTypesHLSL())
        return false;

    // create shaders
    #define SHADER_CS_BEGIN(NAME, FILENAME, ENTRY) \
    { \
        static_assert((uint32_t)ShaderData::Shaders::EStaticBranches_CS_##NAME::COUNT <= 32, "Too many static branches in CS shader: "#NAME); \
        uint64_t maxValue = (uint64_t)1 << ((uint64_t)ShaderData::Shaders::EStaticBranches_CS_##NAME::COUNT); \
        for (uint64_t i = 0; i < maxValue; ++i) \
        { \
            ShaderData::WriteStaticBranches_CS_##NAME(i); \
            if (!ShaderData::Shaders::NAME[i].Load(g_d3d.Device(), WindowGetHWND(), FILENAME, ENTRY, c_shaderDebug, #NAME)) { ReportError("Could not create compute shader: " #NAME "\n"); return false; } \
        } \
    }
    #define SHADER_VSPS_BEGIN(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
    { \
        static_assert((uint32_t)ShaderData::Shaders::EStaticBranches_VSPS_##NAME::COUNT <= 32, "Too many static branches in VSPS shader: "#NAME); \
        uint64_t maxValue = (uint64_t)1 << ((uint64_t)ShaderData::Shaders::EStaticBranches_VSPS_##NAME::COUNT); \
        for (uint64_t i = 0; i < maxValue; ++i) \
        { \
            ShaderData::WriteStaticBranches_VSPS_##NAME(i); \
            if (!ShaderData::Shaders::NAME[i].Load(g_d3d.Device(), WindowGetHWND(), FILENAME, VSENTRY, PSENTRY, ShaderData::VertexFormats::VERTEXFORMAT, (size_t)ShaderData::VertexFormats::VertexFormatFields_##VERTEXFORMAT::COUNT, c_shaderDebug, #NAME)) { ReportError("Could not create shader : " #NAME "\n"); return false; } \
        } \
    }
    #include "ShaderTypesList.h"

    return true;
}