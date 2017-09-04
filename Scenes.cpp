#include "Scenes.h"
#include "ShaderTypes.h"
#include "tiny_obj_loader.h"
#include <d3d11.h>

bool FillSceneData (EScene scene, ID3D11DeviceContext* context)
{
    bool ret = true;

    // reset the sample count back to 0
    ret &= ShaderData::ConstantBuffers::ConstantsPerFrame.Write(
        context,
        [](ShaderTypes::ConstantBuffers::ConstantsPerFrame& data)
        {
            data.sampleCount_yzw[0] = 0;
        }
    );

    switch (scene)
    {
        case EScene::SphereOnPlane_LowLight:
        {
            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.0f, 0.0f, 0.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 2, 0, 0, 2 };
                }
            );

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 4.0f, 4.0f, 6.0f }, 0.5f, { 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f });
                    MakeSphere(spheres[1], { 0.0f, 0.0f, 4.0f }, 2.0f, { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f });
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    MakeQuad(Quads[0], { -4.0f, -3.0f, -4.0f }, { -4.0f, 2.0f, -4.0f }, { -4.0f, 2.0f, 12.0f }, { -4.0f, -3.0f, 12.0f }, { 0.1f, 0.9f, 0.1f }, { 0.0f, 0.0f, 0.0f });
                    MakeQuad(Quads[1], { -15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, -15.0f }, { -15.0f, -2.0f, -15.0f }, { 0.9f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f });
                }
            );
            break;
        }
        case EScene::SphereOnPlane_RegularLight:
        {
            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.1f, 0.4f, 1.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 2, 0, 0, 2 };
                }
            );

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 4.0f, 4.0f, 6.0f }, 0.5f, { 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f });
                    MakeSphere(spheres[1], { 0.0f, 0.0f, 4.0f }, 2.0f, { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f });
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    MakeQuad(Quads[0], { -4.0f, -3.0f, -4.0f }, { -4.0f, 2.0f, -4.0f }, { -4.0f, 2.0f, 12.0f }, { -4.0f, -3.0f, 12.0f }, { 0.1f, 0.9f, 0.1f }, { 0.0f, 0.0f, 0.0f });
                    MakeQuad(Quads[1], { -15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, 15.0f }, { 15.0f, -2.0f, -15.0f }, { -15.0f, -2.0f, -15.0f }, { 0.9f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f });
                }
            );
            break;
        }
        case EScene::CornellBox_SmallLight:
        {
            // slightly modified cornell box, from source: http://www.graphics.cornell.edu/online/box/data.html
            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 2.78f;
                    scene.cameraPos_FOVX[1] = 2.73f;
                    scene.cameraPos_FOVX[2] = -5.0f;

                    scene.cameraAt_FOVY[0] = 2.78f;
                    scene.cameraAt_FOVY[1] = 2.73f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.0f, 0.0f, 0.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 0, 0, 2, 6 };
                }
            );

            ret &= ShaderData::StructuredBuffers::OBBs.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::OBBPrim, 10>& obbs)
                {
                    MakeOBB(obbs[0], { 1.855f, 0.825f, 1.69f }, { 0.825f, 0.825f, 0.825f }, { 0.0f, 1.0f, 0.0f }, DegreesToRadians(-17.0f), { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
                    MakeOBB(obbs[1], { 3.685f, 1.65f, 3.5125f }, { 0.825f, 1.65f, 0.825f }, { 0.0f, 1.0f, 0.0f }, DegreesToRadians(107.0f), { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    // Light
                    MakeQuad(Quads[0], { 3.43f, 5.486f, 2.27f }, { 3.43f, 5.486f, 3.32f }, { 2.13f, 5.486f, 3.32f }, { 2.13f, 5.486f, 2.27f }, { 0.78f, 0.78f, 0.78f }, { 25.0f, 25.0f, 25.0f });

                    // Floor
                    MakeQuad(Quads[1], { 5.528f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 5.592f }, { 5.496f, 0.0f, 5.592f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

                    // Ceiling
                    MakeQuad(Quads[2], { 5.560f, 5.488f,   0.0f }, { 5.56f, 5.488f, 5.592f }, { 0.0f, 5.488f, 5.592f }, { 0.0f, 5.488f,   0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

                    // Back Wall
                    MakeQuad(Quads[3], { 5.496f,   0.0f, 5.592f }, { 0.0f,   0.0f, 5.592f }, { 0.0f, 5.488f, 5.592f }, { 5.56f, 5.488f, 5.592f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

                    // Left Wall
                    MakeQuad(Quads[4], { 0.0f,   0.0f, 5.592f }, { 0.0f,   0.0f, 0.0f }, { 0.0f, 5.488f, 0.0f }, { 0.0f, 5.488f, 5.592f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });

                    // Right Wall
                    MakeQuad(Quads[5], { 5.528f, 0.0f,   0.0f }, { 5.496f,   0.0f, 5.592f }, { 5.560f, 5.488f, 5.592f }, { 5.56f, 5.488f,   0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
                }
            );

            break;
        }
        case EScene::CornellBox_BigLight:
        {
            // slightly modified cornell box, from source: http://www.graphics.cornell.edu/online/box/data.html
            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 2.78f;
                    scene.cameraPos_FOVX[1] = 2.73f;
                    scene.cameraPos_FOVX[2] = -5.0f;

                    scene.cameraAt_FOVY[0] = 2.78f;
                    scene.cameraAt_FOVY[1] = 2.73f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.0f, 0.0f, 0.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 0, 0, 2, 5 };
                }
            );

            ret &= ShaderData::StructuredBuffers::OBBs.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::OBBPrim, 10>& obbs)
                {
                    MakeOBB(obbs[0], { 1.855f, 0.825f, 1.69f }, { 0.825f, 0.825f, 0.825f }, { 0.0f, 1.0f, 0.0f }, DegreesToRadians(-17.0f), { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
                    MakeOBB(obbs[1], { 3.685f, 1.65f, 3.5125f }, { 0.825f, 1.65f, 0.825f }, { 0.0f, 1.0f, 0.0f }, DegreesToRadians(107.0f), { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
                }
            );

            ret &= ShaderData::StructuredBuffers::Quads.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::QuadPrim, 10>& Quads)
                {
                    // Floor
                    MakeQuad(Quads[1], { 5.528f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 5.592f }, { 5.496f, 0.0f, 5.592f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

                    // Ceiling
                    MakeQuad(Quads[2], { 5.560f, 5.488f,   0.0f }, { 5.56f, 5.488f, 5.592f }, { 0.0f, 5.488f, 5.592f }, { 0.0f, 5.488f,   0.0f }, { 0.78f, 0.78f, 0.78f }, { 1.0f, 1.0f, 1.0f });

                    // Back Wall
                    MakeQuad(Quads[3], { 5.496f,   0.0f, 5.592f }, { 0.0f,   0.0f, 5.592f }, { 0.0f, 5.488f, 5.592f }, { 5.56f, 5.488f, 5.592f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

                    // Left Wall
                    MakeQuad(Quads[4], { 0.0f,   0.0f, 5.592f }, { 0.0f,   0.0f, 0.0f }, { 0.0f, 5.488f, 0.0f }, { 0.0f, 5.488f, 5.592f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });

                    // Right Wall
                    MakeQuad(Quads[0], { 5.528f, 0.0f,   0.0f }, { 5.496f,   0.0f, 5.592f }, { 5.560f, 5.488f, 5.592f }, { 5.56f, 5.488f,   0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
                }
            );

            break;
        }
        case EScene::FurnaceTest:
        {
            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -10.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.0f, 0.0f, 0.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 2, 0, 0, 0 };
                }
            );

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 0.0f, 0.0f, 4.0f }, 2.0f, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
                    MakeSphere(spheres[1], { 0.0f, 0.0f, 0.0f }, 20.0f, { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f });
                }
            );
            break;
        }
        case EScene::CornellObj:
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;

            std::string err;
            if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "Art/Models/cornell_box.obj", "./Art/Models/", true))
                return false;

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { 450.0f, 250.0f, 200.0f }, 20.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 75.0f });
                }
            );

            size_t triangleIndex = 0;
            ret &= ShaderData::StructuredBuffers::Triangles.Write(
                context,
                [&] (std::array<ShaderTypes::StructuredBuffers::TrianglePrim, 1000>& triangles)
                {
                    for (const tinyobj::shape_t& shape : shapes)
                    {
                        for (size_t srcIndex = 0; srcIndex < shape.mesh.indices.size(); srcIndex += 3)
                        {
                            float3 a, b, c;
                            float3 albedo, emissive;
                            
                            int indexA = shape.mesh.indices[srcIndex + 0].vertex_index;
                            int indexB = shape.mesh.indices[srcIndex + 1].vertex_index;
                            int indexC = shape.mesh.indices[srcIndex + 2].vertex_index;

                            a[0] = attrib.vertices[indexA * 3 + 0];
                            a[1] = attrib.vertices[indexA * 3 + 1];
                            a[2] = attrib.vertices[indexA * 3 + 2];

                            b[0] = attrib.vertices[indexB * 3 + 0];
                            b[1] = attrib.vertices[indexB * 3 + 1];
                            b[2] = attrib.vertices[indexB * 3 + 2];

                            c[0] = attrib.vertices[indexC * 3 + 0];
                            c[1] = attrib.vertices[indexC * 3 + 1];
                            c[2] = attrib.vertices[indexC * 3 + 2];

                            int materialID = shape.mesh.material_ids[srcIndex / 3];
                            if (materialID >= 0)
                            {
                                albedo[0] = materials[materialID].diffuse[0];
                                albedo[1] = materials[materialID].diffuse[1];
                                albedo[2] = materials[materialID].diffuse[2];

                                emissive[0] = materials[materialID].ambient[0];
                                emissive[1] = materials[materialID].ambient[1];
                                emissive[2] = materials[materialID].ambient[2];
                            }
                            else
                            {
                                albedo = { 1.0f, 1.0f, 1.0f };
                                emissive = { 0.0f, 0.0f, 0.0f };
                            }

                            MakeTriangle(triangles[triangleIndex], a, b, c, albedo, emissive);
                            ++triangleIndex;

                            // TODO: print an error if you bust the array size?
                            if (triangleIndex >= triangles.size())
                            {
                                return;
                            }
                        }
                    }
                }
            );

            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [&](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 278.0f;
                    scene.cameraPos_FOVX[1] = 273.0f;
                    scene.cameraPos_FOVX[2] = -500.0f;

                    scene.cameraAt_FOVY[0] = 278.0f;
                    scene.cameraAt_FOVY[1] = 273.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.0f, 0.0f, 0.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 1, (unsigned int)triangleIndex, 0, 0 };
                }
            );

            break;
        }
        case EScene::ObjTest:
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;

            std::string err;
            if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "Art/Models/barel0-2.obj", "./Art/Models/", true))
                return false;

            ret &= ShaderData::StructuredBuffers::Spheres.Write(
                context,
                [] (std::array<ShaderTypes::StructuredBuffers::SpherePrim, 10>& spheres)
                {
                    MakeSphere(spheres[0], { -2.0f, 0.0f, -3.0f }, 0.5f, { 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f });
                }
            );

            size_t triangleIndex = 0;
            ret &= ShaderData::StructuredBuffers::Triangles.Write(
                context,
                [&] (std::array<ShaderTypes::StructuredBuffers::TrianglePrim, 1000>& triangles)
                {
                    for (const tinyobj::shape_t& shape : shapes)
                    {
                        for (size_t srcIndex = 0; srcIndex < shape.mesh.indices.size(); srcIndex += 3)
                        {
                            float3 a, b, c;
                            float3 albedo, emissive;
                            
                            int indexA = shape.mesh.indices[srcIndex + 0].vertex_index;
                            int indexB = shape.mesh.indices[srcIndex + 1].vertex_index;
                            int indexC = shape.mesh.indices[srcIndex + 2].vertex_index;

                            a[0] = attrib.vertices[indexA * 3 + 0];
                            a[1] = attrib.vertices[indexA * 3 + 1];
                            a[2] = attrib.vertices[indexA * 3 + 2];

                            b[0] = attrib.vertices[indexB * 3 + 0];
                            b[1] = attrib.vertices[indexB * 3 + 1];
                            b[2] = attrib.vertices[indexB * 3 + 2];

                            c[0] = attrib.vertices[indexC * 3 + 0];
                            c[1] = attrib.vertices[indexC * 3 + 1];
                            c[2] = attrib.vertices[indexC * 3 + 2];

                            int materialID = shape.mesh.material_ids[srcIndex / 3];
                            if (materialID >= 0)
                            {
                                albedo[0] = materials[materialID].diffuse[0];
                                albedo[1] = materials[materialID].diffuse[1];
                                albedo[2] = materials[materialID].diffuse[2];

                                emissive[0] = materials[materialID].ambient[0];
                                emissive[1] = materials[materialID].ambient[1];
                                emissive[2] = materials[materialID].ambient[2];
                            }
                            else
                            {
                                albedo = { 1.0f, 1.0f, 1.0f };
                                emissive = { 0.0f, 0.0f, 0.0f };
                            }

                            MakeTriangle(triangles[triangleIndex], a, b, c, albedo, emissive);
                            ++triangleIndex;

                            // TODO: print an error if you bust the array size?
                            if (triangleIndex >= triangles.size())
                            {
                                return;
                            }
                        }
                    }
                }
            );

            ret &= ShaderData::ConstantBuffers::ConstantsOnce.Write(
                context,
                [&](ShaderTypes::ConstantBuffers::ConstantsOnce& scene)
                {
                    scene.cameraPos_FOVX[0] = 0.0f;
                    scene.cameraPos_FOVX[1] = 0.0f;
                    scene.cameraPos_FOVX[2] = -5.0f;

                    scene.cameraAt_FOVY[0] = 0.0f;
                    scene.cameraAt_FOVY[1] = 0.0f;
                    scene.cameraAt_FOVY[2] = 0.0f;

                    scene.nearPlaneDist_missColor = { 0.1f, 0.0f, 0.0f, 0.0f };

                    scene.numSpheres_numTris_numOBBs_numQuads = { 1, (unsigned int)triangleIndex, 0, 0 };
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