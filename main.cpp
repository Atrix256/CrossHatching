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
};

bool WriteShaderTypesHLSL (void)
{
    FILE *file = fopen("Shaders/ShaderTypes.h", "w+t");
    if (!file)
        return false;

    // write the cbuffer declarations
    #define CONSTANT_BUFFER_BEGIN(NAME) fprintf(file, "cbuffer " #NAME "\n{\n");
    #define CONSTANT_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define CONSTANT_BUFFER_END fprintf(file, "};\n\n");

    // write the struct declarations for structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) fprintf(file, "struct " #TYPENAME "\n{\n");
    #define STRUCTURED_BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define STRUCTURED_BUFFER_END fprintf(file, "};\n\n");

    #include "ShaderTypesList.h"

    // write the structured buffer declarations
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) fprintf(file, "StructuredBuffer<" #TYPENAME ">" #NAME ";\n\n");

    #include "ShaderTypesList.h"

    fclose(file);
    return true;
}

template <EShaderType SHADER_TYPE>
void FillShaderParams (ID3D11DeviceContext* deviceContext, ID3D11ShaderReflection* reflector)
{
    D3D11_SHADER_INPUT_BIND_DESC desc;
    HRESULT result;

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

    // create constant buffers
    #define CONSTANT_BUFFER_BEGIN(NAME) if (!ShaderData::ConstantBuffers::NAME.Create(D3D11GetDevice())) done = true;
    #include "ShaderTypesList.h"

    // create structured buffers
    #define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT) if (!ShaderData::StructuredBuffers::NAME.Create(D3D11GetDevice())) done = true;
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

    CTexture texture;
    if (!texture.LoadTGA(D3D11GetDevice(), D3D11GetContext(), "stone01.tga"))
        done = true;

    CRenderTarget testBuffer;
    if (!testBuffer.Create(D3D11GetDevice(), D3D11GetContext(), width, height))
        done = true;

    CTexture rwTexture;
    if (!rwTexture.Create(D3D11GetDevice(), D3D11GetContext(), width, height))
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


            // TODO: temp!
            // TODO: make this part of the dispatch call or something...
            ID3D11UnorderedAccessView *uav = rwTexture.GetTextureCompute();
            UINT count = -1;
            D3D11GetContext()->CSSetUnorderedAccessViews(0, 1, &uav, &count);
            computeShader.Dispatch(D3D11GetContext(), dispatchX, dispatchY, 1);
            FillShaderParams<EShaderType::compute>(D3D11GetContext(), computeShader.GetReflector());
            uav = NULL;
            D3D11GetContext()->CSSetUnorderedAccessViews(0, 1, &uav, &count);


            D3D11BeginScene(0.4f, 0.0f, 0.4f, 1.0f);

            FillShaderParams<EShaderType::vertex>(D3D11GetContext(), shader.GetVSReflector());
            FillShaderParams<EShaderType::pixel>(D3D11GetContext(), shader.GetVSReflector());

            shader.SetConstants(D3D11GetContext(), rwTexture.GetTexture());
            model.Render(D3D11GetContext());
            shader.Draw(D3D11GetContext(), model.GetIndexCount());
            shader.SetConstants(D3D11GetContext(), nullptr);
            D3D11EndScene();

            ++frameNumber;
        }
    }

    D3D11Shutdown();

    return 0;
}