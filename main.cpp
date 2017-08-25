#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <random>
#include "d3d11.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Window.h"
#include "ShaderTypes.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Scenes.h"

// settings
const size_t c_width = 800;
const size_t c_height = 600;
const bool c_fullScreen = false;
const bool c_vsync = true;
const bool c_shaderDebug = true;
const bool c_d3ddebug = true;
const float c_fovX = 0.698132f; // 40 degrees
const float c_fovY = c_fovX * float(c_height) / float(c_width);
const float c_nearPlane = 0.1f;
const float3 c_cameraPos = { 0.0f, 0.0f, -10.0f };
const float3 c_cameraAt = { 0.0f, 0.0f, 0.0f };

// globals
CD3D11 g_d3d;

CRenderTarget g_testBuffer;

CModel<ShaderTypes::VertexFormats::Pos2D> g_fullScreenMesh;

CComputeShader g_pathTrace;
CShader g_shaderShowPathTrace;

float RandomFloat (float min, float max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

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
        #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) CStructuredBuffer<ShaderTypes::StructuredBuffers::##TYPENAME, COUNT> NAME;
        #include "ShaderTypesList.h"
    };

    namespace Textures
    {
        #define TEXTURE(NAME, FILENAME) CTexture NAME;
        #include "ShaderTypesList.h"
    }

    namespace VertexFormats
    {
        #define VERTEX_FORMAT_BEGIN(NAME) D3D11_INPUT_ELEMENT_DESC NAME [] = {
        #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, TYPE, FORMAT) \
            {#SEMANTIC, INDEX, FORMAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        #define VERTEX_FORMAT_END };
        #include "ShaderTypesList.h"

        #define VERTEX_FORMAT_BEGIN(NAME) size_t NAME##Elements = sizeof(NAME) / sizeof(NAME[0]);
        #include "ShaderTypesList.h"
    }
};

// generate shader code ShaderTypes.h
bool WriteShaderTypesHLSL (void)
{
    FILE *file = fopen("Shaders/ShaderTypes.h", "w+t");
    if (!file)
        return false;

    fprintf(file, "//This file is autogenerated by WriteShaderTypesHLSL(), using ShaderTypesList.h as source data\n\n");

    // hard coded sampler states
    fprintf(file, "//----------------------------------------------------------------------------\n//Samplers\n//----------------------------------------------------------------------------\n");
    fprintf(file, "SamplerState SamplerLinearWrap;\n\n");

    // write the texture declarations
    fprintf(file, "//----------------------------------------------------------------------------\n//Textures\n//----------------------------------------------------------------------------\n");
    #define TEXTURE(NAME, FILENAME) fprintf(file, "Texture2D " #NAME ";\nRWTexture2D<float4> " #NAME "_rw;\n\n");
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
    #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, TYPE, FORMAT) fprintf(file, "  " #TYPE " " #NAME " : " #SEMANTIC #INDEX ";\n");
    #define VERTEX_FORMAT_END fprintf(file, "};\n\n");
    #include "ShaderTypesList.h"

    // write the struct declarations for structured buffers
    fprintf(file, "//----------------------------------------------------------------------------\n//Structured Buffer Types\n//----------------------------------------------------------------------------\n");
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) fprintf(file, "struct " #TYPENAME "\n{\n");
    #define STRUCTURED_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define STRUCTURED_BUFFER_END fprintf(file, "};\n\n");
    #include "ShaderTypesList.h"

    // write the structured buffer declarations
    fprintf(file, "//----------------------------------------------------------------------------\n//Structured Buffers\n//----------------------------------------------------------------------------\n");
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) fprintf(file, "StructuredBuffer<" #TYPENAME "> " #NAME ";\n\n");
    #include "ShaderTypesList.h"

    fclose(file);
    return true;
}

template <EShaderType SHADER_TYPE>
void UnbindShaderTextures (ID3D11DeviceContext* deviceContext, ID3D11ShaderReflection* reflector)
{
    D3D11_SHADER_INPUT_BIND_DESC desc;
    HRESULT result;

    // reflect textures
    #define TEXTURE(NAME, FILENAME) \
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
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) \
        result = reflector->GetResourceBindingDescByName(#NAME, &desc); \
        if (!FAILED(result)) { \
            ID3D11ShaderResourceView* srv = ShaderData::StructuredBuffers::##NAME.GetSRV(); \
            if (SHADER_TYPE == EShaderType::vertex) \
                deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); \
            else if (SHADER_TYPE == EShaderType::pixel) \
                deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); \
            else \
                deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); \
        }

    // reflect textures
    #define TEXTURE(NAME, FILENAME) \
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
}

