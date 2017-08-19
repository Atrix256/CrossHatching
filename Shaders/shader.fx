#include "ShaderTypes.h"

/////////////
// GLOBALS //
/////////////

SamplerState SampleType;

//////////////
// TYPEDEFS //
//////////////

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType vs_main (PosColorUV input)
{
    PixelInputType output;

    input.position.w = 1.0f;
    output.position = input.position;
    output.position.w = 2.0f;

    output.color = input.color * pixelColor;

    output.uv = input.uv * 5.0f;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ps_main (PixelInputType input) : SV_TARGET
{
    return input.color * rwtexture.Sample(SampleType, input.uv) + stone.Sample(SampleType, input.uv) * 0.5f;
}