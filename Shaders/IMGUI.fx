#include "ShaderTypes.h"

Texture2D imgui;

//----------------------------------------------------------------------------
struct SPixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

//----------------------------------------------------------------------------
SPixelInput vs_main (IMGUI input)
{
    SPixelInput output;
    output.position = float4(input.position_uv.xy, 0.0f, 1.0f);

    output.position.x /= width_height_zw.x;
    output.position.y /= width_height_zw.y;
    output.position.xy = output.position.xy * 2.0f - 1.0f;

    output.position.y *= -1.0f;

    output.uv = input.position_uv.zw;
    output.color = input.color;
    return output;
}

//----------------------------------------------------------------------------
float4 ps_main (SPixelInput input) : SV_TARGET
{
    return input.color * imgui.Sample(SamplerLinearWrap, input.uv);
}