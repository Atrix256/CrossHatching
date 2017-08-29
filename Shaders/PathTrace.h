#include "ShaderTypes.h"

static const float c_pi = 3.14159265359f;
static const float c_rayEpsilon = 0.001f;
static const float FLT_MAX = 3.402823466e+38F;
static const float GOLDEN_RATIO = 1.61803398875f;
static const int c_numBounces = 5;

//----------------------------------------------------------------------------
struct SRayHitInfo
{
    float  m_intersectTime;
    float3 m_surfaceNormal;
    float  m_albedo;
    float  m_emissive;
};

//----------------------------------------------------------------------------
float3 ChangeBasis(in float3 v, in float3 xAxis, in float3 yAxis, in float3 zAxis)
{
    return float3
    (
        dot(v, float3(xAxis[0], yAxis[0], zAxis[0])),
        dot(v, float3(xAxis[1], yAxis[1], zAxis[1])),
        dot(v, float3(xAxis[2], yAxis[2], zAxis[2]))
    );
}

//----------------------------------------------------------------------------
float3 UndoChangeBasis(in float3 v, in float3 xAxis, in float3 yAxis, in float3 zAxis)
{
    return float3
    (
        dot(v, xAxis),
        dot(v, yAxis),
        dot(v, zAxis)
    );
}

//----------------------------------------------------------------------------
float ScalarTriple(in float3 a, in float3 b, in float3 c)
{
    return dot(cross(a, b), c);
}

//----------------------------------------------------------------------------
void CalculateRay (in float2 uv, out float3 rayPos, out float3 rayDir)
{
    // calculate coordinate of pixel on the screen in [-1,1]
    float2 pixelClipSpace = 2.0f * uv - 1.0f;
    pixelClipSpace *= -1.0f;

    // calculate camera vectors
    float3 cameraFwd = normalize(cameraAt_FOVY.xyz - cameraPos_FOVX.xyz);
    float3 cameraRight = normalize(cross(cameraFwd, float3(0.0f, 1.0f, 0.0f)));
    float3 cameraUp = normalize(cross(cameraFwd, cameraRight));

    // calculate view window dimensions in world space
    float windowRight = tan(cameraPos_FOVX.w) * numSpheres_numTris_nearPlaneDist_missColor.z;
    float windowTop = tan(cameraAt_FOVY.w) * numSpheres_numTris_nearPlaneDist_missColor.z;

    // calculate pixel position in world space, this is the ray's origin
    // start at the camera, go down the forward vector to the near plane, then move right and up based on pixelClipSpace
    rayPos = cameraPos_FOVX.xyz + cameraFwd * numSpheres_numTris_nearPlaneDist_missColor.z;
    rayPos += pixelClipSpace.x * cameraRight * windowRight;
    rayPos += pixelClipSpace.y * cameraUp * windowTop;

    // calculate the direction
    rayDir = normalize(rayPos - cameraPos_FOVX.xyz);
}

//----------------------------------------------------------------------------
void RayIntersectsAABB (in float3 rayPos, in float3 rayDir, in OBBPrim obb, inout SRayHitInfo rayHitInfo)
{
    float rayMinTime = 0.0;
    float rayMaxTime = FLT_MAX;

    // find the intersection of the intersection times of each axis to see if / where the
    // ray hits.
    int axis = 0;
    for (axis = 0; axis < 3; ++axis)
    {
        //calculate the min and max of the box on this axis
        float axisMin = obb.position_Albedo[axis] - obb.radius_Emissive[axis];
        float axisMax = obb.position_Albedo[axis] + obb.radius_Emissive[axis];

        //if the ray is paralel with this axis
        if (abs(rayDir[axis]) < 0.0001f)
        {
            //if the ray isn't in the box, bail out we know there's no intersection
            if (rayPos[axis] < axisMin || rayPos[axis] > axisMax)
                return;
        }
        else
        {
            //figure out the intersection times of the ray with the 2 values of this axis
            float axisMinTime = (axisMin - rayPos[axis]) / rayDir[axis];
            float axisMaxTime = (axisMax - rayPos[axis]) / rayDir[axis];

            //make sure min < max
            if (axisMinTime > axisMaxTime)
            {
                float temp = axisMinTime;
                axisMinTime = axisMaxTime;
                axisMaxTime = temp;
            }

            //union this time slice with our running total time slice
            if (axisMinTime > rayMinTime)
                rayMinTime = axisMinTime;

            if (axisMaxTime < rayMaxTime)
                rayMaxTime = axisMaxTime;

            //if our time slice shrinks to below zero of a time window, we don't intersect
            if (rayMinTime > rayMaxTime)
                return;
        }
    }

    //if we got here, we do intersect, return our collision info
    bool fromInside = (rayMinTime == 0.0);
    float collisionTime;
    if (fromInside)
        collisionTime = rayMaxTime;
    else
        collisionTime = rayMinTime;

    //enforce a max distance if we should
    if (rayHitInfo.m_intersectTime >= 0.0 && collisionTime > rayHitInfo.m_intersectTime)
        return;

    float3 intersectionPoint = rayPos + rayDir * collisionTime;

    // figure out the surface normal by figuring out which axis we are closest to
    float closestDist = FLT_MAX;
    float3 normal;
    float u = 0.0f;
    float v = 0.0f;
    for (axis = 0; axis < 3; ++axis)
    {
        float distFromPos = abs(obb.position_Albedo[axis] - intersectionPoint[axis]);
        float distFromEdge = abs(distFromPos - obb.radius_Emissive[axis]);

        if (distFromEdge < closestDist)
        {
            closestDist = distFromEdge;
            normal = float3( 0.0f, 0.0f, 0.0f );
            if (intersectionPoint[axis] < obb.position_Albedo[axis])
                normal[axis] = -1.0;
            else
                normal[axis] = 1.0;
        }
    }

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    rayHitInfo.m_intersectTime = collisionTime;
    rayHitInfo.m_surfaceNormal = normal;
    rayHitInfo.m_albedo = obb.position_Albedo.w;
    rayHitInfo.m_emissive = obb.radius_Emissive.w;
}

