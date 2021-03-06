#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <random>
#include <chrono>
#include <complex>

typedef uint8_t uint8;

const float c_pi = 3.14159265359f;

#define DO_DFT() false
#define DO_DFT_PHASE() false

//======================================================================================
void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//======================================================================================
//                                     SBlockTimer
//======================================================================================
// times a block of code
struct SBlockTimer
{
    SBlockTimer(const char* label)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        printf("%0.2f seconds\n", seconds.count());
    }

    std::chrono::high_resolution_clock::time_point m_start;
};

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
 
//======================================================================================
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
struct SImageDataComplex
{
    SImageDataComplex ()
        : m_width(0)
        , m_height(0)
    { }
 
    size_t m_width;
    size_t m_height;
    std::vector<std::complex<float>> m_pixels;
};

//======================================================================================
std::complex<float> DFTPixel (const SImageData &srcImage, int K, int L)
{
    std::complex<float> ret(0.0f, 0.0f);
 
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Get the pixel value (assuming greyscale) and convert it to [0,1] space
            const uint8 *src = &srcImage.m_pixels[(y * srcImage.m_pitch) + x * 3];
            float grey = float(src[0]) / 255.0f;
 
            // Add to the sum of the return value
            float v = float(K * x) / float(srcImage.m_width);
            v += float(L * y) / float(srcImage.m_height);
            ret += std::complex<float>(grey, 0.0f) * std::polar<float>(1.0f, -2.0f * c_pi * v);
        }
    }
 
    return ret;
}
 
//======================================================================================
void DFTImage (const SImageData &srcImage, SImageDataComplex &destImage)
{
    // NOTE: this function assumes srcImage is greyscale, so works on only the red component of srcImage.
    // ImageToGrey() will convert an image to greyscale.
 
    // size the output dft data
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);
  
    // calculate 2d dft (brute force, not using fast fourier transform)
    int lastPercent = -1;
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            destImage.m_pixels[y * destImage.m_width + x] = DFTPixel(srcImage, x, y);
        }
        int percent = int(100.0f * float(x) / float(srcImage.m_height));
        if (lastPercent != percent)
        {
            lastPercent = percent;
            printf("            \rDFT: %i%%", lastPercent);
        }
    }
    printf("\n");
}

//======================================================================================
void GetMagnitudeData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = srcImage.m_width * 3;
    if (destImage.m_pitch & 3)
    {
        destImage.m_pitch &= ~3;
        destImage.m_pitch += 4;
    }
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);
 
    // get floating point magnitude data
    std::vector<float> magArray;
    magArray.resize(srcImage.m_width*srcImage.m_height);
    float maxmag = 0.0f;
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + (int)srcImage.m_width / 2) % (int)srcImage.m_width;
            int l = (y + (int)srcImage.m_height / 2) % (int)srcImage.m_height;
            const std::complex<float> &src = srcImage.m_pixels[l*srcImage.m_width + k];
 
            float mag = std::abs(src);
            if (mag > maxmag)
                maxmag = mag;
 
            magArray[y*srcImage.m_width + x] = mag;
        }
    }
    if (maxmag == 0.0f)
        maxmag = 1.0f;
 
    const float c = 255.0f / log(1.0f+maxmag);
 
    // normalize the magnitude data and send it back in [0, 255]
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            float src = c * log(1.0f + magArray[y*srcImage.m_width + x]);
 
            uint8 magu8 = uint8(src);
 
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = magu8;
            dest[1] = magu8;
            dest[2] = magu8;
        }
    }
}
 
