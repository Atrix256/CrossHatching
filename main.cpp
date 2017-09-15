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
#include "IMGUIWrap.h"

// globals
bool g_showGrey = false;
bool g_showCrossHatch = false;
bool g_smoothStep = false;
bool g_aniso = false;
bool g_whiteAlbedo = false;

CModel<ShaderTypes::VertexFormats::Pos2D> g_fullScreenMesh;

// For FPS calculation etc
static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;

float RandomFloat (float min, float max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

bool init ()
{
    WindowInit(c_width, c_height, c_fullScreen);

    if (!g_d3d.Init(c_width, c_height, c_vsync, WindowGetHWND(), c_fullScreen, c_d3ddebug))
        return false;

    if (!ShaderTypesInit())
        return false;

    // make a full screen triangle mesh
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

    if (!writeOK)
    {
        ReportError("Could not create full screen triangle\n");
        return false;
    }

    writeOK = ShaderData::ConstantBuffers::ConstantsOnce.Write(
        g_d3d.Context(),
        [] (ShaderTypes::ConstantBuffers::ConstantsOnce& data)
        {
            data.width_height_zw = { float(c_width), float(c_height), 0.0f, 0.0f };
            data.cameraPos_FOVX = { 0.0f, 0.0f, 0.0f, c_fovX };
            data.cameraAt_FOVY = { 0.0f, 0.0f, 0.0f, c_fovY };
            data.nearPlaneDist_missColor = { 0.0f, 0.0f, 0.0f, 0.0f };
            data.numSpheres_numTris_numOBBs_numQuads = { 0, 0, 0, 0 };
			data.uvmultiplier_blackPoint_whitePoint_w = { 1.0f, 0.0f, 1.0f, 0.0f };
        }
    );
    if (!writeOK)
    {
        ReportError("Could not write constants once\n");
        return false;
    }

    writeOK = ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
        g_d3d.Context(),
        [] (ShaderTypes::ConstantBuffers::ConstantsPerFrame& data)
        {
            data.frameRnd_appTime_zw = { 0.0f, 0.0f, 0.0f, 0.0f };
            data.sampleCount_yzw = {0, 0, 0, 0};
        }
    );
    if (!writeOK)
    {
        ReportError("Could not write constants per frame\n");
        return false;
    }

    if (!InitIMGUI())
    {
        ReportError("Could not init imgui\n");
        return false;
    }

    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
        return false;

    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    return true;
}

