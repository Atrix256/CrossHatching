#include "Texture.h"

#include <stdio.h>
#include <vector>

struct TargaHeader
{
    unsigned char data1[12];
    unsigned short width;
    unsigned short height;
    unsigned char bpp;
    unsigned char data2;
};

static bool LoadTarga(char* filename, int& height, int& width, std::vector<unsigned char>& targaData)
{
    int error, bpp, imageSize, index, i, j, k;
    FILE* filePtr;
    unsigned int count;
    TargaHeader targaFileHeader;
    std::vector<unsigned char> targaImage;

    // Open the targa file for reading in binary.
    error = fopen_s(&filePtr, filename, "rb");
    if (error != 0)
    {
        return false;
    }

    // Read in the file header.
    count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
    if (count != 1)
    {
        return false;
    }

    // Get the important information from the header.
    height = (int)targaFileHeader.height;
    width = (int)targaFileHeader.width;
    bpp = (int)targaFileHeader.bpp;

    // Check that it is 32 bit and not 24 bit.
    if (bpp != 24 && bpp != 32)
    {
        return false;
    }

    int bytesPerPixel = bpp / 8;

    // Calculate the size of the 32 bit image data.
    imageSize = width * height * bytesPerPixel;

    // Allocate memory for the targa image data.
    targaImage.resize(imageSize);

    // Read in the targa image data.
    count = (unsigned int)fread(&targaImage[0], 1, imageSize, filePtr);
    if (count != imageSize)
    {
        return false;
    }

    // Close the file.
    error = fclose(filePtr);
    if (error != 0)
    {
        return false;
    }

    // Allocate memory for the targa destination data.
    targaData.resize(imageSize / bytesPerPixel * 4);

    // Initialize the index into the targa destination data array.
    index = 0;

    // Initialize the index into the targa image data.
    k = (width * height * bytesPerPixel) - (width * bytesPerPixel);

    // Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
    if (bpp == 32)
    {
        for (j = 0; j < height; j++)
        {
            for (i = 0; i < width; i++)
            {
                targaData[index + 0] = targaImage[k + 2];  // Red.
                targaData[index + 1] = targaImage[k + 1];  // Green.
                targaData[index + 2] = targaImage[k + 0];  // Blue
                targaData[index + 3] = targaImage[k + 3];  // Alpha

                                                             // Increment the indexes into the targa data.
                k += bytesPerPixel;
                index += 4;
            }

            // Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
            k -= (width * 2 * bytesPerPixel);
        }
    }
    else
    {
        for (j = 0; j < height; j++)
        {
            for (i = 0; i < width; i++)
            {
                targaData[index + 0] = targaImage[k + 2];  // Red.
                targaData[index + 1] = targaImage[k + 1];  // Green.
                targaData[index + 2] = targaImage[k + 0];  // Blue
                targaData[index + 3] = 255;  // Alpha

                                                           // Increment the indexes into the targa data.
                k += bytesPerPixel;
                index += 4;
            }

            // Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
            k -= (width * 2 * bytesPerPixel);
        }
    }

    return true;
}

bool CTexture::LoadTGA (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    int height, width;
    std::vector<unsigned char> targaData;

    // Load the targa image data into memory.
    if (!LoadTarga(filename, height, width, targaData))
    {
        return false;
    }

    // load the pixel data into a texture
    return LoadFromPixels(device, deviceContext, &targaData[0], width, height, filename);
}

