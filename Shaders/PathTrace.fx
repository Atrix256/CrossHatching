#include "ShaderTypes.h"

bool RayIntersectsSphere (in float3 rayPos, in float3 rayDir, in float4 spherePosRadius, inout float intersectTime, out float3 intersectNormal)
{
    //get the vector from the center of this circle to where the ray begins.
    float3 m = rayPos - spherePosRadius.xyz;

    //get the dot product of the above vector and the ray's vector
    float b = dot(m, rayDir);

    float c = dot(m, m) - spherePosRadius.w * spherePosRadius.w;

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
    if (intersectTime >= 0.0 && collisionTime > intersectTime)
        return false;

    float3 normal = normalize((rayPos + rayDir * collisionTime) - spherePosRadius.xyz);

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    intersectTime = collisionTime;
    intersectNormal = normal;
    return true;
}

float4 PathTrace (in float3 rayPos, in float3 rayDir)
{
    float intersectTime = -1.0f;
    float3 intersectNormal = float3(0.0f, 0.0f, 0.0f);

    int numSpheres = numSpheres_near.x;
    for (int i = 0; i < numSpheres; ++i)
    {
        RayIntersectsSphere(rayPos, rayDir, Spheres[i].posRadius, intersectTime, intersectNormal);
    }

    if (intersectTime >= 0.0f)
        return float4(0.0f, 1.0f, 0.0f, 1.0f);
    else
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
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
    float windowRight = tan(cameraPos_FOVX.w) * numSpheres_near.y;
    float windowTop = tan(cameraAt_FOVY.w) * numSpheres_near.y;

    // calculate pixel position in world space
    // start at the camera, go down the forward vector to the near plane, then move right and up based on pixelClipSpace
    float3 pixelPos = cameraPos_FOVX.xyz + cameraFwd * numSpheres_near.y;
    pixelPos += pixelClipSpace.x * cameraRight * windowRight;
    pixelPos += pixelClipSpace.y * cameraUp * windowTop;

    // calculate the direction
    float3 rayDir = normalize(pixelPos - cameraPos_FOVX.xyz);

    // path trace!
    pathTraceOutput_rw[dispatchThreadID.xy] = PathTrace(pixelPos, rayDir);


    //float4 incolor = float4(Input[0].c[0], Input[1].c[0], Input[0].c[2], Input[0].c[3]);
    //incolor.rgb *= Triangles[1].position;

    //pathTraceOutput_rw[dispatchThreadID.xy] = float4(pixelClipSpace, 0, 1);

    //pathTraceOutput_rw[dispatchThreadID.xy] = float4(frac(dispatchThreadID.xy / 100.0f), 0, 1) * incolor * pixelColor;

}