void IMGUIWindow ()
{
    // calculate frame time
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    float deltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;

    // set delta time and current time
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;
    g_Time = current_time;

    // start a new frame
    ImGui::NewFrame();

    if (GetIMGUIEnabled())
    {
        // used to see the fully featured built in imgui demo window
        if (0)
        {
            ImGui::ShowTestWindow();
            return;
        }

        // handle FPS calculations
        static int FPSFrameCount = 0;
        static float FPSFrameTime = 0.0f;
        static float FPSLast = 0.0f;
        FPSFrameCount++;
        FPSFrameTime += deltaTime;
        if (FPSFrameTime > 0.5f)
        {
            FPSLast = float(FPSFrameCount) / FPSFrameTime;
            FPSFrameCount = 0;
            FPSFrameTime = 0.0f;
        }

        // handle UI
        static bool firstTime = true;

        static float uvScale = 1.0f;
        static float blackPoint = 0.0f;
        static float whitePoint = 1.0f;
        static int scene = 0;

        bool updateConstants = false;
        bool updateScene = false;

        if (firstTime)
        {
            uvScale = ShaderData::ConstantBuffers::ConstantsOnce.Read().uvmultiplier_blackPoint_whitePoint_w[0];
            blackPoint = ShaderData::ConstantBuffers::ConstantsOnce.Read().uvmultiplier_blackPoint_whitePoint_w[1];
            whitePoint = ShaderData::ConstantBuffers::ConstantsOnce.Read().uvmultiplier_blackPoint_whitePoint_w[2];
        }

        ImGui::Begin("", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        const char* scenes[] = {
            "Sphere Plane Dark",
            "Sphere Plane Light",
            "Cornell Box Small Light",
            "Cornell Box Big Light",
            "Furnace Test",
            "Cornell Obj",
            "Obj Test"
        };

        if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
        {
            updateScene |= ImGui::Combo("Scene", &scene, scenes, (int)EScene::COUNT);
            updateScene |= ImGui::Checkbox("White Albedo", &g_whiteAlbedo);
            if (ImGui::Button("Reset Render"))
            {
                ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
                    g_d3d.Context(),
                    [=](ShaderTypes::ConstantBuffers::ConstantsPerFrame& data)
                {
                    data.sampleCount_yzw[0] = 0;
                }
                );
            }
        }

        if (ImGui::CollapsingHeader("Cross Hatching", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Grey Scale", &g_showGrey);
            ImGui::Checkbox("Cross Hatch", &g_showCrossHatch);
            ImGui::Checkbox("16x Anisotropic Sampling", &g_aniso);
            updateConstants |= ImGui::SliderFloat("UV Scale", &uvScale, 0.001f, 3.0f);
        }

        if (ImGui::CollapsingHeader("Brightness", ImGuiTreeNodeFlags_DefaultOpen))
        {
            updateConstants |= ImGui::SliderFloat("Black", &blackPoint, 0.0f, 1.0f);
            updateConstants |= ImGui::SliderFloat("White", &whitePoint, 0.0f, 1.0f);
            ImGui::Checkbox("Smooth Step Brightness", &g_smoothStep);
        }

        if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
        {
            uint4 counts = ShaderData::ConstantBuffers::ConstantsOnce.Read().numSpheres_numTris_numOBBs_numQuads;
            ImGui::Text("Rendering at %u x %u\nSpheres: %u\nTriangles: %u\nOBBs: %u\nQuads: %u\n", c_width, c_height, counts[0], counts[1], counts[2], counts[3]);
            ImGui::Text("FPS: %0.2f (%0.2f ms)", FPSLast, FPSLast > 0 ? 1000.0f / FPSLast : 0.0f);
            ImGui::Text("%u samples\nFPS is samples per second\n", ShaderData::ConstantBuffers::ConstantsPerFrame.Read().sampleCount_yzw[0]);
            ImGui::Separator();
        }

        // show FPS
        ImGui::Text("Press 'H' to hide or unhide this window. Escape to exit.\nWASD to move camera and drag mouse for mouselook.");

        ImGui::End();

        // update constants
        if (updateConstants)
        {
            ShaderData::ConstantBuffers::ConstantsOnce.Write(
                g_d3d.Context(),
                [=] (ShaderTypes::ConstantBuffers::ConstantsOnce& data)
                {
                    data.uvmultiplier_blackPoint_whitePoint_w[0] = uvScale;
                    data.uvmultiplier_blackPoint_whitePoint_w[1] = blackPoint;
                    data.uvmultiplier_blackPoint_whitePoint_w[2] = whitePoint;
                }
            );
        }

        // update scene if we should
        if (updateScene)
        {
            FillSceneData((EScene)scene, g_d3d.Context());

            uvScale = ShaderData::ConstantBuffers::ConstantsOnce.Read().uvmultiplier_blackPoint_whitePoint_w[0];
            blackPoint = ShaderData::ConstantBuffers::ConstantsOnce.Read().uvmultiplier_blackPoint_whitePoint_w[1];
            whitePoint = ShaderData::ConstantBuffers::ConstantsOnce.Read().uvmultiplier_blackPoint_whitePoint_w[2];
        }
    }

    // handle possible camera movement
    float movementSide = 0.0f;
    float movementForward = 0.0f;
    if (io.KeysDown['W'])
        movementForward += 1.0f;
    if (io.KeysDown['A'])
        movementSide -= 1.0f;
    if (io.KeysDown['S'])
        movementForward -= 1.0f;
    if (io.KeysDown['D'])
        movementSide += 1.0f;

    float mouseMoveX = 0.0f;
    float mouseMoveY = 0.0f;
    if (!io.WantCaptureMouse && io.MouseDown[0])
    {
        mouseMoveX = io.MouseDelta.x;
        mouseMoveY = io.MouseDelta.y;
    }

    if (movementSide != 0.0f || movementForward != 0.0f || mouseMoveX != 0.0f || mouseMoveY != 0.0f)
    {
        // update the camera
        ShaderData::ConstantBuffers::ConstantsOnce.Write(
            g_d3d.Context(),
            [=] (ShaderTypes::ConstantBuffers::ConstantsOnce& data) {

                // calculate camera basis vectors
                float3 cameraFwdRaw = XYZ(data.cameraAt_FOVY) - XYZ(data.cameraPos_FOVX);
                float3 cameraFwd = cameraFwdRaw;
                Normalize(cameraFwd);
                float3 cameraRight = Cross({ 0.0f, 1.0f, 0.0f }, cameraFwd);
                Normalize(cameraRight);
                float3 cameraUp = Cross(cameraFwd, cameraRight);
                Normalize(cameraUp);

                // handle mouse look by changing where the cameraAt position is, relative to the cameraPos
                {
                    // convert "cameraFwdRaw" from cartesian to spherical coordinates
                    float radius = Length(cameraFwdRaw);
                    float theta = atan2(cameraFwdRaw[2], cameraFwdRaw[0]);
                    float phi = atan2(std::sqrtf(cameraFwdRaw[0] * cameraFwdRaw[0] + cameraFwdRaw[2] * cameraFwdRaw[2]), cameraFwdRaw[1]);

                    // adjust spherical coordinates based on mouse drag amounts
                    theta -= mouseMoveX * c_mouseLookSpeed * deltaTime;
                    phi += mouseMoveY * c_mouseLookSpeed * deltaTime;

                    // convert from spherical coordinates back into cartesian
                    cameraFwdRaw[0] = radius * sin(phi) * cos(theta);
                    cameraFwdRaw[2] = radius * sin(theta) * sin(phi);
                    cameraFwdRaw[1] = radius * cos(phi);

                    // set new cameraAt vector
                    data.cameraAt_FOVY[0] = data.cameraPos_FOVX[0] + cameraFwdRaw[0];
                    data.cameraAt_FOVY[1] = data.cameraPos_FOVX[1] + cameraFwdRaw[1];
                    data.cameraAt_FOVY[2] = data.cameraPos_FOVX[2] + cameraFwdRaw[2];
                }

                // translate the camera position and target
                {
                    float3 cameraDelta = (cameraFwd * movementForward + cameraRight * movementSide) * deltaTime * c_walkSpeed;

                    data.cameraPos_FOVX[0] += cameraDelta[0];
                    data.cameraPos_FOVX[1] += cameraDelta[1];
                    data.cameraPos_FOVX[2] += cameraDelta[2];

                    data.cameraAt_FOVY[0] += cameraDelta[0];
                    data.cameraAt_FOVY[1] += cameraDelta[1];
                    data.cameraAt_FOVY[2] += cameraDelta[2];
                }
            }
        );

        // reset the render
        ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
            g_d3d.Context(),
            [=] (ShaderTypes::ConstantBuffers::ConstantsPerFrame& data) {
                data.sampleCount_yzw[0] = 0;
            }
        );
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
            IMGUIWindow();

            // update frame specific values
			bool firstSample = false;
            std::chrono::duration<float> appTimeSeconds = std::chrono::high_resolution_clock::now() - appStart;
            bool writeOK = ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
                g_d3d.Context(),
                [&firstSample, &appTimeSeconds] (ShaderTypes::ConstantBuffers::ConstantsPerFrame& data)
                {
                    data.frameRnd_appTime_zw[0] = RandomFloat(0.0f, 1.0f);
                    data.frameRnd_appTime_zw[1] = appTimeSeconds.count();

                    data.sampleCount_yzw[0]++;

					firstSample = (data.sampleCount_yzw[0] == 1);
                }
            );
            if (!writeOK)
                done = true;

			// if this is sample 0, we need to run the code that generates the first intersection that is re-used by the path tracing and the shader that shows the path tracing results
			if (firstSample)
			{
                const CComputeShader& shader = ShaderData::GetShader_pathTraceFirstHit({});
                FillShaderParams<EShaderType::compute>(g_d3d.Context(), shader.GetReflector());
                shader.Dispatch(g_d3d.Context(), dispatchX, dispatchY, 1);
                UnbindShaderTextures<EShaderType::compute>(g_d3d.Context(), shader.GetReflector());
			}

            // path tracing compute shader
            const CComputeShader& computeShader = ShaderData::GetShader_pathTrace({g_whiteAlbedo});
            FillShaderParams<EShaderType::compute>(g_d3d.Context(), computeShader.GetReflector());
            computeShader.Dispatch(g_d3d.Context(), dispatchX, dispatchY, 1);
            UnbindShaderTextures<EShaderType::compute>(g_d3d.Context(), computeShader.GetReflector());

            // TODO: it really would be nice if passing static branches would let you see names. Maybe a function that returns the correct type? Also make sure that all parameters are required. No silent failures!

            // vs/ps to show the results of the path tracing
            const CShader& shader = ShaderData::GetShader_showPathTrace({g_showGrey, g_showCrossHatch, g_smoothStep, g_aniso});
            FillShaderParams<EShaderType::vertex>(g_d3d.Context(), shader.GetVSReflector());
            FillShaderParams<EShaderType::pixel>(g_d3d.Context(), shader.GetPSReflector());
            g_fullScreenMesh.Set(g_d3d.Context());
            shader.Set(g_d3d.Context());
            g_d3d.DrawIndexed(g_fullScreenMesh.GetIndexCount());
            UnbindShaderTextures<EShaderType::vertex>(g_d3d.Context(), shader.GetVSReflector());
            UnbindShaderTextures<EShaderType::pixel>(g_d3d.Context(), shader.GetPSReflector());

            // render imgui
            ImGui::Render();

            g_d3d.Present();
        }
    }

    return 0;
}