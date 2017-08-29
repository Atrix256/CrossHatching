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

    // TODO: get rid of todo's to use a blue noise texture since you are?
    // TODO: get rid of hash12 not needed anymore

    // TODO: should we keep rngSeed in 0-1?

    // calculate a rngSeed by sampling a blue noise texture for this pixel and adding frame number * golden ratio.
    // The blue noise starting seed makes the noise less harsh on the eyes.
    // The golden ratio addition makes the seed into a low discprepancy sequence over time.
    // TODO: is the above comment correct if we don't keep the seed in 0,1?
    float2 blueNoiseUV = uv;
    blueNoiseUV.x *= float(dimsX) / 256.0f;
    blueNoiseUV.y *= float(dimsY) / 256.0f;
    float rngSeed = blueNoise256.SampleLevel(SamplerLinearWrap, blueNoiseUV, 0).r + GOLDEN_RATIO * frameRnd_appTime_sampleCount_numQuads.z;

    // calculate the ray for this pixel
    float3 rayPos, rayDir;
    CalculateRay(uv, rayPos, rayDir);

    // path trace
    float light = Light_Incoming(rayPos, rayDir, rngSeed);

    // use lerping for incremental averageing:  https://blog.demofox.org/2016/08/23/incremental-averaging/
    // lerp from the old value to the current and write it back out
    pathTraceOutput_rw[dispatchThreadID.xy] = lerp(pathTraceOutput_rw[dispatchThreadID.xy], light, 1.0f / frameRnd_appTime_sampleCount_numQuads.z);
}
