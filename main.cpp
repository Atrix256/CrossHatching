#include "d3d11.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Window.h"

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    MSG msg;
    bool done = false;

    // settings
    size_t width = 800;
    size_t height = 600;
    bool fullScreen = false;
    bool vsync = true;
    bool shaderDebug = false;
    bool d3ddebug = false;

    WindowInit(width, height, fullScreen);

    // TODO: if it fails, report why to a log or something. Or maybe show a message box
    if (!D3D11Init(width, height, vsync, WindowGetHWND(), fullScreen, 100.0f, 0.01f, d3ddebug))
        done = true;

    CShader shader;
    if (!shader.Load(D3D11GetDevice(), WindowGetHWND(), L"shader.fx", shaderDebug))
        done = true;

    // TODO: delete when working
    // compute shader info: http://recreationstudios.blogspot.com/2010/04/simple-compute-shader-example.html
    CComputeShader computeShader;
    if (!computeShader.Load(D3D11GetDevice(), WindowGetHWND(), L"computeshader.fx", shaderDebug))
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
            constantBuffer.color[0] = 2.0f;
            constantBuffer.color[1] = 1.0f;
            constantBuffer.color[2] = 2.0f;
            constantBuffer.color[3] = 3.0f;

            D3D11BeginScene(0.4f, 0.0f, 0.4f, 1.0f);
            shader.SetConstants(D3D11GetContext(), constantBuffer, texture.GetTexture());
            model.Render(D3D11GetContext());
            shader.Draw(D3D11GetContext(), model.GetIndexCount());
            D3D11EndScene();
        }
    }

    D3D11Shutdown();

    return 0;
}