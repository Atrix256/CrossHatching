#include "d3d11.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Window.h"
#include "ShaderTypes.h"

bool WriteShaderTypesHLSL(void)
{
    FILE *file = fopen("Shaders/ShaderTypes.h", "w+t");
    if (!file)
        return false;

    #define CONSTANT_BUFFER_BEGIN(NAME) fprintf(file, "cbuffer " #NAME "\n{\n");
    #define BUFFER_FIELD(NAME, TYPE) fprintf(file,"  " #TYPE " " #NAME ";\n");
    #define CONSTANT_BUFFER_END fprintf(file, "};\n");
    #include "ShaderTypesList.h"

    fclose(file);
    return true;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    // TODO: temp!
    ShaderTypes::Constants s;

    

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
            SConstantBuffer constantBuffer;
            constantBuffer.color[0] = float(frameNumber % 4) / float(8.0f) + 0.5f;
            constantBuffer.color[1] = 1.0f;
            constantBuffer.color[2] = 2.0f;
            constantBuffer.color[3] = 3.0f;

            // TODO: temp!
            // TODO: make this part of the dispatch call or something...
            ID3D11UnorderedAccessView *uav = rwTexture.GetTextureCompute();
            UINT count = -1;
            D3D11GetContext()->CSSetUnorderedAccessViews(0, 1, &uav, &count);
            computeShader.Dispatch(D3D11GetContext(), constantBuffer, dispatchX, dispatchY, 1);
            uav = NULL;
            D3D11GetContext()->CSSetUnorderedAccessViews(0, 1, &uav, &count);


            D3D11BeginScene(0.4f, 0.0f, 0.4f, 1.0f);
            shader.SetConstants(D3D11GetContext(), constantBuffer, rwTexture.GetTexture());
            model.Render(D3D11GetContext());
            shader.Draw(D3D11GetContext(), model.GetIndexCount());
            shader.SetConstants(D3D11GetContext(), constantBuffer, nullptr);
            D3D11EndScene();

            ++frameNumber;
        }
    }

    D3D11Shutdown();

    return 0;
}