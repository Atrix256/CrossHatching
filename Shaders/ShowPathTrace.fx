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
float4 ps_main(SPixelInput input) : SV_TARGET
{
    // calculate the ray for this pixel and get the time of the first ray hit
    float3 rayPos, rayDir;
    CalculateRay(input.uv, rayPos, rayDir);
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

    // return the distance
    //float dist = rayHitInfo.m_intersectTime;
    //dist = dist / (dist + 1000.0f);
    //return float4(dist, dist, dist, 1.0f);

    // get the lit value and apply sRGB correction
    float light = pathTraceOutput.Sample(SamplerLinearWrap, input.uv);
    light = pow(light, 1.0f / 2.0f);

    // return the value as greyscale
    return float4(light, light, light, 1.0f);
}