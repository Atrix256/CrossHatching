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
    // TODO: it seems specific to the compute shader, i wonder why?
    // TODO: temp! to investigate ray vs quad problem
#if 1
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
#else
    // calculate a rngSeed by sampling a blue noise texture for this pixel and adding frame number * golden ratio.
    // The blue noise starting seed makes the noise less harsh on the eyes.
    // The golden ratio addition makes a sort of low discrepancy sequence, even though we aren't keeping it in 0-1 range
    float2 blueNoiseUV = input.uv;
    blueNoiseUV.x *= 800.0f / 256.0f;
    blueNoiseUV.y *= 600.0f / 256.0f;
    float rngSeed = blueNoise256.SampleLevel(SamplerNearestWrap, blueNoiseUV, 0).r;// +GOLDEN_RATIO * float(sampleCount_yzw.x);

    // calculate the ray for this pixel
    float3 rayPos, rayDir;
    CalculateRay(input.uv, rayPos, rayDir);

    // path trace
    float light = 0.0f;// Light_Incoming(rayPos, rayDir, rngSeed);

    for (int i = 1; i <= 64; ++i)
    {
        light = lerp(light, Light_Incoming(rayPos, rayDir, rngSeed), 1.0f / float(i));
    }

    return float4(light, light, light, 1.0f);

    // use lerping for incremental averageing:  https://blog.demofox.org/2016/08/23/incremental-averaging/
    // lerp from the old value to the current and write it back out
    //pathTraceOutput_rw[dispatchThreadID.xy] = lerp(pathTraceOutput_rw[dispatchThreadID.xy], light, 1.0f / float(sampleCount_yzw.x));
#endif
}