//======================================================================================
void GetPhaseData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = srcImage.m_width * 3;
    if (destImage.m_pitch & 3)
    {
        destImage.m_pitch &= ~3;
        destImage.m_pitch += 4;
    }
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);
 
    // get floating point phase data, and encode it in [0,255]
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + (int)srcImage.m_width / 2) % (int)srcImage.m_width;
            int l = (y + (int)srcImage.m_height / 2) % (int)srcImage.m_height;
            const std::complex<float> &src = srcImage.m_pixels[l*srcImage.m_width + k];
 
            // get phase, and change it from [-pi,+pi] to [0,255]
            float phase = (0.5f + 0.5f * std::atan2(src.real(), src.imag()) / c_pi);
            if (phase < 0.0f)
                phase = 0.0f;
            if (phase > 1.0f)
                phase = 1.0;
            uint8 phase255 = uint8(phase * 255);
 
            // write the phase as grey scale color
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = phase255;
            dest[1] = phase255;
            dest[2] = phase255;
        }
    }
}

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
void ImageCopy (SImageData& dest, const SImageData& src)
{
    ImageInit(dest, src.m_width, src.m_height);
    dest.m_pixels = src.m_pixels;
}

//======================================================================================
void ImageInvert (SImageData& image)
{
    for (uint8& c : image.m_pixels)
        c = 255 - c;
}

//======================================================================================
// circle algorithm from http://www.dailyfreecode.com/code/midpoint-circle-drawing-695.aspx
void ImageCircleSectionDraw (SImageData& image, int x, int y, int xC, int yC, const SColor& color)
{
    // not the most cache friendly, but whatever
    auto putpixel = [&] (int _x, int _y) {
        if (_x < 0)
            _x += int(image.m_width);
        else if (_x >= image.m_width)
            _x -= int(image.m_width);

        if (_y < 0)
            _y += int(image.m_height);
        else if (_y >= image.m_height)
            _y -= int(image.m_height);

        *((SColor*)&image.m_pixels[_y * image.m_pitch + _x * 3]) = color;
    };

    putpixel(xC - x, yC + y);
    putpixel(xC + x, yC + y);

    putpixel(xC - x, yC - y);
    putpixel(xC + x, yC - y);
    
    putpixel(xC - y, yC + x);
    putpixel(xC + y, yC + x);
    
    putpixel(xC - y, yC - x);
    putpixel(xC + y, yC - x);
}

//======================================================================================
void ImageCircleSectionDrawFilled (SImageData& image, int x, int y, int xC, int yC, const SColor& color)
{
    // not the most efficient, but whatever
    auto putpixel = [&] (int _x, int _y) {
        if (_x < 0)
            _x += int(image.m_width);
        else if (_x >= image.m_width)
            _x -= int(image.m_width);

        if (_y < 0)
            _y += int(image.m_height);
        else if (_y >= image.m_height)
            _y -= int(image.m_height);

        *((SColor*)&image.m_pixels[_y * image.m_pitch + _x * 3]) = color;
    };

    for (int i = -x; i <= x; ++i)
    {
        putpixel(xC + i, yC + y);
        putpixel(xC + i, yC - y);
    }

    for (int i = -y; i <= y; ++i)
    {
        putpixel(xC + i, yC + x);
        putpixel(xC + i, yC - x);
    }
}

