#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

// TODO: should we pluralize this? only thing we really need is full screen passes...

bool ModelInit (ID3D11Device* device);
void ModelShutdown ();
void ModelRender (ID3D11DeviceContext* deviceContext);