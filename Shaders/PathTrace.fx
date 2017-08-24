#include "ShaderTypes.h"

static const float c_pi = 3.14159265359f;
static const float c_rayEpsilon = 0.001f;

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
    // TODO:  443.8975f or 0.1031f?
    float3 p3 = frac(seed * 0.1031f);
    p3 += dot(p3, p3.yzx + 19.19);
    return frac((p3.x + p3.y) * p3.z);
}

//----------------------------------------------------------------------------
// from smallpt path tracer: http://www.kevinbeason.com/smallpt/
float3 CosineSampleHemisphere (in float3 normal, in float pixelIndex, in float depth)
{
    float r1 = 2.0f * c_pi * RandomFloat(float3(frameRnd_appTime_sampleCount_numQuads.x, pixelIndex, depth + 0.1238f));
    float r2 = RandomFloat(float3(frameRnd_appTime_sampleCount_numQuads.x, pixelIndex, depth + 0.3167f));
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
float ScalarTriple (in float3 a, in float3 b, in float3 c)
{
    return dot(cross(a, b), c);
}

//----------------------------------------------------------------------------
void RayIntersectsQuad (in float3 rayPos, in float3 rayDir, in QuadPrim quad, inout SRayHitInfo rayHitInfo)
{
    // This function adapted from "Real Time Collision Detection" 5.3.5 Intersecting Line Against Quadrilateral
    // IntersectLineQuad()
    float3 pa = quad.positionA_Albedo.xyz - rayPos;
    float3 pb = quad.positionB_Emissive.xyz - rayPos;
    float3 pc = quad.positionC_w.xyz - rayPos;
    // Determine which triangle to test against by testing against diagonal first
    float3 m = cross(pc, rayDir);
    float3 r;
    float v = dot(pa, m); // ScalarTriple(pq, pa, pc);
    if (v >= 0.0f) {
        // Test intersection against triangle abc
        float u = -dot(pb, m); // ScalarTriple(pq, pc, pb);
        if (u < 0.0f) return;
        float w = ScalarTriple(rayDir, pb, pa);
        if (w < 0.0f) return;
        // Compute r, r = u*a + v*b + w*c, from barycentric coordinates (u, v, w)
        float denom = 1.0f / (u + v + w);
        u *= denom;
        v *= denom;
        w *= denom; // w = 1.0f - u - v;
        r = u*quad.positionA_Albedo.xyz + v*quad.positionB_Emissive.xyz + w*quad.positionC_w.xyz;
    }
    else {
        // Test intersection against triangle dac
        float3 pd = quad.positionD_w.xyz - rayPos;
        float u = dot(pd, m); // ScalarTriple(pq, pd, pc);
        if (u < 0.0f) return;
        float w = ScalarTriple(rayDir, pa, pd);
        if (w < 0.0f) return;
        v = -v;
        // Compute r, r = u*a + v*d + w*c, from barycentric coordinates (u, v, w)
        float denom = 1.0f / (u + v + w);
        u *= denom;
        v *= denom;
        w *= denom; // w = 1.0f - u - v;
        r = u*quad.positionA_Albedo.xyz + v*quad.positionD_w + w*quad.positionC_w.xyz;
    }

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    float3 normal = quad.normal_w.xyz;
    if (dot(quad.normal_w.xyz, rayDir) > 0.0f)
        normal *= -1.0f;

    // figure out the time t that we hit the plane (quad)
    float t;
    if (abs(rayDir[0]) > 0.0f)
        t = (r[0] - rayPos[0]) / rayDir[0];
    else if (abs(rayDir[1]) > 0.0f)
        t = (r[1] - rayPos[1]) / rayDir[1];
    else if (abs(rayDir[2]) > 0.0f)
        t = (r[2] - rayPos[2]) / rayDir[2];

    // only positive time hits allowed!
    if (t < 0.0f)
        return;

    //enforce a max distance if we should
    if (rayHitInfo.m_intersectTime >= 0.0 && t > rayHitInfo.m_intersectTime)
        return;

    rayHitInfo.m_intersectTime = t;
    rayHitInfo.m_surfaceNormal = normal;
    rayHitInfo.m_albedo = quad.positionA_Albedo.w;
    rayHitInfo.m_emissive = quad.positionB_Emissive.w;
    return;
}

//----------------------------------------------------------------------------
void RayIntersectsSphere (in float3 rayPos, in float3 rayDir, in SpherePrim sphere, inout SRayHitInfo rayHitInfo)
{
    //get the vector from the center of this circle to where the ray begins.
    float3 m = rayPos - sphere.position_Radius.xyz;

    //get the dot product of the above vector and the ray's vector
    float b = dot(m, rayDir);

    float c = dot(m, m) - sphere.position_Radius.w * sphere.position_Radius.w;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr <= 0.0)
        return;

    //ray now found to intersect sphere, compute smallest t value of intersection
    float collisionTime = -b - sqrt(discr);

    //if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
    if (collisionTime < 0.0)
        collisionTime = -b + sqrt(discr);

    //enforce a max distance if we should
    if (rayHitInfo.m_intersectTime >= 0.0 && collisionTime > rayHitInfo.m_intersectTime)
        return;

    float3 normal = normalize((rayPos + rayDir * collisionTime) - sphere.position_Radius.xyz);

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    rayHitInfo.m_intersectTime = collisionTime;
    rayHitInfo.m_surfaceNormal = normal;
    rayHitInfo.m_albedo = sphere.albedo_Emissive_zw.x;
    rayHitInfo.m_emissive = sphere.albedo_Emissive_zw.y;
    return;
}

