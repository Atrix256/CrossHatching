#pragma once

#include <d3d11.h>

// TODO: pluralize and do constructor / destructor stuff!

bool TextureInitialize (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);
void TextureShutdown ();
ID3D11ShaderResourceView* TextureGetTexture ();