#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

bool ModelInit (ID3D11Device* device);
void ModelShutdown ();
void ModelRender (ID3D11DeviceContext* deviceContext);