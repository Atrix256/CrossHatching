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
#include "VertexFormat.h"

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
};

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
    #define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, TYPE) fprintf(file, "  " #TYPE " " #NAME " : " #SEMANTIC ";\n");
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

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    MSG msg;
    bool done = false;

    // settings
    size_t width = 800;
    size_t height = 600;
    bool fullScreen = false;
    bool vsync = true;
    bool shaderDebug = true;
    bool d3ddebug = true;

    WindowInit(width, height, fullScreen);

    if (!WriteShaderTypesHLSL())
        done = true;

    // TODO: if it fails, report why to a log or something. Or maybe show a message box
    if (!D3D11Init(width, height, vsync, WindowGetHWND(), fullScreen, 100.0f, 0.01f, d3ddebug))
        done = true;

    // TODO: move the automatic init stuff into it's own function to declutter main. Maybe move all init stuff there?

    // TODO: there may be problems because we are giving "TEXCOORD0" but should just give "TEXCOORD". maybe has to do with semantic index.
    CVertexFormat<3> vertexFormat;
    const char* names[3] = { "POSITION", "COLOR", "TEXCOORD0" };
    UINT indices[3] = { 0, 0, 0 };
    if (!vertexFormat.Create(names, indices))
        done = true;

    // create constant buffers
    #define CONSTANT_BUFFER_BEGIN(NAME) if (!ShaderData::ConstantBuffers::NAME.Create(D3D11GetDevice())) done = true;
    #include "ShaderTypesList.h"

    // create structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) if (!ShaderData::StructuredBuffers::NAME.Create(D3D11GetDevice())) done = true;
    #include "ShaderTypesList.h"

    // create textures
    #define TEXTURE(NAME, FILENAME) \
        if (FILENAME == nullptr) \
            done = ShaderData::Textures::NAME.Create(D3D11GetDevice(), D3D11GetContext(), width, height) && done; \
        else \
            done = ShaderData::Textures::NAME.LoadTGA(D3D11GetDevice(), D3D11GetContext(), FILENAME) && done; 
    #include "ShaderTypesList.h"

    // write some triangles
    bool writeOK = ShaderData::StructuredBuffers::Triangles.Write(
        D3D11GetContext(),
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
        done = true;

    writeOK = ShaderData::StructuredBuffers::Input.Write(
        D3D11GetContext(),
        [] (std::array<ShaderTypes::StructuredBuffers::SBufferItem, 1>& data)
        {
            data[0].c[0] = 0.4f;
            data[0].c[1] = 0.6f;
            data[0].c[2] = 0.8f;
            data[0].c[3] = 1.0f;
        }
    );
    if (!writeOK)
        done = true;

    CShader shader;
    if (!shader.Load(D3D11GetDevice(), WindowGetHWND(), L"Shaders/shader.fx", shaderDebug))
        done = true;

    // TODO: delete when working
    // compute shader info: http://recreationstudios.blogspot.com/2010/04/simple-compute-shader-example.html
    CComputeShader computeShader;
    if (!computeShader.Load(D3D11GetDevice(), WindowGetHWND(), L"Shaders/computeshader.fx", shaderDebug))
        done = true;

    CModel model;
    if (!model.Load(D3D11GetDevice()))
        done = true;

    CRenderTarget testBuffer;
    if (!testBuffer.Create(D3D11GetDevice(), D3D11GetContext(), width, height))
        done = true;

    // TODO: temp! write to test buffer render target
    //testBuffer.SetAsRenderTarget(D3D11GetContext());

    size_t dispatchX = 1 + width / 32;
    size_t dispatchY = 1 + height / 32;

    size_t frameNumber = 0; // TODO: temp!
    while (!done)
    {
        // Handle the windows messages.
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
                D3D11GetContext(),
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
            FillShaderParams<EShaderType::compute>(D3D11GetContext(), computeShader.GetReflector());
            computeShader.Dispatch(D3D11GetContext(), dispatchX, dispatchY, 1);
            UnbindShaderTextures<EShaderType::compute>(D3D11GetContext(), computeShader.GetReflector());

            // vs & ps
            D3D11BeginScene(0.4f, 0.0f, 0.4f, 1.0f);
            FillShaderParams<EShaderType::vertex>(D3D11GetContext(), shader.GetVSReflector());
            FillShaderParams<EShaderType::pixel>(D3D11GetContext(), shader.GetPSReflector());
            model.Render(D3D11GetContext());
            shader.Draw(D3D11GetContext(), model.GetIndexCount());
            UnbindShaderTextures<EShaderType::vertex>(D3D11GetContext(), shader.GetVSReflector());
            UnbindShaderTextures<EShaderType::pixel>(D3D11GetContext(), shader.GetPSReflector());
            D3D11EndScene();

            ++frameNumber;
        }
    }

    D3D11Shutdown();

    return 0;
}