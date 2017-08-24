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

    // TODO: add the other scenes!  https://github.com/Atrix256/RandomCode/blob/master/PTBlogPost1/Source.cpp

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
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.1f;

                    scene.frameRnd_appTime_sampleCount_numQuads[3] = 2.0f;

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
        default:
        {
            ret = false;
        }
    }

    return ret;
}