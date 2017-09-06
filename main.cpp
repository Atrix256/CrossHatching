#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <random>
#include "d3d11.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "Window.h"
#include "ShaderTypes.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "Scenes.h"

// settings
const size_t c_width = 800;
const size_t c_height = 600;
const bool c_fullScreen = false;
const bool c_vsync = false;
const bool c_shaderDebug = true;
const bool c_d3ddebug = true; // TODO: turn this off
const float c_fovX = DegreesToRadians(40.0f);
const float c_fovY = c_fovX * float(c_height) / float(c_width);
const float c_nearPlane = 0.1f;
const float3 c_cameraPos = { 0.0f, 0.0f, -10.0f };
const float3 c_cameraAt = { 0.0f, 0.0f, 0.0f };

// globals
CD3D11 g_d3d;

// TODO: temp
CTexture crossHatching;

bool g_showGrey = false;
bool g_showCrossHatch = false;

CModel<ShaderTypes::VertexFormats::Pos2D> g_fullScreenMesh;

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
        #define TEXTURE_IMAGE(NAME, FILENAME) CTexture NAME;
        #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) CTexture NAME;
        #include "ShaderTypesList.h"
    }

    namespace Shaders
    {
        #define SHADER_CS(NAME, FILENAME, ENTRY) CComputeShader NAME;
        #define SHADER_VSPS(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) CShader NAME;
        #include "ShaderTypesList.h"
    };

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
    fprintf(file, "SamplerState SamplerLinearWrap;\nSamplerState SamplerNearestWrap;\n\n");

    // write the texture declarations
    fprintf(file, "//----------------------------------------------------------------------------\n//Textures\n//----------------------------------------------------------------------------\n");
    #define TEXTURE_IMAGE(NAME, FILENAME) fprintf(file, "Texture2D " #NAME ";\nRWTexture2D<float4> " #NAME "_rw;\n\n");
    #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) fprintf(file, "Texture2D " #NAME ";\nRWTexture2D<" #SHADERTYPE "> " #NAME "_rw;\n\n");
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

    #include "ShaderTypesList.h"

    // TODO: temp! hard coded textures
    result = reflector->GetResourceBindingDescByName("chvolume", &desc);
    if (!FAILED(result))
    {
        ID3D11ShaderResourceView* srv = crossHatching.GetSRV();
        if (SHADER_TYPE == EShaderType::vertex) 
            deviceContext->VSSetShaderResources(desc.BindPoint, 1, &srv); 
        else if (SHADER_TYPE == EShaderType::pixel) 
            deviceContext->PSSetShaderResources(desc.BindPoint, 1, &srv); 
        else 
            deviceContext->CSSetShaderResources(desc.BindPoint, 1, &srv); 
    }

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
        case '3': FillSceneData(EScene::CornellBox_SmallLight, g_d3d.Context()); break;
        case '4': FillSceneData(EScene::CornellBox_BigLight, g_d3d.Context()); break;
        case '5': FillSceneData(EScene::FurnaceTest, g_d3d.Context()); break;
        case '6': FillSceneData(EScene::CornellObj, g_d3d.Context()); break;
        case '7': FillSceneData(EScene::ObjTest, g_d3d.Context()); break;

        case 'C': g_showCrossHatch = !g_showCrossHatch; break;
        case 'G': g_showGrey = !g_showGrey; break;
    }
}

