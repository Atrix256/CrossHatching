#pragma once

#include <d3d11.h>

bool ShaderInit (ID3D11Device* device, HWND hWnd, wchar_t* fileName);

void ShaderShutdown (void);

bool ShaderSetConstants (ID3D11DeviceContext* deviceContext, float r, float g, float b, float a, ID3D11ShaderResourceView* texture);

void ShaderDraw (ID3D11DeviceContext* deviceContext, int indexCount);