bool CTexture::LoadFromPixels (ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned char* pixels, int width, int height, const char* debugName)
{
    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT hResult;
    unsigned int rowPitch;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;

    // Setup the description of the texture.
    textureDesc.Height = height;
    textureDesc.Width = width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_texture.SetDebugName(debugName);

    // Set the row pitch of the targa image data.
    rowPitch = (width * 4) * sizeof(unsigned char);

    // Copy the targa image data into the texture.
    deviceContext->UpdateSubresource(m_texture.m_ptr, 0, NULL, pixels, rowPitch, 0);

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture.m_ptr, &srvDesc, &m_textureSRV.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_textureSRV.SetDebugName(debugName);

    // setup the unordered access view description
    uavDesc.Format = textureDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    // create the unordered access view for the texture.
    hResult = device->CreateUnorderedAccessView(m_texture.m_ptr, &uavDesc, &m_textureUAV.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_textureUAV.SetDebugName(debugName);

    // Generate mipmaps for this texture.
    deviceContext->GenerateMips(m_textureSRV.m_ptr);

    return true;
}

bool CTexture::Create (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height, DXGI_FORMAT format, const char* debugName)
{
    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT hResult;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;

    // Setup the description of the texture.
    textureDesc.Height = (UINT)height;
    textureDesc.Width = (UINT)width;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_texture.SetDebugName(debugName);

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture.m_ptr, &srvDesc, &m_textureSRV.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_textureSRV.SetDebugName(debugName);

    // setup the unordered access view description
    uavDesc.Format = textureDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    // create the unordered access view for the texture.
    hResult = device->CreateUnorderedAccessView(m_texture.m_ptr, &uavDesc, &m_textureUAV.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_textureUAV.SetDebugName(debugName);

    return true;
}

bool CTexture::CreateVolume (ID3D11Device* device, ID3D11DeviceContext* deviceContext, CTexture** slices, size_t numSlices, const char* debugName)
{
    D3D11_TEXTURE3D_DESC textureDesc;
    HRESULT hResult;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    D3D11_TEXTURE2D_DESC srcTextureDesc;
    slices[0]->GetTexture2D()->GetDesc(&srcTextureDesc);

    // Setup the description of the texture.
    textureDesc.Height = (UINT)srcTextureDesc.Height;
    textureDesc.Width = (UINT)srcTextureDesc.Width;
    textureDesc.MipLevels = 0;
    textureDesc.Depth = (UINT)numSlices;
    textureDesc.Format = srcTextureDesc.Format;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // Create the empty texture.
    hResult = device->CreateTexture3D(&textureDesc, NULL, &m_texture3D.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_texture3D.SetDebugName(debugName);

    for (size_t i = 0; i < numSlices; ++i)
        deviceContext->CopySubresourceRegion(m_texture3D.m_ptr, 0, 0, 0, (UINT)i, slices[i]->GetTexture2D(), 0, NULL);

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MostDetailedMip = 0;
    srvDesc.Texture3D.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture3D.m_ptr, &srvDesc, &m_textureSRV.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_textureSRV.SetDebugName(debugName);

    // Generate mipmaps for this texture.
    deviceContext->GenerateMips(m_textureSRV.m_ptr);

    return true;
}

bool CTexture::CreateArray (ID3D11Device* device, ID3D11DeviceContext* deviceContext, CTexture** slices, size_t numSlices, const char* debugName)
{
    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT hResult;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    D3D11_TEXTURE2D_DESC srcTextureDesc;
    slices[0]->GetTexture2D()->GetDesc(&srcTextureDesc);

    // Setup the description of the texture.
    textureDesc.Height = (UINT)srcTextureDesc.Height;
    textureDesc.Width = (UINT)srcTextureDesc.Width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = (UINT)numSlices;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_texture.SetDebugName(debugName);

    D3D11_TEXTURE2D_DESC createdDesc;
    m_texture.m_ptr->GetDesc(&createdDesc);

    for (size_t i = 0; i < numSlices; ++i)
        deviceContext->CopySubresourceRegion(m_texture.m_ptr, (UINT)i * createdDesc.MipLevels, 0, 0, 0, slices[i]->GetTexture2D(), 0, NULL);

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = (UINT)numSlices;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture.m_ptr, &srvDesc, &m_textureSRV.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }
    m_textureSRV.SetDebugName(debugName);

    // Generate mipmaps for this texture.
    deviceContext->GenerateMips(m_textureSRV.m_ptr);

    return true;
}