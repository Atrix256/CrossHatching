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
float SampleHatchingTexture (float3 worldPos, float3 normal, float brightness, bool aniso, bool explicitCrossHatch)
{
    // caclulate the array slice to read
    uint volumeDimsX, volumeDimsY, volumeDimsZ;
    circlesarray.GetDimensions(volumeDimsX, volumeDimsY, volumeDimsZ);
    float w = brightness * float(volumeDimsZ);

    // triplanar projection sample the crosshatching texture
    // manually lerp to get trilinear interpolation between slice samples
    float2 uvx = worldPos.yz * uvmultiplier_blackPoint_whitePoint_triplanarPow.x;
    float2 uvy = worldPos.xz * uvmultiplier_blackPoint_whitePoint_triplanarPow.x;
    float2 uvz = worldPos.xy * uvmultiplier_blackPoint_whitePoint_triplanarPow.x;

    SamplerState chosenSampler = aniso ? SamplerAnisoWrap : SamplerLinearWrap;

    float3 absNormal = pow(abs(normal), uvmultiplier_blackPoint_whitePoint_triplanarPow.w);
    float absNormalWeight = absNormal.x + absNormal.y + absNormal.z;

    float crossHatchTexelFloor =
        circlesarray.Sample(chosenSampler, float3(uvx, floor(w))).r * absNormal.x +
        circlesarray.Sample(chosenSampler, float3(uvy, floor(w))).r * absNormal.y +
        circlesarray.Sample(chosenSampler, float3(uvz, floor(w))).r * absNormal.z;
    crossHatchTexelFloor /= absNormalWeight;

    float crossHatchTexelCeil =
        circlesarray.Sample(chosenSampler, float3(uvx, ceil(w))).r * absNormal.x +
        circlesarray.Sample(chosenSampler, float3(uvy, ceil(w))).r * absNormal.y +
        circlesarray.Sample(chosenSampler, float3(uvz, ceil(w))).r * absNormal.z;
    crossHatchTexelCeil /= absNormalWeight;
    
    float crossHatchTexel = lerp(crossHatchTexelFloor, crossHatchTexelCeil, frac(w));

    // do explicit cross hatching if we should, by doing a perpendicular sample with uv's *2, averaged in to the other
    if (explicitCrossHatch)
    {
        crossHatchTexelFloor =
            circlesarray.Sample(chosenSampler, float3(uvx.yx * 2.0f, floor(w))).r * absNormal.x +
            circlesarray.Sample(chosenSampler, float3(uvy.yx * 2.0f, floor(w))).r * absNormal.y +
            circlesarray.Sample(chosenSampler, float3(uvz.yx * 2.0f, floor(w))).r * absNormal.z;
        crossHatchTexelFloor /= absNormalWeight;

        crossHatchTexelCeil =
            circlesarray.Sample(chosenSampler, float3(uvx.yx * 2.0f, ceil(w))).r * absNormal.x +
            circlesarray.Sample(chosenSampler, float3(uvy.yx * 2.0f, ceil(w))).r * absNormal.y +
            circlesarray.Sample(chosenSampler, float3(uvz.yx * 2.0f, ceil(w))).r * absNormal.z;
        crossHatchTexelCeil /= absNormalWeight;

        float otherCrossHatchTexel = lerp(crossHatchTexelFloor, crossHatchTexelCeil, frac(w));

        crossHatchTexel = (crossHatchTexel + otherCrossHatchTexel) * 0.5f;
    }
    
    return crossHatchTexel;
}

//----------------------------------------------------------------------------
float3 GetPixelColor (SPixelInput input, bool greyScale, bool crossHatch, bool smoothStep, bool aniso, bool explicitCrossHatch)
{
    // get our first ray hit info from the FirstRayHits buffer
    uint dimsX, dimsY;
    pathTraceOutput.GetDimensions(dimsX, dimsY);
    uint2 pixelPos = uint2(input.uv * float2(dimsX, dimsY));
    uint pixelIndex = pixelPos.y * dimsX + pixelPos.x;
    SRayHitInfo rayHitInfo;
    rayHitInfo.m_surfaceNormal = FirstRayHits[pixelIndex].surfaceNormal_intersectTime.xyz;
    rayHitInfo.m_intersectTime = FirstRayHits[pixelIndex].surfaceNormal_intersectTime.w;
    rayHitInfo.m_albedo = FirstRayHits[pixelIndex].albedo_w.xyz;
    rayHitInfo.m_emissive = FirstRayHits[pixelIndex].emissive_w.xyz;

    // calculate the ray for this pixel and get the time of the first ray hit
    float3 rayPos, rayDir;
    CalculateRay(input.uv, rayPos, rayDir);

    // get the lit value
    float3 light = pathTraceOutput.Sample(SamplerLinearWrap, input.uv).xyz;

    // if the ray didn't hit anything, fake a planar hit for shading purposes
    if (rayHitInfo.m_intersectTime < 0.0f)
    {
        // alternately could position the sphere at rayPos, but that makes the triplanar projection change as the camera moves!
        rayPos = float3(0.0f, 0.0f, 0.0f);

        SpherePrim sphere;
        sphere.position_Radius = float4(rayPos, 10.0f);
        sphere.albedo_w = float4(0.0f, 0.0f, 0.0f, 0.0f);
        sphere.emissive_w = float4(0.0f, 0.0f, 0.0f, 0.0f);

        RayIntersectsSphere(rayPos, rayDir, sphere, rayHitInfo);

        light = nearPlaneDist_missColor.yzw;
    }

    // store off the color sample for use with overlay opacity blending
    float3 colorSample = light;

	// reinhard operator to convert from HDR to SDR
	light = light / (light + 1.0f);

	// get the brightness of this SDR RGB value. aka get Y from YUV.
    float brightness = dot(light, float3(0.299f, 0.587f, 0.114f));

    // remap the brightness using the black point / white point
    brightness = uvmultiplier_blackPoint_whitePoint_triplanarPow.y + brightness * (uvmultiplier_blackPoint_whitePoint_triplanarPow.z - uvmultiplier_blackPoint_whitePoint_triplanarPow.y);

    // smoothstep the result if we are supposed to
    if (smoothStep)
        brightness = smoothstep(0.0f, 1.0f, brightness);

    // apply cross hatching
    if (crossHatch)
    {
        // get the color of the cross hatched texel at this location
        float3 pixelPos = rayPos + rayDir * rayHitInfo.m_intersectTime;
        float crossHatchTexel = SampleHatchingTexture(pixelPos, rayHitInfo.m_surfaceNormal, brightness, aniso, explicitCrossHatch);

        // apply crosshatching texture
		if (greyScale)
			light = crossHatchTexel;
		else
			light *= crossHatchTexel;
    }
	else if (greyScale)
	{
		light.xyz = brightness;
	}

    // do overlay opacity blending
    light = lerp(colorSample, light, overlayOpacity_yzw.x);

    // return sRGB corrected value
    return pow(light, 1.0f / 2.0f);
}

//----------------------------------------------------------------------------
float4 ps_main(SPixelInput input) : SV_TARGET
{
    float3 light = GetPixelColor(input, SBGrey, SBCrossHatch, SBSmoothStep, SBAniso, SBExplicitCrossHatch);
    return float4(light, 1.0f);
}
