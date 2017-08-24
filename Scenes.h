#pragma once

struct ID3D11DeviceContext;

enum class EScene
{
    SphereOnPlane_LowLight,
    SphereOnPlane_RegularLight,
    SpheresInBox_LowLight,
    SpheresInBox_RegularLight,
    FurnaceTest
};

bool FillSceneData (EScene scene, ID3D11DeviceContext* context);