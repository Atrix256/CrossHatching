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
    // get the lit value
    float light = pathTraceOutput.Sample(SamplerLinearWrap, input.uv);

    // apply sRGB correction
    light = pow(light, 1.0f / 2.0f);

    // return the value as greyscale
    return float4(light, light, light, 1.0f);
}