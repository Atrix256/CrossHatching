#include "PathTrace.h"

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
    output.uv = input.position.xy * 0.5f + 0.5f;

    return output;
}

//----------------------------------------------------------------------------
float3 GetPixelColor (SPixelInput input, bool greyScale, bool crossHatch)
{
    // calculate the ray for this pixel and get the time of the first ray hit
    float3 rayPos, rayDir;
    CalculateRay(float2(1.0f, 1.0f) - input.uv, rayPos, rayDir);
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

    // return the distance
    //float dist = rayHitInfo.m_intersectTime;
    //dist = dist / (dist + 1000.0f);
    //return float4(dist, dist, dist, 1.0f);

    // get the lit value
    float3 light;
    if (greyScale)
    {
        light.xyz = dot(pathTraceOutput.Sample(SamplerLinearWrap, float2(1.0f, 1.0f) - input.uv).xyz, float3(0.3f, 0.59f, 0.11f));
    }
    else
    {
        light = pathTraceOutput.Sample(SamplerLinearWrap, float2(1.0f, 1.0f) - input.uv).xyz;
    }

    if (crossHatch)
    {
        // TODO: temp! need to do triplanar projection to get uv to sample here, don't use input uv
        light *= crosshatch5.Sample(SamplerLinearWrap, float2(1.0f, 1.0f) - input.uv).xyz;
    }

    // sRGB correct
    light = pow(light, 1.0f / 2.0f);

    // return the value as greyscale
    return float4(light, 1.0f);
}

//----------------------------------------------------------------------------
float4 ps_main_color_shade(SPixelInput input) : SV_TARGET
{
    float3 light = GetPixelColor(input, false, false);
    return float4(light, 1.0f);
}

//----------------------------------------------------------------------------
float4 ps_main_grey_shade(SPixelInput input) : SV_TARGET
{
    float3 light = GetPixelColor(input, true, false);
    return float4(light, 1.0f);
}

//----------------------------------------------------------------------------
float4 ps_main_color_crosshatch(SPixelInput input) : SV_TARGET
{
    float3 light = GetPixelColor(input, false, true);
    return float4(light, 1.0f);
}

//----------------------------------------------------------------------------
float4 ps_main_grey_crosshatch(SPixelInput input) : SV_TARGET
{
    float3 light = GetPixelColor(input, true, true);
    return float4(light, 1.0f);
}