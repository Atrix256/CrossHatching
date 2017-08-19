#define _CRT_SECURE_NO_WARNINGS

#include "d3d11.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Window.h"
#include "ShaderTypes.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"

// settings
const size_t c_width = 800;
const size_t c_height = 600;
const bool c_fullScreen = false;
const bool c_vsync = true;
const bool c_shaderDebug = true;
const bool c_d3ddebug = true;

// globals
CD3D11 g_d3d;
CShader g_shader;
CComputeShader g_computeShader;
CModel<ShaderTypes::VertexFormats::PosColorUV> g_model;
CRenderTarget g_testBuffer;

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

    // write the texture declarations
    #define TEXTURE(NAME, FILENAME) fprintf(file, "Texture2D " #NAME ";\nRWTexture2D<float4> " #NAME "_rw;\n\n");

    // write the cbuffer declarations
    #define CONSTANT_BUFFER_BEGIN(NAME) fprintf(file, "cbuffer " #NAME "\n{\n");
    #define CONSTANT_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define CONSTANT_BUFFER_END fprintf(file, "};\n\n");

    // write the struct declarations for structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) fprintf(file, "struct " #TYPENAME "\n{\n");
    #define STRUCTURED_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define STRUCTURED_BUFFER_END fprintf(file, "};\n\n");

    // write the vertex formats
    #define VERTEX_FORMAT_BEGIN(NAME) fprintf(file, "struct " #NAME "\n{\n");
    #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, TYPE, FORMAT) fprintf(file, "  " #TYPE " " #NAME " : " #SEMANTIC #INDEX ";\n");
    #define VERTEX_FORMAT_END fprintf(file, "};\n\n");

    #include "ShaderTypesList.h"

    // write the structured buffer declarations
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
}

bool init ()
{
    WindowInit(c_width, c_height, c_fullScreen);

    // TODO: if anything fails, report why to a log or something. Or maybe show a message box
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
        [] (std::array<ShaderTypes::StructuredBuffers::Triangle, 10>& data)
        {
            for (size_t i = 0; i < 10; ++i)
            {
                data[i].position[0] = float(i);
                data[i].position[1] = float(i) + 0.1f;
                data[i].position[2] = float(i) + 0.2f;
            }
        }
    );
    if (!writeOK)
        return false;

    writeOK = ShaderData::StructuredBuffers::Input.Write(
        g_d3d.Context(),
        [] (std::array<ShaderTypes::StructuredBuffers::SBufferItem, 1>& data)
        {
            data[0].c[0] = 0.4f;
            data[0].c[1] = 0.6f;
            data[0].c[2] = 0.8f;
            data[0].c[3] = 1.0f;
        }
    );
    if (!writeOK)
        return false;

    if (!g_shader.Load(g_d3d.Device(), WindowGetHWND(), L"Shaders/shader.fx", ShaderData::VertexFormats::PosColorUV, ShaderData::VertexFormats::PosColorUVElements, c_shaderDebug))
        return false;

    if (!g_computeShader.Load(g_d3d.Device(), WindowGetHWND(), L"Shaders/computeshader.fx", c_shaderDebug))
        return false;

    // create a simple triangle model
    writeOK = g_model.Create(
        g_d3d.Device(),
        [] (std::vector<ShaderTypes::VertexFormats::PosColorUV>& vertexData, std::vector<unsigned long>& indexData)
        {
            // Create the vertex array.
            vertexData.resize(3);

            // Create the index array.
            indexData.resize(3);

            // Load the vertex array with data.
            vertexData[0].position = { -1.0f, -1.0f, 0.0f };  // Bottom left.
            vertexData[0].color = { 1.0f, 1.0f, 0.0f, 1.0f };
            vertexData[0].uv = { 0.0f, 0.0f };

            vertexData[1].position = { 0.0f, 1.0f, 0.0f };  // Top middle.
            vertexData[1].color = { 0.0f, 1.0f, 1.0f, 1.0f };
            vertexData[1].uv = { 0.5f, 1.0f };

            vertexData[2].position = { 1.0f, -1.0f, 0.0f };  // Bottom right.
            vertexData[2].color = { 1.0f, 0.0f, 1.0f, 1.0f };
            vertexData[2].uv = { 1.0f, 0.0f };

            // Load the index array with data.
            indexData[0] = 0;  // Bottom left.
            indexData[1] = 1;  // Top middle.
            indexData[2] = 2;  // Bottom right.
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
    bool done = !init();

    // TODO: remove if we don't need to ever change render targets
    //g_testBuffer.SetAsRenderTarget(g_d3d.Context());

    const size_t dispatchX = 1 + c_width / 32;
    const size_t dispatchY = 1 + c_height / 32;

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
            // update the constant buffer
            bool writeOk = ShaderData::ConstantBuffers::Constants.Write(
                g_d3d.Context(),
                [frameNumber] (ShaderTypes::ConstantBuffers::Constants& constants)
                {
                    constants.pixelColor[0] = float(frameNumber % 4) / float(8.0f) + 0.5f;
                    constants.pixelColor[1] = 1.0f;
                    constants.pixelColor[2] = 2.0f;
                    constants.pixelColor[3] = 3.0f;
                }
            );
            if (!writeOk)
                done = true;

            // compute shader
            FillShaderParams<EShaderType::compute>(g_d3d.Context(), g_computeShader.GetReflector());
            g_computeShader.Dispatch(g_d3d.Context(), dispatchX, dispatchY, 1);
            UnbindShaderTextures<EShaderType::compute>(g_d3d.Context(), g_computeShader.GetReflector());

            // vs & ps
            g_d3d.BeginScene(0.4f, 0.0f, 0.4f, 1.0f);
            FillShaderParams<EShaderType::vertex>(g_d3d.Context(), g_shader.GetVSReflector());
            FillShaderParams<EShaderType::pixel>(g_d3d.Context(), g_shader.GetPSReflector());
            g_model.Render(g_d3d.Context());
            g_shader.Draw(g_d3d.Context(), g_model.GetIndexCount());
            UnbindShaderTextures<EShaderType::vertex>(g_d3d.Context(), g_shader.GetVSReflector());
            UnbindShaderTextures<EShaderType::pixel>(g_d3d.Context(), g_shader.GetPSReflector());
            g_d3d.EndScene();

            ++frameNumber;
        }
    }

    return 0;
}