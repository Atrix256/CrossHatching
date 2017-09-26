#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <stdio.h>
#include <stdint.h>
#include <vector>

typedef uint8_t uint8;

//======================================================================================
struct SImageData
{
    SImageData ()
        : m_width(0)
        , m_height(0)
    { }
   
    size_t m_width;
    size_t m_height;
    size_t m_pitch;
    std::vector<uint8> m_pixels;
};
 
struct SColor
{
    SColor (uint8 _R = 0, uint8 _G = 0, uint8 _B = 0)
        : R(_R), G(_G), B(_B)
    { }
 
    uint8 B, G, R;
};

//======================================================================================
bool ImageSave (const SImageData &image, const char *fileName)
{
    // open the file if we can
    FILE *file;
    file = fopen(fileName, "wb");
    if (!file) {
        printf("Could not save %s\n", fileName);
        return false;
    }
   
    // make the header info
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoHeader;
   
    header.bfType = 0x4D42;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits = 54;
   
    infoHeader.biSize = 40;
    infoHeader.biWidth = (LONG)image.m_width;
    infoHeader.biHeight = (LONG)image.m_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = (DWORD) image.m_pixels.size();
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;
   
    header.bfSize = infoHeader.biSizeImage + header.bfOffBits;
   
    // write the data and close the file
    fwrite(&header, sizeof(header), 1, file);
    fwrite(&infoHeader, sizeof(infoHeader), 1, file);
    fwrite(&image.m_pixels[0], infoHeader.biSizeImage, 1, file);
    fclose(file);
  
    return true;
}
 
//======================================================================================
void ImageInit (SImageData& image, size_t width, size_t height)
{
    image.m_width = width;
    image.m_height = height;
    image.m_pitch = 4 * ((width * 24 + 31) / 32);
    image.m_pixels.resize(image.m_pitch * image.m_width);
    std::fill(image.m_pixels.begin(), image.m_pixels.end(), 0);
}
 
//======================================================================================
void ImageClear (SImageData& image, const SColor& color)
{
    uint8* row = &image.m_pixels[0];
    for (size_t rowIndex = 0; rowIndex < image.m_height; ++rowIndex)
    {
        SColor* pixels = (SColor*)row;
        std::fill(pixels, pixels + image.m_width, color);
 
        row += image.m_pitch;
    }
}
 
//======================================================================================
void ImageBox (SImageData& image, size_t x1, size_t x2, size_t y1, size_t y2, const SColor& color)
{
    for (size_t y = y1; y < y2; ++y)
    {
        uint8* row = &image.m_pixels[y * image.m_pitch];
        SColor* start = &((SColor*)row)[x1];
        std::fill(start, start + x2 - x1, color);
    }
}

//======================================================================================
struct TAMBlueNoisePixels
{
public:
    inline static void InitializeImage (SImageData& imageData)
    {
        ImageClear(imageData, SColor(255, 255, 255));
    }
};

//======================================================================================
template <typename TAMGENERATOR>
void GenerateTAM (const char* baseFileName, size_t dimensions, size_t numMips)
{
    // initialize the column of mips
    std::vector<SImageData> mipColumn;
    mipColumn.resize(numMips);
    size_t mipDims = dimensions;
    for (size_t i = 0; i < numMips; ++i)
    {
        ImageInit(mipColumn[i], mipDims, mipDims);
        mipDims /= 2;
    }

    // Initialize the mip columns to their starting state
    for (SImageData& imageData : mipColumn)
        TAMGENERATOR::InitializeImage(imageData);

    // TODO: the work here! for each mip column, do the stuff! a lambda passed in for a stroke and maybe the fitness function too. Likely also the number of "best candidates" to review at each step

    // save this mip column
    char fileName[1024];
    for (size_t i = 0; i < numMips; ++i)
    {
        sprintf(fileName, baseFileName, 0, i);
        ImageSave(mipColumn[i], fileName);
    }
}

//======================================================================================
int main (int argc, char** argv)
{
    GenerateTAM<TAMBlueNoisePixels>("TAMs/Dots/Dots_%zu_%zu.bmp", 256, 4);

    return 0;
}

/*
TAM GENERATOR TODO:

1) make a column of mips, initialize to white.
2) find a candidate stroke / dot / etc to add to all the mips
3) use the best candidate and apply to all mips
4) repeat until we have desired average brightness
5) move to next column up mips, using this as a starting point.
6) rinse and repeat until all columns are filled

* do a few types of tams:
 * blue noise pixels 
 * blue noise circles
 * horiz then vert then mixed hatching
 * horiz and vert only?
 * the bunny one that had different directional strokes that made it look furry?
 * random direction lines?
 * circles of random scales?
 * random sized quads of random colors
 * white on black
 * colors?

? can we multithread this? I bet we can for the "best candidate" generation / selection
* make it print out progress if it takes long enough.

? what should we do about mips not going to 1 px? maybe manually make those mips? maybe don't have the CrossHatching program use them? (limit # of mips). dunno

* get these loaded / working in the cross hatching project

*/