bool init ()
{
    WindowInit(c_width, c_height, c_fullScreen, OnKeyPress);

    if (!g_d3d.Init(c_width, c_height, c_vsync, WindowGetHWND(), c_fullScreen, c_d3ddebug))
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
    #define TEXTURE_IMAGE(NAME, FILENAME) \
        if(!ShaderData::Textures::NAME.LoadTGA(g_d3d.Device(), g_d3d.Context(), FILENAME)) return false;
    #define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT) \
        if(!ShaderData::Textures::NAME.Create(g_d3d.Device(), g_d3d.Context(), c_width, c_height, FORMAT)) return false;
    #include "ShaderTypesList.h"

    // create shaders
    #define SHADER_CS(NAME, FILENAME, ENTRY) \
        if (!ShaderData::Shaders::NAME.Load(g_d3d.Device(), WindowGetHWND(), FILENAME, ENTRY, c_shaderDebug)) return false;
    #define SHADER_VSPS(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT) \
        if (!ShaderData::Shaders::NAME.Load(g_d3d.Device(), WindowGetHWND(), FILENAME, VSENTRY, PSENTRY, ShaderData::VertexFormats::VERTEXFORMAT, ShaderData::VertexFormats::VERTEXFORMAT##Elements, c_shaderDebug)) return false;
    #include "ShaderTypesList.h"

    // make a full screen triangle
   bool writeOK = g_fullScreenMesh.Create(
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

    writeOK = ShaderData::ConstantBuffers::ConstantsOnce.Write(
        g_d3d.Context(),
        [] (ShaderTypes::ConstantBuffers::ConstantsOnce& data)
        {
            data.cameraPos_FOVX = { 0.0f, 0.0f, 0.0f, c_fovX };
            data.cameraAt_FOVY = { 0.0f, 0.0f, 0.0f, c_fovY };
            data.nearPlaneDist_missColor = { 0.0f, 0.0f, 0.0f, 0.0f };
            data.numSpheres_numTris_numOBBs_numQuads = { 0, 0, 0, 0 };
        }
    );
    if (!writeOK)
        return false;

    writeOK = ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
        g_d3d.Context(),
        [] (ShaderTypes::ConstantBuffers::ConstantsPerFrame& data)
        {
            data.frameRnd_appTime_zw = { 0.0f, 0.0f, 0.0f, 0.0f };
            data.sampleCount_yzw = {0, 0, 0, 0};
        }
    );
    if (!writeOK)
        return false;

    return true;
}

CShader& SelectShaderShowPathTrace ()
{
    if (g_showGrey)
    {
        if (g_showCrossHatch)
            return ShaderData::Shaders::showPathTrace_Grey_CrossHatch;
        else
            return ShaderData::Shaders::showPathTrace_Grey_Shade;
    }
    else
    {
        if (g_showCrossHatch)
            return ShaderData::Shaders::showPathTrace_Color_CrossHatch;
        else
            return ShaderData::Shaders::showPathTrace_Color_Shade;
    }
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    std::chrono::high_resolution_clock::time_point appStart = std::chrono::high_resolution_clock::now();

    if (!init())
        return 0;

    const size_t dispatchX = 1 + c_width / 32;
    const size_t dispatchY = 1 + c_height / 32;

    FillSceneData(EScene::SphereOnPlane_LowLight, g_d3d.Context());

    // TODO: formalize this after it's working
    std::vector<CTexture*> slices;
    slices.push_back(&ShaderData::Textures::crosshatch0);
    slices.push_back(&ShaderData::Textures::crosshatch1);
    slices.push_back(&ShaderData::Textures::crosshatch2);
    slices.push_back(&ShaderData::Textures::crosshatch3);
    slices.push_back(&ShaderData::Textures::crosshatch4);
    slices.push_back(&ShaderData::Textures::crosshatch5);
    slices.push_back(&ShaderData::Textures::crosshatch6);
    slices.push_back(&ShaderData::Textures::crosshatch7);
    slices.push_back(&ShaderData::Textures::crosshatch8);

    if (!crossHatching.CreateVolume(g_d3d.Device(), g_d3d.Context(), 193, 193, 9, slices, DXGI_FORMAT_R8G8B8A8_UNORM))
        return 0;

    bool done = false;
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
            bool writeOK = ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
                g_d3d.Context(),
                [&appTimeSeconds] (ShaderTypes::ConstantBuffers::ConstantsPerFrame& data)
                {
                    data.frameRnd_appTime_zw[0] = RandomFloat(0.0f, 1.0f);
                    data.frameRnd_appTime_zw[1] = appTimeSeconds.count();

                    data.sampleCount_yzw[0]++;
                }
            );
            if (!writeOK)
                done = true;

            // compute
            FillShaderParams<EShaderType::compute>(g_d3d.Context(), ShaderData::Shaders::pathTrace.GetReflector());
            ShaderData::Shaders::pathTrace.Dispatch(g_d3d.Context(), dispatchX, dispatchY, 1);
            UnbindShaderTextures<EShaderType::compute>(g_d3d.Context(), ShaderData::Shaders::pathTrace.GetReflector());

            // vs & ps
            CShader& shader = SelectShaderShowPathTrace();
            FillShaderParams<EShaderType::vertex>(g_d3d.Context(), shader.GetVSReflector());
            FillShaderParams<EShaderType::pixel>(g_d3d.Context(), shader.GetPSReflector());
            g_fullScreenMesh.Render(g_d3d.Context());
            shader.Draw(g_d3d.Context(), g_fullScreenMesh.GetIndexCount());
            UnbindShaderTextures<EShaderType::vertex>(g_d3d.Context(), shader.GetVSReflector());
            UnbindShaderTextures<EShaderType::pixel>(g_d3d.Context(), shader.GetPSReflector());
            g_d3d.Present();
        }
    }

    return 0;
}