//======================================================================================
template <bool FILLED>
void ImageCircle (SImageData& image, int Radius,int xC,int yC, const SColor& color)
{
    int P = 1 - Radius;
    int x = 0;
    int y = Radius;
    if (FILLED)
        ImageCircleSectionDrawFilled(image, x, y, xC, yC, color);
    else
        ImageCircleSectionDraw(image, x, y, xC, yC, color);
    while (x<=y)
    {
        x++;
        if (P<0)
        {
            P += 2 * x + 1;
        }
        else
        {
            P += 2 * (x - y) + 1;
            y--;
        }
        if (FILLED)
            ImageCircleSectionDrawFilled(image, x, y, xC, yC, color);
        else
            ImageCircleSectionDraw(image, x, y, xC, yC, color);
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
    inline float Distance (const TAMStroke_Pixel& other, int imageWidth)
    {
        // this returns the toroidal distance between the points
        // aka the interval [0, width) wraps around
        float dx = std::abs(other.m_posX - m_posX);
        float dy = std::abs(other.m_posY - m_posY);

        if (dx > float(imageWidth / 2))
            dx = float(imageWidth) - dx;

        if (dy > float(imageWidth / 2))
            dy = float(imageWidth) - dy;

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
        uint8* pixel = &image.m_pixels[size_t(m_posY) * image.m_pitch + size_t(m_posX) * 3];
        ((SColor*)pixel)->Set(0, 0, 0);
    }

    static size_t DesiredStrokes (SImageData& image, float currentBrightness, float targetBrightness)
    {
        return size_t(float(image.m_width * image.m_width) * (currentBrightness - targetBrightness));
    }

    float m_posX;
    float m_posY;
};

//======================================================================================
class TAMStroke_Circle
{
public:
    inline float Distance (const TAMStroke_Circle& other, int imageWidth)
    {
        // this returns the toroidal distance between the circles
        // aka the interval [0, width) wraps around

        // calculate the squared distance between the circle centerpoints
        float dx = std::abs(other.m_posX - m_posX);
        float dy = std::abs(other.m_posY - m_posY);

        if (dx > float(imageWidth / 2))
            dx = float(imageWidth) - dx;

        if (dy > float(imageWidth / 2))
            dy = float(imageWidth) - dy;

        float distSquared = dx*dx + dy*dy;

        // calculate the square of the sum of the radii.
        float combinedRadiusSquared = m_radius + other.m_radius;
        combinedRadiusSquared *= combinedRadiusSquared;

        // return the distance minus the radius (but both are squared), so a negative distance means the circles overlap
        return distSquared - combinedRadiusSquared;
    }

    void Randomize (size_t imageSize, float targetBrightness)
    {
        // seed the random number generator
        static std::random_device rd;
        static std::mt19937 rng(rd());

        // calculate position
        std::uniform_real_distribution<float> dist(0, float(imageSize));
        m_posX = dist(rng);
        m_posY = dist(rng);

        // calculate radius
        std::uniform_real_distribution<float> distRadiusMultiplier(0.01f, 0.05f);
        m_radius = float(imageSize) * distRadiusMultiplier(rng);
        m_radius = max(m_radius, 1.0f);
    }

    void DrawStroke (SImageData& image)
    {
        ImageCircle<true>(image, int(m_radius), int(m_posX), int(m_posY), SColor(0, 0, 0));
    }

    static size_t DesiredStrokes(SImageData& image, float currentBrightness, float targetBrightness)
    {
        return 0;
    }

    float m_posX;
    float m_posY;
    float m_radius;
};

//======================================================================================
class TAMStroke_HorizontalLineSegment
{
public:
    // TODO: rename distance 2nd parameter to imageWidth in all 3 places
    inline float Distance (const TAMStroke_HorizontalLineSegment& other, int imageWidth)
    {
        // TODO: this! toroidal distance between two horizontal line segments
        // TODO: something like, if they have any overlap on x axis, their distance is their y difference.
        // if they don't have any overlap, find which end points are closer to eachother and use the distance between them?
        return 0.0f;
    }

    void Randomize (size_t imageSize, float targetBrightness)
    {
        // seed the random number generator
        static std::random_device rd;
        static std::mt19937 rng(rd());

        // calculate positions
        std::uniform_real_distribution<float> dist(0, float(imageSize));
        m_X = dist(rng);
        m_Y = dist(rng);
        m_length = dist(rng);
    }

    void DrawStroke (SImageData& image)
    {
        // not the most efficient way to draw a horizontal line but whatever
        for (size_t index = 0; index < m_length; ++index)
        {
            size_t x = (size_t(m_X) + index) % image.m_width;
            size_t y = size_t(m_Y);

            uint8* pixel = &image.m_pixels[y * image.m_pitch + x * 3];
            ((SColor*)pixel)->Set(0, 0, 0);
        }
    }

    static size_t DesiredStrokes(SImageData& image, float currentBrightness, float targetBrightness)
    {
        return 0;
    }

    float m_X;
    float m_Y;
    float m_length;
};

//======================================================================================
// TODO: this may be overkill to have this be it's own thing. Everything may follow this code.
template <typename TAMSTROKE, size_t CANDIDATESPERSTROKE, size_t MAXCANDIDATES>
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
        bool haveBestCandidate = false;
        TAMSTROKE bestCandidate;
        TAMSTROKE currentCandidate;
        float bestScore = 0.0f;
        
        // calculate the number of candidates to generate
        size_t numCandidates = m_strokes.size() * CANDIDATESPERSTROKE + 1;
        if (numCandidates > MAXCANDIDATES)
            numCandidates = MAXCANDIDATES;

        // generate the candidates
        for (size_t i = 0; i < numCandidates; ++i)
        {
            // generate a candidate
            currentCandidate.Randomize(image.m_width, targetBrightness);

            // score this candidate by finding the shortest distance from it to all other strokes
            float score = FLT_MAX;
            for (TAMSTROKE& stroke : m_strokes)
            {
                float dist = stroke.Distance(currentCandidate, (int)image.m_width);
                score = min(score, dist);
            }

            // if this score is higher than our previous best, take it as our best
            if (!haveBestCandidate || score > bestScore)
            {
                haveBestCandidate = true;
                bestScore = score;
                bestCandidate = currentCandidate;
            }
        }

        // commit the stroke to the stroke list
        m_strokes.push_back(bestCandidate);

        // commit the stroke to the texture
        m_strokes.rbegin()->DrawStroke(image);
    }

    // returns the brightness level
    float GenerateStrokes (SImageData& image, float currentBrightness, float targetBrightness)
    {
        size_t desiredStrokes = TAMSTROKE::DesiredStrokes(image, currentBrightness, targetBrightness);

        if (desiredStrokes > 0)
        {
            size_t tick = desiredStrokes / 100;

            for (size_t i = 0; i < desiredStrokes; ++i)
            {
                GenerateStroke(image, targetBrightness);

                if (tick == 0 || i % tick == 0)
                {
                    printf("               \r%i%%  (%zu / %zu)\r", int(100.0f*float(i) / float(desiredStrokes)), i, desiredStrokes);
                }
            }
            return ImageAverageBrightness(image);
        }
        else
        {
            int lastPercent = -1;

            float brightness = currentBrightness;
            while (brightness > targetBrightness)
            {
                GenerateStroke(image, targetBrightness);
                brightness = ImageAverageBrightness(image);

                int percent = 100 - int(100.0f * (targetBrightness - brightness) / (targetBrightness - currentBrightness));
                percent = min(percent, 100);
                if (percent != lastPercent)
                {
                    lastPercent = percent;
                    printf("               \r%i%%  (%0.2f / %0.2f)\r", lastPercent, brightness, targetBrightness);
                }
            }
            return brightness;
        }
    }

    std::vector<TAMSTROKE> m_strokes;
};

