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
    if (bpp != 32)
    {
        return false;
    }

    // Calculate the size of the 32 bit image data.
    imageSize = width * height * 4;

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
    targaData.resize(imageSize);

    // Initialize the index into the targa destination data array.
    index = 0;

    // Initialize the index into the targa image data.
    k = (width * height * 4) - (width * 4);

    // Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            targaData[index + 0] = targaImage[k + 2];  // Red.
            targaData[index + 1] = targaImage[k + 1];  // Green.
            targaData[index + 2] = targaImage[k + 0];  // Blue
            targaData[index + 3] = targaImage[k + 3];  // Alpha

                                                         // Increment the indexes into the targa data.
            k += 4;
            index += 4;
        }

        // Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
        k -= (width * 8);
    }

    return true;
}

bool CTexture::LoadTGA (ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;
    int height, width;
    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT hResult;
    unsigned int rowPitch;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    std::vector<unsigned char> targaData;

    // Load the targa image data into memory.
    result = LoadTarga(filename, height, width, targaData);
    if (!result)
    {
        return false;
    }

    // Setup the description of the texture.
    textureDesc.Height = height;
    textureDesc.Width = width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    // Set the row pitch of the targa image data.
    rowPitch = (width * 4) * sizeof(unsigned char);

    // Copy the targa image data into the texture.
    deviceContext->UpdateSubresource(m_texture.m_ptr, 0, NULL, &targaData[0], rowPitch, 0);

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture.m_ptr, &srvDesc, &m_textureView.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    // Generate mipmaps for this texture.
    deviceContext->GenerateMips(m_textureView.m_ptr);

    return true;
}

bool CTexture::CreateRenderTarget (ID3D11Device* device, ID3D11DeviceContext* deviceContext, size_t width, size_t height)
{
    // relevant article on rendering to textures: http://www.rastertek.com/dx11tut22.html
    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    HRESULT hResult;

    // Setup the description of the texture.
    textureDesc.Height = (UINT)height;
    textureDesc.Width = (UINT)width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    // set up the render target view description
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    // Create the render target view.
    hResult = device->CreateRenderTargetView(m_texture.m_ptr, &rtvDesc, &m_renderTargetView.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    // Setup the shader resource view description.
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture.m_ptr, &srvDesc, &m_textureView.m_ptr);
    if (FAILED(hResult))
    {
        return false;
    }

    return true;
}

void CTexture::SetAsRenderTarget (ID3D11DeviceContext* deviceContext)
{
    deviceContext->OMSetRenderTargets(1, &m_renderTargetView.m_ptr, nullptr);
}