//----------------------------------------------------------------------------
void RayIntersectsTriangle (in float3 rayPos, in float3 rayDir, in TrianglePrim trianglePrim, inout SRayHitInfo rayHitInfo)
{
    // This function adapted from GraphicsCodex.com

    /* If ray P + tw hits triangle V[0], V[1], V[2], then the function returns true,
    stores the barycentric coordinates in b[], and stores the distance to the intersection
    in t. Otherwise returns false and the other output parameters are undefined.*/

    // Edge vectors
    float3 e_1 = trianglePrim.positionB_Emissive.xyz - trianglePrim.positionA_Albedo.xyz;
    float3 e_2 = trianglePrim.positionC_w.xyz - trianglePrim.positionA_Albedo.xyz;

    float3 q = cross(rayDir, e_2);
    float a = dot(e_1, q);

    if (abs(a) == 0.0f)
        return;

    float3 s = (rayPos - trianglePrim.positionA_Albedo.xyz) / a;
    float3 r = cross(s, e_1);
    float3 b; // b is barycentric coordinates
    b[0] = dot(s, q);
    b[1] = dot(r, rayDir);
    b[2] = 1.0f - b[0] - b[1];
    // Intersected outside triangle?
    if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f))
        return;
    float t = dot(e_2, r);
    if (t < 0.0f)
        return;

    //enforce a max distance if we should
    if (rayHitInfo.m_intersectTime >= 0.0 && t > rayHitInfo.m_intersectTime)
        return;

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    float3 normal = trianglePrim.normal_w.xyz;
    if (dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    rayHitInfo.m_intersectTime = t;
    rayHitInfo.m_surfaceNormal = normal;
    rayHitInfo.m_albedo = trianglePrim.positionA_Albedo.w;
    rayHitInfo.m_emissive = trianglePrim.positionB_Emissive.w;
    return;
}

//----------------------------------------------------------------------------
SRayHitInfo ClosestIntersection (in float3 rayPos, in float3 rayDir)
{
    SRayHitInfo rayHitInfo;
    rayHitInfo.m_intersectTime = -1.0f;

    // spheres
    {
        int numSpheres = numSpheres_numTris_nearPlaneDist_missColor.x;
        for (int i = 0; i < numSpheres; ++i)
            RayIntersectsSphere(rayPos, rayDir, Spheres[i], rayHitInfo);
    }

    // triangles
    {
        int numTris = numSpheres_numTris_nearPlaneDist_missColor.y;
        for (int i = 0; i < numTris; ++i)
            RayIntersectsTriangle(rayPos, rayDir, Triangles[i], rayHitInfo);
    }

    // quads
    {
        int numQuads = frameRnd_appTime_sampleCount_numQuads.w;
        for (int i = 0; i < numQuads; ++i)
            RayIntersectsQuad(rayPos, rayDir, Quads[i], rayHitInfo);
    }

    return rayHitInfo;
}