//======================================================================================
template <typename TAMGENERATOR>
void GenerateTAM (const char* baseFileName, size_t dimensions, size_t numShades, bool invertToMakeDarkHalf)
{
    TAMGENERATOR generator;
    char fileName[1024];
    char dftFileName[1024];

    // initialize images to starting state
    std::vector<SImageData> images;
    images.resize(numShades);
    for (size_t i = 0; i < numShades; ++i)
        ImageInit(images[i], dimensions, dimensions);
    float brightness = generator.InitializeImage(images[0]);

    // images for DFT
    SImageDataComplex dftImage;
    SImageData dftDataImage;

    // generate each shade, but don't include pure black or pure white as those are easy to generate
    for (size_t i = 0; i < numShades; ++i)
    {
        SImageData& image = images[i];
        if (i > 0)
            ImageCopy(image, images[i - 1]);

        // print file name
        sprintf(fileName, baseFileName, i);
        printf("%s\n", fileName);

        // generate the strokes, save the image and report final brightness
        if (!invertToMakeDarkHalf || i < (numShades + 1) / 2)
        {
            SBlockTimer timer(fileName);

            float desiredBrightness = 1.0f - float(i + 1) / float(numShades + 1);
            brightness = generator.GenerateStrokes(image, brightness, desiredBrightness);
            ImageSave(image, fileName);

            printf("\nbrightness = %f\n", brightness);
        }
        // invert the other images if we should
        else
        {
            SBlockTimer timer(fileName);

            ImageCopy(image, images[numShades - 1 - i]);
            ImageInvert(image);

            brightness = ImageAverageBrightness(image);

            ImageSave(image, fileName);

            printf("inverting image %zu\nbrightness = %f\n", numShades - 1 - i, brightness);
        }

        // DFT the image and save amplitude / phase information, if we should
        if (DO_DFT())
        {
            SBlockTimer timer(fileName);
            DFTImage(image, dftImage);

            GetMagnitudeData(dftImage, dftDataImage);
            strcpy(dftFileName, fileName);
            strcat(dftFileName, ".mag.bmp");
            ImageSave(dftDataImage, dftFileName);

            if (DO_DFT_PHASE())
            {
                GetPhaseData(dftImage, dftDataImage);
                strcpy(dftFileName, fileName);
                strcat(dftFileName, ".phase.bmp");
                ImageSave(dftDataImage, dftFileName);
            }
        }
        printf("\n");
    }
}

