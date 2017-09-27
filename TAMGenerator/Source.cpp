#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <random>

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

    inline void Set (uint8 _R, uint8 _G, uint8 _B)
    {
        R = _R;
        G = _G;
        B = _B;
    }
 
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
inline float Lerp (float A, float B, float t)
{
    return A * (1 - t) + B * t;
}

//======================================================================================
inline float ColorBrightness (const SColor& color)
{
    return (float(color.R) * 0.3f + float(color.G * 0.59f) + float(color.B * 0.11f)) / 255.0f;
}

//======================================================================================
float ImageAverageBrightness (const SImageData& image)
{
    size_t sampleCount = 0;
    float averageBrightness = 0.0f;

    const uint8* row = &image.m_pixels[0];
    for (size_t rowIndex = 0; rowIndex < image.m_height; ++rowIndex)
    {
        SColor* pixels = (SColor*)row;

        for (size_t pixelIndex = 0; pixelIndex < image.m_width; ++pixelIndex)
        {
            float brightness = ColorBrightness(pixels[pixelIndex]);
            sampleCount++;
            averageBrightness = Lerp(averageBrightness, brightness, 1.0f / float(sampleCount));
        }

        row += image.m_pitch;
    }

    return averageBrightness;
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
class TAMStroke_Pixel
{
public:
    inline float Distance (const TAMStroke_Pixel& other, int width)
    {
        // this returns the toroidal distance between the points
        // aka the interval [0, width) wraps around
        float dx = std::abs(other.m_posX - m_posX);
        float dy = std::abs(other.m_posY - m_posY);

        if (dx > float(width / 2))
            dx = float(width) - dx;

        if (dy > float(width / 2))
            dy = float(width) - dy;

        // returning squared distance cause why not
        return dx*dx + dy*dy;
    }

    void Randomize (size_t imageSize, float targetBrightness)
    {
        // seed the random number generator
        static std::random_device rd;
        static std::mt19937 rng(rd());

        std::uniform_real_distribution<float> dist(0, float(imageSize));

        m_posX = dist(rng);
        m_posY = dist(rng);
    }

    void DrawStroke (SImageData& image)
    {
        // TODO: is floor ok? or should we add half before flooring?
        uint8* pixel = &image.m_pixels[size_t(m_posY) * image.m_pitch + size_t(m_posX) * 3];
        ((SColor*)pixel)->Set(0, 0, 0);
    }

    float m_posX;
    float m_posY;
};

//======================================================================================
class TAMStroke_Circle
{
public:
    inline float Distance (const TAMStroke_Circle& other, int width)
    {
        // this returns the toroidal distance between the points
        // aka the interval [0, width) wraps around
        float dx = std::abs(other.m_posX - m_posX);
        float dy = std::abs(other.m_posY - m_posY);

        if (dx > float(width / 2))
            dx = float(width) - dx;

        if (dy > float(width / 2))
            dy = float(width) - dy;

        // returning squared distance cause why not
        return dx*dx + dy*dy;
    }

    void Randomize (size_t imageSize, float targetBrightness)
    {
        // seed the random number generator
        static std::random_device rd;
        static std::mt19937 rng(rd());

        std::uniform_real_distribution<float> dist(0, float(imageSize));

        m_posX = dist(rng);
        m_posY = dist(rng);
    }

    void DrawStroke (SImageData& image)
    {
        // TODO: is floor ok? or should we add half before flooring?
        uint8* pixel = &image.m_pixels[size_t(m_posY) * image.m_pitch + size_t(m_posX) * 3];
        ((SColor*)pixel)->Set(0, 0, 0);
    }

    float m_posX;
    float m_posY;
    float m_radius;
};

//======================================================================================
// TODO: this may be overkill to have this be it's own thing. Everything may follow this code.
template <typename TAMSTROKE, size_t NUMCANDIDATES>
class TAMGenerator_BlueNoise
{
public:
    inline float InitializeImage (SImageData& imageData)
    {
        // clear to white and return the brightness of the fill
        ImageClear(imageData, SColor(255, 255, 255));
        return ColorBrightness(SColor(255, 255, 255));
    }

    void GenerateStroke (SImageData& image, float targetBrightness)
    {
        TAMSTROKE bestCandidate;
        TAMSTROKE currentCandidate;
        float bestScore = 0.0f;
        for (size_t i = 0; i < NUMCANDIDATES; ++i)
        {
            // generate a candidate
            currentCandidate.Randomize(image.m_width, targetBrightness);

            // TODO: use a grid for finding closest point. should speed things up a lot as things get denser, which should allow for higher candidate counts

            // score this candidate by finding the shortest distance from it to all other strokes
            float score = FLT_MAX;
            for (TAMSTROKE& stroke : m_strokes)
            {
                float dist = stroke.Distance(currentCandidate, (int)image.m_width);
                score = min(score, dist);
            }

            // if this score is higher than our previous best, take it as our best
            if (score > bestScore)
            {
                bestScore = score;
                bestCandidate = currentCandidate;
            }
        }

        // commit the stroke to the stroke list
        m_strokes.push_back(bestCandidate);

        // commit the stroke to the texture
        m_strokes.rbegin()->DrawStroke(image);
    }

    float GenerateStrokes (SImageData& image, float currentBrightness, float targetBrightness)
    {
        // TODO: this works for pixels but not for brush strokes and other things
        size_t desiredStrokes = size_t(float(image.m_width * image.m_width) * (currentBrightness - targetBrightness));
        size_t tick = desiredStrokes / 100;

        for (size_t i = 0; i < desiredStrokes; ++i)
        {
            GenerateStroke(image, targetBrightness);

            if (i % tick == 0)
            {
                printf("               \r%i%%  (%zu / %zu)\r", int(100.0f*float(i) / float(desiredStrokes)), i, desiredStrokes);
            }
        }

        // return the actual brightness level
        return ImageAverageBrightness(image);
    }

    std::vector<TAMSTROKE> m_strokes;
};

//======================================================================================
template <typename TAMGENERATOR>
void GenerateTAM (const char* baseFileName, size_t dimensions, size_t numShades)
{
    TAMGENERATOR generator;
    char fileName[1024];

    // initialize an image to it's starting state
    SImageData image;
    ImageInit(image, dimensions, dimensions);
    float brightness = generator.InitializeImage(image);

    // generate each shade, but don't include pure black or pure white as those are easy to generate
    for (size_t i = 0; i < numShades; ++i)
    {
        float desiredBrightness = 1.0f - float(i + 1) / float(numShades + 1);
        brightness = generator.GenerateStrokes(image, brightness, desiredBrightness);
        sprintf(fileName, baseFileName, i);
        ImageSave(image, fileName);
        printf("\n%s brightness = %f\n\n", fileName, brightness);
    }
}

//======================================================================================
int main (int argc, char** argv)
{
    // TODO: this blue noise kinda sucks for tiling, and also sucks when it's denser.  Increasing candidate count didn't really help much.
    GenerateTAM<TAMGenerator_BlueNoise<TAMStroke_Pixel, 1000>>("TAMs/Dots/Dots_%zu.bmp", 32, 8);

    //GenerateTAM<TAMGenerator_BlueNoise<TAMStroke_Circle, 1000>>("TAMs/Dots/Circles_%zu.bmp", 32, 8);

    return 0;
}

/*
TAM GENERATOR TODO:

! do the TAM thing, but don't auto-generate mips

? figure out good cost function, like length or what?

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

* maybe have different image generations run on different threads. could have an option for numThreads, or just use however many cores there are.

* check this out with sleepy to see what is slow

? can we multithread this? I bet we can for the "best candidate" generation / selection
* make it print out progress if it takes long enough.

? what should we do about mips not going to 1 px? maybe manually make those mips? maybe don't have the CrossHatching program use them? (limit # of mips). dunno

* get these loaded / working in the cross hatching project

? maybe ask stack exchange how to make high density 1 bit blue noise textures?

*/