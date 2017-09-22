#pragma once

struct ID3D11DeviceContext;

enum class EScene
{
    SphereOnPlane_LowLight,
    SphereOnPlane_RegularLight,
    CornellBox_SmallLight,
    CornellBox_BigLight,
    FurnaceTest,
    CornellObj,
    ObjTest,
    Spheres,
    COUNT
};

bool FillSceneData (EScene scene, ID3D11DeviceContext* context);