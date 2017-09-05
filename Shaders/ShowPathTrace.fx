#include "PathTrace.h"

// TODO: temp til volume textures are more formalized
Texture3D chvolume;

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

    // have a fallback sphere in the sky to catch anything missed
    if (rayHitInfo.m_intersectTime < 0.0f)
    {
        SpherePrim fallbackSphere;
        fallbackSphere.position_Radius.xyz = cameraPos_FOVX.xyz;
        fallbackSphere.position_Radius.w = 10.0f;
        RayIntersectsSphere(rayPos, rayDir, fallbackSphere, rayHitInfo);
    }

    // get the lit value and brightness
    float3 light = pathTraceOutput.Sample(SamplerLinearWrap, float2(1.0f, 1.0f) - input.uv).xyz;
    float brightness = dot(light, float3(0.3f, 0.59f, 0.11f));
    if (greyScale)
        light.xyz = brightness;

    // TODO: do HDR to SDR here?

    // apply cross hatching
    if (crossHatch)
    {
        // sample the crosshatching with triplanar projection
        float3 pixelPos = rayPos + rayDir * rayHitInfo.m_intersectTime;

        float2 uvx = pixelPos.yz;
        float2 uvy = pixelPos.xz;
        float2 uvz = pixelPos.xy;
        
        // TODO: brightness of pixel needs to be used to select which crosshatch slice to use. (+do trilinear interpolation)
        /*
        float crossHatchTexel =
            crosshatch5.Sample(SamplerLinearWrap, uvx).r * rayHitInfo.m_surfaceNormal.x +
            crosshatch5.Sample(SamplerLinearWrap, uvy).r * rayHitInfo.m_surfaceNormal.y +
            crosshatch5.Sample(SamplerLinearWrap, uvz).r * rayHitInfo.m_surfaceNormal.z;
        */

        // TODO: not sure if brightness is used correctly here. does it want a 0-1?
        // TODO; also should be SDR brightness, right?
        float crossHatchTexel =
            chvolume.Sample(SamplerLinearWrap, float3(uvx, brightness)).r * rayHitInfo.m_surfaceNormal.x +
            chvolume.Sample(SamplerLinearWrap, float3(uvy, brightness)).r * rayHitInfo.m_surfaceNormal.y +
            chvolume.Sample(SamplerLinearWrap, float3(uvz, brightness)).r * rayHitInfo.m_surfaceNormal.z;

        crossHatchTexel = crossHatchTexel / (rayHitInfo.m_surfaceNormal.x + rayHitInfo.m_surfaceNormal.y + rayHitInfo.m_surfaceNormal.z);

        // apply crosshatching
        if (greyScale)
            light = crossHatchTexel;
        else
            light *= crossHatchTexel;
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