#include "PathTrace.h"

//----------------------------------------------------------------------------
[numthreads(32, 32, 1)]
void cs_main (
    uint3 groupID : SV_GroupID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint groupIndex : SV_GroupIndex,
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Don't write out of bounds pixels
    uint dimsX, dimsY;
    pathTraceOutput_rw.GetDimensions(dimsX, dimsY);
    if (dispatchThreadID.x > dimsX || dispatchThreadID.y > dimsY)
        return;

    // calculate screen uv
    float2 uv = float2(dispatchThreadID.xy) / float2(dimsX, dimsY);

    // calculate a rngSeed by sampling a blue noise texture for this pixel and adding frame number * golden ratio.
    // The blue noise starting seed makes the noise less harsh on the eyes.
    // The golden ratio addition makes a sort of low discrepancy sequence, even though we aren't keeping it in 0-1 range
    float2 blueNoiseUV = uv;
    blueNoiseUV.x *= float(dimsX) / 256.0f;
    blueNoiseUV.y *= float(dimsY) / 256.0f;
    float rngSeed = blueNoise256.SampleLevel(SamplerNearestWrap, blueNoiseUV, 0).r + GOLDEN_RATIO * float(sampleCount_yzw.x);

    // calculate the ray for this pixel
    float3 rayPos, rayDir;
    CalculateRay(uv, rayPos, rayDir);

    // path trace
    float3 light = Light_Incoming(rayPos, rayDir, rngSeed);

    // use lerping for incremental averageing:  https://blog.demofox.org/2016/08/23/incremental-averaging/
    // lerp from the old value to the current and write it back out
    float3 integration = lerp(pathTraceOutput_rw[dispatchThreadID.xy].xyz, light, 1.0f / float(sampleCount_yzw.x));
    pathTraceOutput_rw[dispatchThreadID.xy] = float4(integration, 1.0f);
}

//----------------------------------------------------------------------------
[numthreads(32, 32, 1)]
void cs_main_FirstHit (uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Don't write out of bounds pixels
    uint dimsX, dimsY;
    pathTraceOutput_rw.GetDimensions(dimsX, dimsY);
    if (dispatchThreadID.x > dimsX || dispatchThreadID.y > dimsY)
        return;

    // calculate screen uv
    float2 uv = float2(dispatchThreadID.xy) / float2(dimsX, dimsY);

    // calculate the ray for this pixel and get the time of the first ray hit
    float3 rayPos, rayDir;
    CalculateRay(uv, rayPos, rayDir);
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

    // write the results
    uint pixelIndex = dispatchThreadID.y * dimsX + dispatchThreadID.x;
    FirstRayHits_rw[pixelIndex].surfaceNormal_intersectTime = float4(rayHitInfo.m_surfaceNormal, rayHitInfo.m_intersectTime);
    FirstRayHits_rw[pixelIndex].albedo_w = float4(rayHitInfo.m_albedo, 0.0f);
    FirstRayHits_rw[pixelIndex].emissive_w = float4(rayHitInfo.m_emissive, 0.0f);
}