void OnKeyPress (unsigned char key, bool pressed)
{
    // only do actions on key release right now
    if (pressed)
        return;

    switch (key)
    {
        case '1': FillSceneData(EScene::SphereOnPlane_LowLight, g_d3d.Context()); break;
        case '2': FillSceneData(EScene::SphereOnPlane_RegularLight, g_d3d.Context()); break;
        case '3': FillSceneData(EScene::SpheresInBox_LowLight, g_d3d.Context()); break;
        case '4': FillSceneData(EScene::SpheresInBox_RegularLight, g_d3d.Context()); break;
        case '5': FillSceneData(EScene::FurnaceTest, g_d3d.Context()); break;
    }
}

bool init ()
{
    WindowInit(c_width, c_height, c_fullScreen, OnKeyPress);

    if (!g_d3d.Init(c_width, c_height, c_vsync, WindowGetHWND(), c_fullScreen, 100.0f, 0.01f, c_d3ddebug))
        return false;

    if (!WriteShaderTypesHLSL())
        return false;

    // create constant buffers
    #define CONSTANT_BUFFER_BEGIN(NAME) if (!ShaderData::ConstantBuffers::NAME.Create(g_d3d.Device())) return false;
    #include "ShaderTypesList.h"

    // create structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) if (!ShaderData::StructuredBuffers::NAME.Create(g_d3d.Device())) return false;
    #include "ShaderTypesList.h"

    // create textures
    #define TEXTURE(NAME, FILENAME) \
        if (FILENAME == nullptr) { \
            if(!ShaderData::Textures::NAME.Create(g_d3d.Device(), g_d3d.Context(), c_width, c_height)) return false; \
        } else { \
            if(!ShaderData::Textures::NAME.LoadTGA(g_d3d.Device(), g_d3d.Context(), FILENAME)) return false; \
        }
    #include "ShaderTypesList.h"

    // write some triangles
    bool writeOK = ShaderData::StructuredBuffers::Triangles.Write(
        g_d3d.Context(),
        [] (std::array<ShaderTypes::StructuredBuffers::TrianglePrim, 10>& data)
        {
            for (size_t i = 0; i < 10; ++i)
            {
                data[i].positionA_Albedo[0] = 0.0f;
                data[i].positionA_Albedo[1] = 0.0f;
                data[i].positionA_Albedo[2] = 0.0f;
                data[i].positionA_Albedo[3] = 0.0f;

                data[i].positionB_Emissive[0] = 0.0f;
                data[i].positionB_Emissive[1] = 0.0f;
                data[i].positionB_Emissive[2] = 0.0f;
                data[i].positionB_Emissive[3] = 0.0f;

                data[i].positionC_w[0] = 0.0f;
                data[i].positionC_w[1] = 0.0f;
                data[i].positionC_w[2] = 0.0f;
                data[i].positionC_w[3] = 0.0f;

                data[i].normal_w[0] = 0.0f;
                data[i].normal_w[1] = 0.0f;
                data[i].normal_w[2] = 0.0f;
                data[i].normal_w[3] = 0.0f;
            }
        }
    );
    if (!writeOK)
        return false;

    if (!g_pathTrace.Load(g_d3d.Device(), WindowGetHWND(), L"Shaders/PathTrace.fx", c_shaderDebug))
        return false;

    if (!g_shaderShowPathTrace.Load(g_d3d.Device(), WindowGetHWND(), L"Shaders/ShowPathTrace.fx", ShaderData::VertexFormats::Pos2D, ShaderData::VertexFormats::Pos2DElements, c_shaderDebug))
        return false;


    // make a full screen triangle
    writeOK = g_fullScreenMesh.Create(
        g_d3d.Device(),
        [] (std::vector<ShaderTypes::VertexFormats::Pos2D>& vertexData, std::vector<unsigned long>& indexData)
        {
            vertexData.resize(3);
            indexData.resize(3);

            vertexData[0] = { -1.0f,  3.0f, 0.0f, 1.0f };
            vertexData[1] = {  3.0f, -1.0f, 0.0f, 1.0f };
            vertexData[2] = { -1.0f, -1.0f, 0.0f, 1.0f };

            indexData[0] = 0;
            indexData[1] = 1;
            indexData[2] = 2;
        }
    );

    // TODO: maybe separate scene data from other stuff (frameRnd_appTime_sampleCount_w) since it doesn't need to be updated every frame. maybe just seperate "set infrequently" from "set frequently"
    // TODO: make this an initialization of scene data to sane values / defaults (like for near place).
    // TODO: temp scene data!
    writeOK = ShaderData::ConstantBuffers::Scene.Write(
        g_d3d.Context(),
        [] (ShaderTypes::ConstantBuffers::Scene& scene)
        {
            scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 3.0f;
            scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 1.0f;
            scene.numSpheres_numTris_nearPlaneDist_missColor[2] = c_nearPlane;
            scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.0f;

            scene.frameRnd_appTime_sampleCount_numQuads[0] = 0.0f;
            scene.frameRnd_appTime_sampleCount_numQuads[1] = 0.0f;
            scene.frameRnd_appTime_sampleCount_numQuads[2] = 0.0f;
            scene.frameRnd_appTime_sampleCount_numQuads[3] = 0.0f;

            scene.cameraPos_FOVX = { c_cameraPos[0], c_cameraPos[1], c_cameraPos[2], c_fovX };
            scene.cameraAt_FOVY = { c_cameraAt[0], c_cameraAt[1], c_cameraAt[2], c_fovY };
        }
    );
    if (!writeOK)
        return false;

    writeOK = ShaderData::StructuredBuffers::Spheres.Write(
        g_d3d.Context(),
        [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
        {
            spheres[0].position_Radius = { 0.0f, 0.0f, 0.0f, 1.0f };
            spheres[1].position_Radius = { 3.0f, 1.0f, 2.0f, 1.0f };
            spheres[2].position_Radius = { -2.0f, 3.0f, -1.0f, 1.0f };

            spheres[0].albedo_Emissive_zw = { 0.0f, 1.0f, 0.0f, 0.0f };
            spheres[1].albedo_Emissive_zw = { 1.0f, 0.0f, 0.0f, 0.0f };
            spheres[2].albedo_Emissive_zw = { 1.0f, 0.1f, 0.0f, 0.0f };
        }
    );
    if (!writeOK)
        return false;

    writeOK = ShaderData::StructuredBuffers::Triangles.Write(
        g_d3d.Context(),
        [] (std::array<ShaderTypes::StructuredBuffers::TrianglePrim, 10>& triangles)
        {
            triangles[0].positionA_Albedo = { 2.0f, -5.0f, 5.0f, 1.0f };
            triangles[0].positionB_Emissive = { 12.0f, -5.0f, 5.0f, 0.0f };
            triangles[0].positionC_w = { 2.0f, 5.0f, 5.0f, 0.0f };
            triangles[0].normal_w = { 0.0f, 0.0f, -1.0f, 0.0f };
        }
    );
    if (!writeOK)
        return false;

    if (!g_testBuffer.Create(g_d3d.Device(), g_d3d.Context(), c_width, c_height))
        return false;

    return true;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    std::chrono::high_resolution_clock::time_point appStart = std::chrono::high_resolution_clock::now();

    bool done = !init();

    // TODO: remove if we don't need to ever change render targets
    //g_testBuffer.SetAsRenderTarget(g_d3d.Context());

    const size_t dispatchX = 1 + c_width / 32;
    const size_t dispatchY = 1 + c_height / 32;

    FillSceneData(EScene::SphereOnPlane_LowLight, g_d3d.Context());

    size_t frameNumber = 0; // TODO: temp!
    while (!done)
    {
        // Handle the windows messages.
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If windows signals to end the application then exit out.
        if (msg.message == WM_QUIT)
        {
            done = true;
        }
        else
        {
            // update frame specific values
            std::chrono::duration<float> appTimeSeconds = std::chrono::high_resolution_clock::now() - appStart;
            bool writeOK = ShaderData::ConstantBuffers::Scene.Write(
                g_d3d.Context(),
                [&appTimeSeconds] (ShaderTypes::ConstantBuffers::Scene& scene)
            {
                scene.frameRnd_appTime_sampleCount_numQuads[0] = RandomFloat(0.0f, 1.0f);
                scene.frameRnd_appTime_sampleCount_numQuads[1] = appTimeSeconds.count();
                scene.frameRnd_appTime_sampleCount_numQuads[2] += 1.0f;
                
            }
            );
            if (!writeOK)
                done = true;

            // compute
            FillShaderParams<EShaderType::compute>(g_d3d.Context(), g_pathTrace.GetReflector());
            g_pathTrace.Dispatch(g_d3d.Context(), dispatchX, dispatchY, 1);
            UnbindShaderTextures<EShaderType::compute>(g_d3d.Context(), g_pathTrace.GetReflector());

            // vs & ps
            g_d3d.BeginScene(0.4f, 0.0f, 0.4f, 1.0f);
            FillShaderParams<EShaderType::vertex>(g_d3d.Context(), g_shaderShowPathTrace.GetVSReflector());
            FillShaderParams<EShaderType::pixel>(g_d3d.Context(), g_shaderShowPathTrace.GetPSReflector());
            g_fullScreenMesh.Render(g_d3d.Context());
            g_shaderShowPathTrace.Draw(g_d3d.Context(), g_fullScreenMesh.GetIndexCount());
            UnbindShaderTextures<EShaderType::vertex>(g_d3d.Context(), g_shaderShowPathTrace.GetVSReflector());
            UnbindShaderTextures<EShaderType::pixel>(g_d3d.Context(), g_shaderShowPathTrace.GetPSReflector());
            g_d3d.EndScene();

            ++frameNumber;
        }
    }

    return 0;
}