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
                    spheres[0].position_Radius = { 4.0f, 4.0f, 6.0f, 0.5f };
                    spheres[0].albedo_Emissive_zw = { 0.0f, 10.0f, 0.0f, 0.0f };

                    spheres[1].position_Radius = { 0.0f, 0.0f, 4.0f, 2.0f };
                    spheres[1].albedo_Emissive_zw = { 0.5f, 0.0f, 0.0f, 0.0f };
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    Quads[0].positionA_Albedo = { -4.0f, -3.0f, -4.0f, 0.9f };
                    Quads[0].positionB_Emissive = { -4.0f, 2.0f, -4.0f, 0.0f };
                    Quads[0].positionC_w = { -4.0f, 2.0f, 12.0f, 0.0f };
                    Quads[0].positionD_w = { -4.0f, -3.0f, 12.0f, 0.0f };

                    Quads[1].positionA_Albedo = { -15.0f, -2.0f, 15.0f, 0.9f };
                    Quads[1].positionB_Emissive = { 15.0f, -2.0f, 15.0f, 0.0f };
                    Quads[1].positionC_w = { 15.0f, -2.0f, -15.0f, 0.0f };
                    Quads[1].positionD_w = { -15.0f, -2.0f, -15.0f, 0.0f };

                    CalculateQuadNormal(Quads[0]);
                    CalculateQuadNormal(Quads[1]);
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
                    spheres[0].position_Radius = { 4.0f, 4.0f, 6.0f, 0.5f };
                    spheres[0].albedo_Emissive_zw = { 0.0f, 10.0f, 0.0f, 0.0f };

                    spheres[1].position_Radius = { 0.0f, 0.0f, 4.0f, 2.0f };
                    spheres[1].albedo_Emissive_zw = { 0.5f, 0.0f, 0.0f, 0.0f };
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    Quads[0].positionA_Albedo = { -4.0f, -3.0f, -4.0f, 0.9f };
                    Quads[0].positionB_Emissive = { -4.0f, 2.0f, -4.0f, 0.0f };
                    Quads[0].positionC_w = { -4.0f, 2.0f, 12.0f, 0.0f };
                    Quads[0].positionD_w = { -4.0f, -3.0f, 12.0f, 0.0f };

                    Quads[1].positionA_Albedo = { -15.0f, -2.0f, 15.0f, 0.9f };
                    Quads[1].positionB_Emissive = { 15.0f, -2.0f, 15.0f, 0.0f };
                    Quads[1].positionC_w = { 15.0f, -2.0f, -15.0f, 0.0f };
                    Quads[1].positionD_w = { -15.0f, -2.0f, -15.0f, 0.0f };

                    CalculateQuadNormal(Quads[0]);
                    CalculateQuadNormal(Quads[1]);
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
                    Quads[0].positionA_Albedo = { 343.0f, 548.6f, 227.0f, 0.78f };
                    Quads[0].positionB_Emissive = { 343.0f, 548.6f, 332.0f, 25.0f };
                    Quads[0].positionC_w = { 213.0f, 548.6f, 332.0f, 0.0f };
                    Quads[0].positionD_w = { 213.0f, 548.6f, 227.0f, 0.0f };

                    // Floor
                    Quads[1].positionA_Albedo = { 552.8f, 0.0f, 0.0f, 1.0f };
                    Quads[1].positionB_Emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
                    Quads[1].positionC_w = { 0.0f, 0.0f, 559.2f, 0.0f };
                    Quads[1].positionD_w = { 549.6f, 0.0f, 559.2f, 0.0f };

                    // Ceiling
                    Quads[2].positionA_Albedo = { 556.0f, 548.8f,   0.0f, 1.0f };
                    Quads[2].positionB_Emissive = { 556.0f, 548.8f, 559.2f, 0.0f };
                    Quads[2].positionC_w = { 0.0f, 548.8f, 559.2f, 0.0f };
                    Quads[2].positionD_w = { 0.0f, 548.8f,   0.0f, 0.0f };

                    // Back Wall
                    Quads[3].positionA_Albedo = { 549.6f,   0.0f, 559.2f, 1.0f };
                    Quads[3].positionB_Emissive = { 0.0f,   0.0f, 559.2f, 0.0f };
                    Quads[3].positionC_w = { 0.0f, 548.8f, 559.2f, 0.0f };
                    Quads[3].positionD_w = { 556.0f, 548.8f, 559.2f, 0.0f };

                    // Left Wall
                    Quads[4].positionA_Albedo = { 0.0f,   0.0f, 559.2f, 1.0f };
                    Quads[4].positionB_Emissive = { 0.0f,   0.0f, 0.0f, 0.0f };
                    Quads[4].positionC_w = { 0.0f, 548.8f, 0.0f, 0.0f };
                    Quads[4].positionD_w = { 0.0f, 548.8f, 559.2f, 0.0f };

                    // Right Wall
                    Quads[5].positionA_Albedo = { 552.8f, 0.0f,   0.0f, 1.0f };
                    Quads[5].positionB_Emissive = { 549.6f,   0.0f, 559.2f, 0.0f };
                    Quads[5].positionC_w = { 556.0f, 548.8f, 559.2f, 0.0f };
                    Quads[5].positionD_w = { 556.0f, 548.8f,   0.0f, 0.0f };

                    CalculateQuadNormal(Quads[0]);
                    CalculateQuadNormal(Quads[1]);
                    CalculateQuadNormal(Quads[2]);
                    CalculateQuadNormal(Quads[3]);
                    CalculateQuadNormal(Quads[4]);
                    CalculateQuadNormal(Quads[5]);
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
                    Quads[1].positionA_Albedo = { 552.8f, 0.0f, 0.0f, 1.0f };
                    Quads[1].positionB_Emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
                    Quads[1].positionC_w = { 0.0f, 0.0f, 559.2f, 0.0f };
                    Quads[1].positionD_w = { 549.6f, 0.0f, 559.2f, 0.0f };

                    // Ceiling
                    Quads[2].positionA_Albedo = { 556.0f, 548.8f,   0.0f, 0.78f };
                    Quads[2].positionB_Emissive = { 556.0f, 548.8f, 559.2f, 1.0f };
                    Quads[2].positionC_w = { 0.0f, 548.8f, 559.2f, 0.0f };
                    Quads[2].positionD_w = { 0.0f, 548.8f,   0.0f, 0.0f };

                    // Back Wall
                    Quads[3].positionA_Albedo = { 549.6f,   0.0f, 559.2f, 1.0f };
                    Quads[3].positionB_Emissive = { 0.0f,   0.0f, 559.2f, 0.0f };
                    Quads[3].positionC_w = { 0.0f, 548.8f, 559.2f, 0.0f };
                    Quads[3].positionD_w = { 556.0f, 548.8f, 559.2f, 0.0f };

                    // Left Wall
                    Quads[4].positionA_Albedo = { 0.0f,   0.0f, 559.2f, 1.0f };
                    Quads[4].positionB_Emissive = { 0.0f,   0.0f, 0.0f, 0.0f };
                    Quads[4].positionC_w = { 0.0f, 548.8f, 0.0f, 0.0f };
                    Quads[4].positionD_w = { 0.0f, 548.8f, 559.2f, 0.0f };

                    // Right Wall
                    Quads[0].positionA_Albedo = { 552.8f, 0.0f,   0.0f, 1.0f };
                    Quads[0].positionB_Emissive = { 549.6f,   0.0f, 559.2f, 0.0f };
                    Quads[0].positionC_w = { 556.0f, 548.8f, 559.2f, 0.0f };
                    Quads[0].positionD_w = { 556.0f, 548.8f,   0.0f, 0.0f };

                    CalculateQuadNormal(Quads[0]);
                    CalculateQuadNormal(Quads[1]);
                    CalculateQuadNormal(Quads[2]);
                    CalculateQuadNormal(Quads[3]);
                    CalculateQuadNormal(Quads[4]);
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
                    spheres[0].position_Radius = { 0.0f, 0.0f, 4.0f, 2.0f };
                    spheres[0].albedo_Emissive_zw = { 1.0f, 0.0f, 0.0f, 0.0f };

                    spheres[1].position_Radius = { 0.0f, 0.0f, 0.0f, 20.0f };
                    spheres[1].albedo_Emissive_zw = { 0.0f, 0.5f, 0.0f, 0.0f };
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