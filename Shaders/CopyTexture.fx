#include "ShaderTypes.h"

//----------------------------------------------------------------------------
struct SPixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//----------------------------------------------------------------------------
SPixelInput vs_main(Pos2D input)
{
    SPixelInput output;

    output.position.xy = input.position;
    output.position.z = 0.0f;
    output.position.w = 1.0f;

    output.uv = input.position * 0.5f + 0.5f;

    return output;
}

//----------------------------------------------------------------------------
float4 ps_main(SPixelInput input) : SV_TARGET
{
    return stone.Sample(SamplerLinearWrap, input.uv);
}