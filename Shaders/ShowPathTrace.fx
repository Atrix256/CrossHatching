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
// TODO: put these color conversion functions in a header
// TODO: may not need these in the end
// from https://www.pcmag.com/encyclopedia/term/55166/yuv-rgb-conversion-formulas
float3 RGBToYUV (float3 rgb)
{
	float3 yuv;
	yuv.x = dot(rgb, float3(0.299f, 0.587f, 0.114f));
	yuv.y = 0.492f * (rgb.b - yuv.x);
	yuv.z = 0.877f * (rgb.r - yuv.x);
	return yuv;
}

float3 YUVToRGB (float3 yuv)
{
	float3 rgb;
	rgb.r = yuv.x + 1.140f*yuv.z;
	rgb.g = yuv.x - 0.395f*yuv.y - 0.581f*yuv.z;
	rgb.b = yuv.x + 2.032f*yuv.y;
	return clamp(rgb, 0.0f, 1.0f);
}

//----------------------------------------------------------------------------
float3 GetPixelColor (SPixelInput input, bool greyScale, bool crossHatch, bool smoothStep, bool aniso)
{
    // get our first ray hit info from the FirstRayHits buffer
    uint dimsX, dimsY;
    pathTraceOutput.GetDimensions(dimsX, dimsY);
    uint2 pixelPos = uint2((float2(1.0f, 1.0f) - input.uv) * float2(dimsX, dimsY));
    uint pixelIndex = pixelPos.y * dimsX + pixelPos.x;
    SRayHitInfo rayHitInfo;
    rayHitInfo.m_surfaceNormal = FirstRayHits[pixelIndex].surfaceNormal_intersectTime.xyz;
    rayHitInfo.m_intersectTime = FirstRayHits[pixelIndex].surfaceNormal_intersectTime.w;
    rayHitInfo.m_albedo = FirstRayHits[pixelIndex].albedo_w.xyz;
    rayHitInfo.m_emissive = FirstRayHits[pixelIndex].emissive_w.xyz;

    // calculate the ray for this pixel and get the time of the first ray hit
    float3 rayPos, rayDir;
    CalculateRay(float2(1.0f, 1.0f) - input.uv, rayPos, rayDir);

    // get the lit value
    float3 light = pathTraceOutput.Sample(SamplerLinearWrap, float2(1.0f, 1.0f) - input.uv).xyz;

    // if the ray didn't hit anything, fake a planar hit for shading purposes
    if (rayHitInfo.m_intersectTime < 0.0f)
    {
        rayHitInfo.m_intersectTime = 2.0f / uvmultiplier_blackPoint_whitePoint_w.x;
        rayHitInfo.m_surfaceNormal = normalize(cameraPos_FOVX.xyz - cameraAt_FOVY.xyz);
        rayHitInfo.m_albedo = float3(0.0f, 0.0f, 0.0f);
        rayHitInfo.m_emissive = float3(0.0f, 0.0f, 0.0f);
        light = nearPlaneDist_missColor.yzw;
    }

	// reinhard operator to convert from HDR to SDR
	light = light / (light + 1.0f);

	// convert SDR RGB to YUV. Y is brightness and UV is hue.
	float3 yuv = RGBToYUV(light);

    // remap the brightness using the black point / white point
    yuv.x = uvmultiplier_blackPoint_whitePoint_w.y + yuv.x * (uvmultiplier_blackPoint_whitePoint_w.z - uvmultiplier_blackPoint_whitePoint_w.y);

    // smoothstep the result if we are supposed to
    if (smoothStep)
        yuv.x = smoothstep(0.0f, 1.0f, yuv.x);

	// convert full bright UV back to RGB to get the fully bright color of this pixel
	// TODO: honestly i think maybe the idea of full bright color is flawed. need to rethink this...
	// TODO: how do we get full bright color??
	//float3 fullBrightRGB = YUVToRGB(float3(0.5f, yuv.yz));

    // apply cross hatching
    if (crossHatch)
    {
        // sample the crosshatching with triplanar projection
        float3 pixelPos = rayPos + rayDir * rayHitInfo.m_intersectTime;

        float2 uvx = pixelPos.yz * uvmultiplier_blackPoint_whitePoint_w.x;
        float2 uvy = pixelPos.xz * uvmultiplier_blackPoint_whitePoint_w.x;
        float2 uvz = pixelPos.xy * uvmultiplier_blackPoint_whitePoint_w.x;

        // caclulate the array slice to read
		uint volumeDimsX, volumeDimsY, volumeDimsZ;
        circlesarray.GetDimensions(volumeDimsX, volumeDimsY, volumeDimsZ);
        float w = yuv.x * float(volumeDimsZ);
        
		// triplanar projection sample the crosshatching texture
        // we need to manually lerp to get trilinear interpolation between slice samples
        float crossHatchTexelFloor;
        float crossHatchTexelCeil;
        if (aniso)
        {
            crossHatchTexelFloor =
                circlesarray.Sample(SamplerAnisoWrap, float3(uvx, floor(w))).r * rayHitInfo.m_surfaceNormal.x +
                circlesarray.Sample(SamplerAnisoWrap, float3(uvy, floor(w))).r * rayHitInfo.m_surfaceNormal.y +
                circlesarray.Sample(SamplerAnisoWrap, float3(uvz, floor(w))).r * rayHitInfo.m_surfaceNormal.z;
            
            crossHatchTexelCeil =
                circlesarray.Sample(SamplerAnisoWrap, float3(uvx, ceil(w))).r * rayHitInfo.m_surfaceNormal.x +
                circlesarray.Sample(SamplerAnisoWrap, float3(uvy, ceil(w))).r * rayHitInfo.m_surfaceNormal.y +
                circlesarray.Sample(SamplerAnisoWrap, float3(uvz, ceil(w))).r * rayHitInfo.m_surfaceNormal.z;
        }
        else
        {
            crossHatchTexelFloor =
                circlesarray.Sample(SamplerLinearWrap, float3(uvx, floor(w))).r * rayHitInfo.m_surfaceNormal.x +
                circlesarray.Sample(SamplerLinearWrap, float3(uvy, floor(w))).r * rayHitInfo.m_surfaceNormal.y +
                circlesarray.Sample(SamplerLinearWrap, float3(uvz, floor(w))).r * rayHitInfo.m_surfaceNormal.z;

            crossHatchTexelCeil =
                circlesarray.Sample(SamplerLinearWrap, float3(uvx, ceil(w))).r * rayHitInfo.m_surfaceNormal.x +
                circlesarray.Sample(SamplerLinearWrap, float3(uvy, ceil(w))).r * rayHitInfo.m_surfaceNormal.y +
                circlesarray.Sample(SamplerLinearWrap, float3(uvz, ceil(w))).r * rayHitInfo.m_surfaceNormal.z;
        }
        crossHatchTexelFloor /= (rayHitInfo.m_surfaceNormal.x + rayHitInfo.m_surfaceNormal.y + rayHitInfo.m_surfaceNormal.z);
        crossHatchTexelCeil /= (rayHitInfo.m_surfaceNormal.x + rayHitInfo.m_surfaceNormal.y + rayHitInfo.m_surfaceNormal.z);

        float crossHatchTexel = lerp(crossHatchTexelFloor, crossHatchTexelCeil, frac(w));

        // apply crosshatching texture
		if (greyScale)
			light = crossHatchTexel;
		else
			light *= crossHatchTexel;
    }
	else if (greyScale)
	{
		light.xyz = yuv.x;
	}

    // return sRGB corrected value
    return pow(light, 1.0f / 2.0f);
}

//----------------------------------------------------------------------------
float4 ps_main(SPixelInput input) : SV_TARGET
{
    float3 light = GetPixelColor(input, SBGrey, SBCrossHatch, SBSmoothStep, SBAniso);
    return float4(light, 1.0f);
}
