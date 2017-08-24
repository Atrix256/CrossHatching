#include "ShaderTypes.h"

static const float c_pi = 3.14159265359f;
static const float c_rayEpsilon = 0.01f;

// TODO: convert the repeated faked recursive functions into a for loop and then make a constant for number of iterations again.
// TODO: for seed, use thread id, and some index that increments with each call i think.
// TODO: incrementally average samples from frame to frame

//----------------------------------------------------------------------------
struct SRayHitInfo
{
    float  m_intersectTime;
    float3 m_surfaceNormal;
    float  m_albedo;
    float  m_emissive;
};

//----------------------------------------------------------------------------
// this is hash13() from the "Hash without Sine" shadertoy by Dave_Hoskins.
// https://www.shadertoy.com/view/4djSRW
float RandomFloat (float3 seed)
{
    float3 p3 = frac(seed * 443.8975);
    p3 += dot(p3, p3.yzx + 19.19);
    return frac((p3.x + p3.y) * p3.z);
}

//----------------------------------------------------------------------------
// from smallpt path tracer: http://www.kevinbeason.com/smallpt/
float3 CosineSampleHemisphere (in float3 normal)
{
    float r1 = 2.0f * c_pi * RandomFloat(float3(numSpheres_near_appTime_w.z, 0.2435f, 0.1238f));
    float r2 = RandomFloat(float3(numSpheres_near_appTime_w.z, 0.8941f, 0.3167f));
    float r2s = sqrt(r2);

    float3 w = normal;
    float3 u;
    if (abs(w[0]) > 0.1f)
        u = cross(float3(0.0f, 1.0f, 0.0f), w);
    else
        u = cross(float3(1.0f, 0.0f, 0.0f), w);

    u = normalize(u);
    float3 v = cross(w, u);
    float3 d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1.0f - r2));
    d = normalize(d);

    return d;
}

//----------------------------------------------------------------------------
bool RayIntersectsSphere (in float3 rayPos, in float3 rayDir, in Sphere sphere, inout SRayHitInfo rayHitInfo)
{
    //get the vector from the center of this circle to where the ray begins.
    float3 m = rayPos - sphere.position_Radius.xyz;

    //get the dot product of the above vector and the ray's vector
    float b = dot(m, rayDir);

    float c = dot(m, m) - sphere.position_Radius.w * sphere.position_Radius.w;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr <= 0.0)
        return false;

    //ray now found to intersect sphere, compute smallest t value of intersection
    float collisionTime = -b - sqrt(discr);

    //if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
    if (collisionTime < 0.0)
        collisionTime = -b + sqrt(discr);

    //enforce a max distance if we should
    if (rayHitInfo.m_intersectTime >= 0.0 && collisionTime > rayHitInfo.m_intersectTime)
        return false;

    float3 normal = normalize((rayPos + rayDir * collisionTime) - sphere.position_Radius.xyz);

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    rayHitInfo.m_intersectTime = collisionTime;
    rayHitInfo.m_surfaceNormal = normal;
    rayHitInfo.m_albedo = sphere.albedo_Emissive_zw.x;
    rayHitInfo.m_emissive = sphere.albedo_Emissive_zw.y;
    return true;
}

//----------------------------------------------------------------------------
SRayHitInfo ClosestIntersection (in float3 rayPos, in float3 rayDir)
{
    SRayHitInfo rayHitInfo;
    rayHitInfo.m_intersectTime = -1.0f;

    int numSpheres = numSpheres_near_appTime_w.x;
    for (int i = 0; i < numSpheres; ++i)
        RayIntersectsSphere(rayPos, rayDir, Spheres[i], rayHitInfo);

    return rayHitInfo;
}

//----------------------------------------------------------------------------
float Light_Outgoing_0 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_1 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += Light_Outgoing_0(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir);

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_2 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += Light_Outgoing_1(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir);

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_3 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += Light_Outgoing_2(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir);

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_4 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += Light_Outgoing_3(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir);

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_5 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += Light_Outgoing_4(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir);

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Incoming (in float3 rayPos, in float3 rayDir)
{
    // find out what our ray hit first
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

    // if it missed, return darkness
    if (rayHitInfo.m_intersectTime < 0.0f)
        return 0.0f;

    // else, return the amount of light coming towards us from that point on the object we hit
    return Light_Outgoing_5(rayHitInfo, rayPos + rayDir * rayHitInfo.m_intersectTime + -rayDir * c_rayEpsilon, -rayDir);
}

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

    // calculate coordinate of pixel on the screen in [-1,1]
    float2 pixelClipSpace = 2.0f * float2(dispatchThreadID.xy) / float2(dimsX, dimsY) - 1.0f;

    // calculate camera vectors
    float3 cameraFwd = normalize(cameraAt_FOVY.xyz - cameraPos_FOVX.xyz);
    float3 cameraRight = normalize(cross(cameraFwd, float3(0.0f, 1.0f, 0.0f)));
    float3 cameraUp = normalize(cross(cameraFwd, cameraRight));

    // calculate view window dimensions in world space
    float windowRight = tan(cameraPos_FOVX.w) * numSpheres_near_appTime_w.y;
    float windowTop = tan(cameraAt_FOVY.w) * numSpheres_near_appTime_w.y;

    // calculate pixel position in world space
    // start at the camera, go down the forward vector to the near plane, then move right and up based on pixelClipSpace
    float3 pixelPos = cameraPos_FOVX.xyz + cameraFwd * numSpheres_near_appTime_w.y;
    pixelPos += pixelClipSpace.x * cameraRight * windowRight;
    pixelPos += pixelClipSpace.y * cameraUp * windowTop;

    // calculate the direction
    float3 rayDir = normalize(pixelPos - cameraPos_FOVX.xyz);

    // path trace!
    float light = Light_Incoming(pixelPos, rayDir);
    pathTraceOutput_rw[dispatchThreadID.xy] = float4(light, light, light, light);
}
