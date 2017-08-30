#include "Scenes.h"
#include "ShaderTypes.h"
#include <d3d11.h>

bool FillSceneData (EScene scene, ID3D11DeviceContext* context)
{
    bool ret = true;

    // reset the sample count back to 0
    ret &= ShaderData::ConstantBuffers::Scene.Write(
        context,
        [](ShaderTypes::ConstantBuffers::Scene& scene)
        {
            scene.frameRnd_appTime_sampleCount_numQuads[2] = 0.0f;
        }
    );

    switch (scene)
    {
        case EScene::SphereOnPlane_LowLight:
        {
            ret &= ShaderData::ConstantBuffers::Scene.Write(
                context,
                [](ShaderTypes::ConstantBuffers::Scene& scene)
                {
                    scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 2.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.0f;

                    scene.frameRnd_appTime_sampleCount_numQuads[3] = 2.0f;
                    scene.numOBBs_yzw[0] = 0.0f;

                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;
                }
            );

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 4.0f, 4.0f, 6.0f }, 0.5f, 0.0f, 10.0f);
                    MakeSphere(spheres[1], { 0.0f, 0.0f, 4.0f }, 2.0f, 0.5f, 0.0f);
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    MakeQuad(Quads[0], { -4.0f, -3.0f, -4.0f }, { -4.0f, 2.0f, -4.0f }, { -4.0f, 2.0f, 12.0f }, { -4.0f, -3.0f, 12.0f }, 0.9f, 0.0f);
                    MakeQuad(Quads[1], { -15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, -15.0f }, { -15.0f, -2.0f, -15.0f }, 0.9f, 0.0f);
                }
            );
            break;
        }
        case EScene::SphereOnPlane_RegularLight:
        {
            ret &= ShaderData::ConstantBuffers::Scene.Write(
                context,
                [](ShaderTypes::ConstantBuffers::Scene& scene)
                {
                    scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 2.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.025f;

                    scene.frameRnd_appTime_sampleCount_numQuads[3] = 2.0f;
                    scene.numOBBs_yzw[0] = 0.0f;

                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;
                }
            );

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 4.0f, 4.0f, 6.0f }, 0.5f, 0.0f, 10.0f);
                    MakeSphere(spheres[1], { 0.0f, 0.0f, 4.0f }, 2.0f, 0.5f, 0.0f);
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    MakeQuad(Quads[0], { -4.0f, -3.0f, -4.0f }, { -4.0f, 2.0f, -4.0f }, { -4.0f, 2.0f, 12.0f }, { -4.0f, -3.0f, 12.0f }, 0.9f, 0.0f);
                    MakeQuad(Quads[1], { -15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, -15.0f }, { -15.0f, -2.0f, -15.0f }, 0.9f, 0.0f);
                }
            );
            break;
        }
        case EScene::CornellBox_SmallLight:
        {
            // TODO: do some HDR to SDR tone mapping
            // TODO: can we somehow unify this stuff into something easier to read and harder to get wrong?
            // TODO: put FOV in the scene data instead of being global. have aspect ratio be global though

            // slightly modified cornell box, from source: http://www.graphics.cornell.edu/online/box/data.html
            ret &= ShaderData::ConstantBuffers::Scene.Write(
                context,
                [](ShaderTypes::ConstantBuffers::Scene& scene)
                {
                    scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.0f;

                    scene.frameRnd_appTime_sampleCount_numQuads[3] = 6.0f;
                    scene.numOBBs_yzw[0] = 2.0f;

                    scene.cameraPos_FOVX[0] = 278.0f;
                    scene.cameraPos_FOVX[1] = 273.0f;
                    scene.cameraPos_FOVX[2] = -800.0f;

                    scene.cameraAt_FOVY[0] = 278.0f;
                    scene.cameraAt_FOVY[1] = 273.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;
                }
            );

            ret &= ShaderData::StructuredBuffers::OBBs.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::OBBPrim, 10>& obbs)
                {
                    obbs[0].position_Albedo = { 185.5f, 82.5f, 169.0f, 1.0f };
                    obbs[0].radius_Emissive = { 82.5f, 82.5f, 82.5f, 0.0f };
                    obbs[0].rotationAxis_rotationAngle = { 0.0f, 1.0f, 0.0f, DegreesToRadians(-17.0f) };
                    
                    obbs[1].position_Albedo = { 368.5f, 165.0f, 351.25f, 1.0f };
                    obbs[1].radius_Emissive = { 82.5f, 165.0f, 82.5f, 0.0f };
                    obbs[1].rotationAxis_rotationAngle = { 0.0f, 1.0f, 0.0f, DegreesToRadians(107.0f) };

                    CalculateOBBNormals(obbs[0]);
                    CalculateOBBNormals(obbs[1]);
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    // Light
                    MakeQuad(Quads[0], { 343.0f, 548.6f, 227.0f }, { 343.0f, 548.6f, 332.0f }, { 213.0f, 548.6f, 332.0f }, { 213.0f, 548.6f, 227.0f }, 0.78f, 25.0f);

                    // Floor
                    MakeQuad(Quads[1], { 552.8f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 559.2f }, { 549.6f, 0.0f, 559.2f }, 1.0f, 0.0f);

                    // Ceiling
                    MakeQuad(Quads[2], { 556.0f, 548.8f,   0.0f }, { 556.0f, 548.8f, 559.2f }, { 0.0f, 548.8f, 559.2f }, { 0.0f, 548.8f,   0.0f }, 1.0f, 0.0f);

                    // Back Wall
                    MakeQuad(Quads[3], { 549.6f,   0.0f, 559.2f }, { 0.0f,   0.0f, 559.2f }, { 0.0f, 548.8f, 559.2f }, { 556.0f, 548.8f, 559.2f }, 1.0f, 0.0f);

                    // Left Wall
                    MakeQuad(Quads[4], { 0.0f,   0.0f, 559.2f }, { 0.0f,   0.0f, 0.0f }, { 0.0f, 548.8f, 0.0f }, { 0.0f, 548.8f, 559.2f }, 1.0f, 0.0f);

                    // Right Wall
                    MakeQuad(Quads[5], { 552.8f, 0.0f,   0.0f }, { 549.6f,   0.0f, 559.2f }, { 556.0f, 548.8f, 559.2f }, { 556.0f, 548.8f,   0.0f }, 1.0f, 0.0f);
                }
            );

            break;
        }
        case EScene::CornellBox_BigLight:
        {
            // slightly modified cornell box, from source: http://www.graphics.cornell.edu/online/box/data.html
            ret &= ShaderData::ConstantBuffers::Scene.Write(
                context,
                [](ShaderTypes::ConstantBuffers::Scene& scene)
                {
                    scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.0f;

                    scene.frameRnd_appTime_sampleCount_numQuads[3] = 5.0f;
                    scene.numOBBs_yzw[0] = 2.0f;

                    scene.cameraPos_FOVX[0] = 278.0f;
                    scene.cameraPos_FOVX[1] = 273.0f;
                    scene.cameraPos_FOVX[2] = -800.0f;

                    scene.cameraAt_FOVY[0] = 278.0f;
                    scene.cameraAt_FOVY[1] = 273.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;
                }
            );

            ret &= ShaderData::StructuredBuffers::OBBs.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::OBBPrim, 10>& obbs)
                {
                    obbs[0].position_Albedo = { 185.5f, 82.5f, 169.0f, 1.0f };
                    obbs[0].radius_Emissive = { 82.5f, 82.5f, 82.5f, 0.0f };
                    obbs[0].rotationAxis_rotationAngle = { 0.0f, 1.0f, 0.0f, DegreesToRadians(-17.0f) };
                    
                    obbs[1].position_Albedo = { 368.5f, 165.0f, 351.25f, 1.0f };
                    obbs[1].radius_Emissive = { 82.5f, 165.0f, 82.5f, 0.0f };
                    obbs[1].rotationAxis_rotationAngle = { 0.0f, 1.0f, 0.0f, DegreesToRadians(107) };

                    CalculateOBBNormals(obbs[0]);
                    CalculateOBBNormals(obbs[1]);
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    // Floor
                    MakeQuad(Quads[1], { 552.8f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 559.2f }, { 549.6f, 0.0f, 559.2f }, 1.0f, 0.0f);

                    // Ceiling
                    MakeQuad(Quads[2], { 556.0f, 548.8f,   0.0f }, { 556.0f, 548.8f, 559.2f }, { 0.0f, 548.8f, 559.2f }, { 0.0f, 548.8f,   0.0f }, 0.78f, 1.0f);

                    // Back Wall
                    MakeQuad(Quads[3], { 549.6f,   0.0f, 559.2f }, { 0.0f,   0.0f, 559.2f }, { 0.0f, 548.8f, 559.2f }, { 556.0f, 548.8f, 559.2f }, 1.0f, 0.0f);

                    // Left Wall
                    MakeQuad(Quads[4], { 0.0f,   0.0f, 559.2f }, { 0.0f,   0.0f, 0.0f }, { 0.0f, 548.8f, 0.0f }, { 0.0f, 548.8f, 559.2f }, 1.0f, 0.0f);

                    // Right Wall
                    MakeQuad(Quads[0], { 552.8f, 0.0f,   0.0f }, { 549.6f,   0.0f, 559.2f }, { 556.0f, 548.8f, 559.2f }, { 556.0f, 548.8f,   0.0f }, 1.0f, 0.0f);
                }
            );

            break;
        }
        case EScene::FurnaceTest:
        {
            ret &= ShaderData::ConstantBuffers::Scene.Write(
                context,
                [](ShaderTypes::ConstantBuffers::Scene& scene)
                {
                    scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 2.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 0.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.0f;

                    scene.frameRnd_appTime_sampleCount_numQuads[3] = 0.0f;
                    scene.numOBBs_yzw[0] = 0.0f;

                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;
                }
            );

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 0.0f, 0.0f, 4.0f }, 2.0f, 1.0f, 0.0f);
                    MakeSphere(spheres[1], { 0.0f, 0.0f, 0.0f }, 20.0f, 0.0f, 0.5f);
                }
            );
            break;
        }
        default:
        {
            ret = false;
        }
    }

    return ret;
}