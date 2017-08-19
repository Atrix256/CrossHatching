#include "ShaderTypes.h"

//----------------------------------------------------------------------------
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

//----------------------------------------------------------------------------
PixelInputType vs_main (PosColorUV input)
{
    PixelInputType output;

    output.position.xyz = input.position;
    output.position.w = 2.0f;

    output.color = input.color * pixelColor;

    output.uv = input.uv * 5.0f;

    return output;
}

//----------------------------------------------------------------------------
float4 ps_main (PixelInputType input) : SV_TARGET
{
    return input.color * rwtexture.Sample(SamplerLinearWrap, input.uv) + stone.Sample(SamplerLinearWrap, input.uv) * 0.5f;
}