#include "d3d11.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
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

    WindowInit(width, height, fullScreen);

    // TODO: if it fails, report why to a log or something. Or maybe show a message box
    if (!D3D11Init(width, height, vsync, WindowGetHWND(), fullScreen, 100.0f, 0.01f))
        done = true;

    CShader shader;
    if (!shader.Load(D3D11GetDevice(), WindowGetHWND(), L"shader.fx", shaderDebug))
        done = true;

    if (!ModelInit(D3D11GetDevice()))
        done = true;

    CTexture texture;
    if (!texture.LoadTGA(D3D11GetDevice(), D3D11GetContext(), "stone01.tga"))
        done = true;

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
            ModelRender(D3D11GetContext());
            shader.Draw(D3D11GetContext(), 3);
            D3D11EndScene();
        }
    }

    ModelShutdown();
    D3D11Shutdown();

    return 0;
}