//----------------------------------------------------------------------------
void RayIntersectsOBB (in float3 rayPos, in float3 rayDir, in OBBPrim obb, inout SRayHitInfo rayHitInfo)
{
    // put the ray into local space of the obb
    float3 newRayPos = ChangeBasis(rayPos - obb.position_Albedo.xyz, obb.XAxis_w, obb.YAxis_w, obb.ZAxis_w) + obb.position_Albedo.xyz;
    float3 newRayDir = ChangeBasis(rayDir, obb.XAxis_w, obb.YAxis_w, obb.ZAxis_w);

    // do ray vs abb intersection
    RayIntersectsAABB(newRayPos, newRayDir, obb, rayHitInfo);
    if (rayHitInfo.m_intersectTime < 0.0f)
        return;

    // convert surface normal back to global space
    rayHitInfo.m_surfaceNormal = UndoChangeBasis(rayHitInfo.m_surfaceNormal, obb.XAxis_w, obb.YAxis_w, obb.ZAxis_w);
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
        r = u*quad.positionA_Albedo.xyz + v*quad.positionD_w.xyz + w*quad.positionC_w.xyz;
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

    // obbs
    {
        int numOBBs = numOBBs_yzw.x;
        for (int i = 0; i < numOBBs; ++i)
            RayIntersectsOBB(rayPos, rayDir, OBBs[i], rayHitInfo);
    }

    return rayHitInfo;
}

//----------------------------------------------------------------------------
// Links to some shader friendly prngs:
// https://www.shadertoy.com/view/4djSRW "Hash Without Sine" by Dave_Hoskins
// https://www.shadertoy.com/view/MsV3z3 2d Weyl Hash by MBR
// https://github.com/gheshu/gputracer/blob/master/src/depth.glsl#L43 From Lauren @lh0xfb
// https://www.shadertoy.com/view/4tl3z4
//----------------------------------------------------------------------------
// from "hash without sine" https://www.shadertoy.com/view/4djSRW
//  2 out, 1 in...
float2 hash21(inout float p)
{
    float3 p3 = frac(float3(p,p,p) * float3(0.1031f, 0.1030f, 0.0973f));
    p3 += dot(p3, p3.yzx + 19.19);
    float2 ret = frac((p3.xx + p3.yz)*p3.zy);
    p += 0.3514;
    return ret;
}

//----------------------------------------------------------------------------
// from smallpt path tracer: http://www.kevinbeason.com/smallpt/
float3 CosineSampleHemisphere (in float3 normal, inout float rngSeed)
{
    float2 rnd = hash21(rngSeed);

    float r1 = 2.0f * c_pi * rnd.x;
    float r2 = rnd.y;
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
float Light_Outgoing (in SRayHitInfo rayHitInfo, in float3 rayHitPos, inout float rngSeed)
{
    float lightSum = 0.0f;
    float lightMultiplier = 1.0f;
    
    for (int i = 0; i < c_numBounces; ++i)
    {
        // update our light sum and future light multiplier
        lightSum += rayHitInfo.m_emissive * lightMultiplier;
        lightMultiplier *= rayHitInfo.m_albedo;

        // add a random recursive sample for global illumination
        float3 newRayDir = CosineSampleHemisphere(rayHitInfo.m_surfaceNormal, rngSeed);
        SRayHitInfo newRayHitInfo = ClosestIntersection(rayHitPos + newRayDir * c_rayEpsilon, newRayDir);

        // if we hit something new, we continue
        if (newRayHitInfo.m_intersectTime >= 0.0f)
        {
            rayHitInfo = newRayHitInfo;
            rayHitPos += newRayDir * newRayHitInfo.m_intersectTime;
        }
        // else we missed so light using the miss color (skybox lighting) and return the light we've summed up
        else
        {
            lightSum += numSpheres_numTris_nearPlaneDist_missColor.w * lightMultiplier;
            return lightSum;
        }
    }

    return lightSum;
}

//----------------------------------------------------------------------------
float Light_Incoming (in float3 rayPos, in float3 rayDir, inout float rngSeed)
{
    // find out what our ray hit first
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

    // if it missed, return the miss color
    if (rayHitInfo.m_intersectTime < 0.0f)
        return numSpheres_numTris_nearPlaneDist_missColor.w;

    // else, return the amount of light coming towards us from that point on the object we hit
    return Light_Outgoing(rayHitInfo, rayPos + rayDir * rayHitInfo.m_intersectTime + -rayDir * c_rayEpsilon, rngSeed);
}