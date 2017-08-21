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

    // position out is position in
    output.position = input.position;

    // convert position from [-1,1] space to being in [0,1] space for uv
    output.uv = input.position * 0.5f + 0.5f;

    return output;
}

//----------------------------------------------------------------------------
float4 ps_main(SPixelInput input) : SV_TARGET
{
    //return pathTraceOutput_rw[input.uv * float2(800, 600)].rgba;
    
    //= float4(frac(threadID.xy / 100.0f), 0, 1) * incolor * pixelColor;

    return pathTraceOutput.Sample(SamplerLinearWrap, input.uv);
}