//----------------------------------------------------------------------------
float Light_Outgoing_0 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir, in float pixelIndex)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in albedo * miss color
    light += rayHitInfo.m_albedo * numSpheres_numTris_nearPlaneDist_missColor.w;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_1 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir, in float pixelIndex)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal, pixelIndex, 1.0f);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += rayHitInfo.m_albedo * Light_Outgoing_0(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir, pixelIndex);
    else
        light += rayHitInfo.m_albedo * numSpheres_numTris_nearPlaneDist_missColor.w;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_2 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir, in float pixelIndex)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal, pixelIndex, 2.0f);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += rayHitInfo.m_albedo * Light_Outgoing_1(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir, pixelIndex);
    else
        light += rayHitInfo.m_albedo * numSpheres_numTris_nearPlaneDist_missColor.w;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_3 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir, in float pixelIndex)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal, pixelIndex, 3.0f);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += rayHitInfo.m_albedo * Light_Outgoing_2(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir, pixelIndex);
    else
        light += rayHitInfo.m_albedo * numSpheres_numTris_nearPlaneDist_missColor.w;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_4 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir, in float pixelIndex)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal, pixelIndex, 4.0f);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += rayHitInfo.m_albedo * Light_Outgoing_3(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir, pixelIndex);
    else
        light += rayHitInfo.m_albedo * numSpheres_numTris_nearPlaneDist_missColor.w;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Outgoing_5 (in SRayHitInfo rayHitInfo, in float3 rayHitPos, in float3 outDir, in float pixelIndex)
{
    // start with emissive lighting
    float light = rayHitInfo.m_emissive;

    // add in a random recursive sample for global illumination
    float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal, pixelIndex, 5.0f);
    SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos, newRayDir);
    if (newRayHitInfo.m_intersectTime >= 0.0f)
        light += rayHitInfo.m_albedo * Light_Outgoing_4(newRayHitInfo, rayHitPos + newRayDir * newRayHitInfo.m_intersectTime + -newRayDir * c_rayEpsilon, -newRayDir, pixelIndex);
    else
        light += rayHitInfo.m_albedo * numSpheres_numTris_nearPlaneDist_missColor.w;

    // return our recursively calculated light amount
    return light;
}

//----------------------------------------------------------------------------
float Light_Incoming (in float3 rayPos, in float3 rayDir, in uint pixelIndex)
{
    // find out what our ray hit first
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

    // if it missed, return the miss color
    if (rayHitInfo.m_intersectTime < 0.0f)
        return numSpheres_numTris_nearPlaneDist_missColor.w;

    // else, return the amount of light coming towards us from that point on the object we hit
    return Light_Outgoing_5(rayHitInfo, rayPos + rayDir * rayHitInfo.m_intersectTime + -rayDir * c_rayEpsilon, -rayDir, pixelIndex);
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
    float pixelIndex = dispatchThreadID.y * dimsY + dispatchThreadID.x;

    // calculate coordinate of pixel on the screen in [-1,1]
    float2 pixelClipSpace = 2.0f * float2(dispatchThreadID.xy) / float2(dimsX, dimsY) - 1.0f;
    pixelClipSpace *= -1.0f;

    // calculate camera vectors
    float3 cameraFwd = normalize(cameraAt_FOVY.xyz - cameraPos_FOVX.xyz);
    float3 cameraRight = normalize(cross(cameraFwd, float3(0.0f, 1.0f, 0.0f)));
    float3 cameraUp = normalize(cross(cameraFwd, cameraRight));

    // calculate view window dimensions in world space
    float windowRight = tan(cameraPos_FOVX.w) * numSpheres_numTris_nearPlaneDist_missColor.z;
    float windowTop = tan(cameraAt_FOVY.w) * numSpheres_numTris_nearPlaneDist_missColor.z;

    // calculate pixel position in world space
    // start at the camera, go down the forward vector to the near plane, then move right and up based on pixelClipSpace
    float3 pixelPos = cameraPos_FOVX.xyz + cameraFwd * numSpheres_numTris_nearPlaneDist_missColor.z;
    pixelPos += pixelClipSpace.x * cameraRight * windowRight;
    pixelPos += pixelClipSpace.y * cameraUp * windowTop;

    // calculate the direction
    float3 rayDir = normalize(pixelPos - cameraPos_FOVX.xyz);

    // path trace!
    float light = Light_Incoming(pixelPos, rayDir, pixelIndex);
    pathTraceOutput_rw[dispatchThreadID.xy] = lerp(pathTraceOutput_rw[dispatchThreadID.xy], float4(light, light, light, light), 1.0f / frameRnd_appTime_sampleCount_numQuads.z);
}


// TODO: make a "miss color". this is a solid color emissive sky box. Have it sent in as a constant in the scene data.