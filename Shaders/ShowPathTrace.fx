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
    return pathTraceOutput.Sample(SamplerLinearWrap, input.uv);
}