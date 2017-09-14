
#include "PathTrace.h"

//----------------------------------------------------------------------------
[numthreads(32, 32, 1)]
void cs_main (uint3 dispatchThreadID : SV_DispatchThreadID)
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