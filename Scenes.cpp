#include "Scenes.h"
#include "ShaderTypes.h"
#include <d3d11.h>

bool FillSceneData (EScene scene, ID3D11DeviceContext* context)
{
    bool ret = true;

    switch (scene)
    {
        case EScene::SphereOnPlane_LowLight:
        {
            ret &= ShaderData::ConstantBuffers::Scene.Write(
                context,
                [](ShaderTypes::ConstantBuffers::Scene& scene)
                {
                    scene.numSpheres_numTris_nearPlaneDist_missColor[0] = 2.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 4.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.0f;

                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.frameRnd_appTime_sampleCount_w[2] = 0.0f; // reset the sample count back to 0!
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

            ret &= ShaderData::StructuredBuffers::Triangles.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::TrianglePrim, 10>& Triangles)
                {
                    // wall
                    Triangles[0].positionA_Albedo = { -4.0f, -3.0f, -4.0f, 0.9f };
                    Triangles[0].positionB_Emissive = { -4.0f, 2.0f, -4.0f, 0.0f };
                    Triangles[0].positionC_w = { -4.0f, 2.0f, 12.0f, 0.0f };

                    Triangles[1].positionA_Albedo = { -4.0f, -3.0f, -4.0f, 0.9f };
                    Triangles[1].positionB_Emissive = { -4.0f, 2.0f, 12.0f, 0.0f };
                    Triangles[1].positionC_w = { -4.0f, -3.0f, 12.0f, 0.0f };

                    // floor
                    Triangles[2].positionA_Albedo = { -15.0f, -2.0f, 15.0f, 0.9f };
                    Triangles[2].positionB_Emissive = { 15.0f, -2.0f, 15.0f, 0.0f };
                    Triangles[2].positionC_w = { 15.0f, -2.0f, -15.0f, 0.0f };

                    Triangles[3].positionA_Albedo = { -15.0f, -2.0f, 15.0f, 0.9f };
                    Triangles[3].positionB_Emissive = { 15.0f, -2.0f, -15.0f, 0.0f };
                    Triangles[3].positionC_w = { -15.0f, -2.0f, -15.0f, 0.0f };

                    CalculateTriangleNormal(Triangles[0]);
                    CalculateTriangleNormal(Triangles[1]);
                    CalculateTriangleNormal(Triangles[2]);
                    CalculateTriangleNormal(Triangles[3]);
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
                    scene.numSpheres_numTris_nearPlaneDist_missColor[1] = 4.0f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[2] = 0.1f;
                    scene.numSpheres_numTris_nearPlaneDist_missColor[3] = 0.4f;

                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.frameRnd_appTime_sampleCount_w[2] = 0.0f; // reset the sample count back to 0!
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

            ret &= ShaderData::StructuredBuffers::Triangles.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::TrianglePrim, 10>& Triangles)
                {
                    // wall
                    Triangles[0].positionA_Albedo = { -4.0f, -3.0f, -4.0f, 0.9f };
                    Triangles[0].positionB_Emissive = { -4.0f, 2.0f, -4.0f, 0.0f };
                    Triangles[0].positionC_w = { -4.0f, 2.0f, 12.0f, 0.0f };

                    Triangles[1].positionA_Albedo = { -4.0f, -3.0f, -4.0f, 0.9f };
                    Triangles[1].positionB_Emissive = { -4.0f, 2.0f, 12.0f, 0.0f };
                    Triangles[1].positionC_w = { -4.0f, -3.0f, 12.0f, 0.0f };

                    // floor
                    Triangles[2].positionA_Albedo = { -15.0f, -2.0f, 15.0f, 0.9f };
                    Triangles[2].positionB_Emissive = { 15.0f, -2.0f, 15.0f, 0.0f };
                    Triangles[2].positionC_w = { 15.0f, -2.0f, -15.0f, 0.0f };

                    Triangles[3].positionA_Albedo = { -15.0f, -2.0f, 15.0f, 0.9f };
                    Triangles[3].positionB_Emissive = { 15.0f, -2.0f, -15.0f, 0.0f };
                    Triangles[3].positionC_w = { -15.0f, -2.0f, -15.0f, 0.0f };

                    CalculateTriangleNormal(Triangles[0]);
                    CalculateTriangleNormal(Triangles[1]);
                    CalculateTriangleNormal(Triangles[2]);
                    CalculateTriangleNormal(Triangles[3]);
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