//======================================================================================
int main (int argc, char** argv)
{
    // TODO: use a grid for dots to make finding closest neighbor easier

    // TODO: this blue noise kinda sucks for tiling (why?), and also sucks when it's denser.  Increasing candidate count didn't really help much.
    //GenerateTAM<TAMGenerator_BlueNoise<TAMStroke_Pixel, 5, 1024>>("TAMs/Dots/Dots_%zu.bmp", 64, 8, false);
    //GenerateTAM<TAMGenerator_BlueNoise<TAMStroke_Pixel, 5, 1024>>("TAMs/Dots/DotsInverted_%zu.bmp", 64, 8, true);

    //GenerateTAM<TAMGenerator_BlueNoise<TAMStroke_Circle, 1, 50>>("TAMs/Dots/Circles_%zu.bmp", 256, 8, false);

    // TODO: make lines calculate distance!
    GenerateTAM<TAMGenerator_BlueNoise<TAMStroke_HorizontalLineSegment, 1, 50>>("TAMs/Dots/Lines_%zu.bmp", 256, 8, false);

    WaitForEnter();

    return 0;
}

/*
TAM GENERATOR TODO:

* test the dots and circle images in the path tracer now that you've improved them a bit
 * could try circle inversion
 * could try line inversion

* use a grid to help find closest objects faster.
 * each grid cell is a list of indices of things inside of them
 * they basically rasterize into it like they rasterize into an image i guess? or similar? maybe implement it per object.

? figure out good cost function, like length or what? (for brushy type strokes)

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

* check this out with sleepy to see what is slow

* get these loaded / working in the cross hatching project and check em out

? is dense blue noise the same as sparse blue noise with image inverted? do DFT i think and check it out?
 ? maybe ask stack exchange how to make high density 1 bit blue noise textures?
 * hypothesis: at 50% darkness, you ought to invert the problem.  Maybe work towards 50% darkness, and then invert all the textures < 50% to make > 50%?
 * if this works, may be worth a blog post.

? does it matter that the final brightness may be much dimmer than desired? maybe need to deal with that somehow by penalizing going over?

? maybe be able to write out as tga so easier to load into other program? or have the other program able to read bmps too (maybe that's easier!)

? clean up this code a bit? maybe multiple files? drawing routines, dft, etc?

? should you rename things not to be TAM since it isn't a tonal art map anymore? since there are no mips?

* the blue noise dots get worse as they get darker, maybe need to find out how to make dense blue noise samples?

* look into "hand tinted prints" - a technique where a printer prints black / white (dots?) and then someone colors over the top to give it color.
 * Aunt Raffaella actually has some of these. check em out!

*/