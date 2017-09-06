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
float3 GetPixelColor (SPixelInput input, bool greyScale, bool crossHatch)
{
    // calculate the ray for this pixel and get the time of the first ray hit
    float3 rayPos, rayDir;
    CalculateRay(float2(1.0f, 1.0f) - input.uv, rayPos, rayDir);
    SRayHitInfo rayHitInfo = ClosestIntersection(rayPos, rayDir);

	// TODO: make this a fallback plane (triangle probably or quad) so the background isn't curved! But, should be regardless of camera direction.
    // have a fallback sphere in the sky to catch anything missed
    if (rayHitInfo.m_intersectTime < 0.0f)
    {
        SpherePrim fallbackSphere;
        fallbackSphere.position_Radius.xyz = cameraPos_FOVX.xyz;
        fallbackSphere.position_Radius.w = 10.0f;
        RayIntersectsSphere(rayPos, rayDir, fallbackSphere, rayHitInfo);
		rayHitInfo.m_intersectTime /= uvmultiplier_yzw.x;
    }

    // get the lit value
    float3 light = pathTraceOutput.Sample(SamplerLinearWrap, float2(1.0f, 1.0f) - input.uv).xyz;

	// reinhard operator to convert from HDR to SDR
	light = light / (light + 1.0f);

	// convert SDR RGB to YUV. Y is brightness and UV is hue.
	float3 yuv = RGBToYUV(light);

	// convert full bright UV back to RGB to get the fully bright color of this pixel
	// TODO: honestly i think maybe the idea of full bright color is flawed. need to rethink this...
	// TODO: how do we get full bright color??
	//float3 fullBrightRGB = YUVToRGB(float3(0.5f, yuv.yz));

    // apply cross hatching
    if (crossHatch)
    {
        // sample the crosshatching with triplanar projection
        float3 pixelPos = rayPos + rayDir * rayHitInfo.m_intersectTime;

        float2 uvx = pixelPos.yz * uvmultiplier_yzw.x;
        float2 uvy = pixelPos.xz * uvmultiplier_yzw.x;
        float2 uvz = pixelPos.xy * uvmultiplier_yzw.x;

		// convert brightness to a w for a uvw coordinate.
		// This is necesary because values reside in the center of pixels, even in volume textures.
		// For instance, brightness 0 isn't the darkest value. brightess at pixel depth 0.5 is.
		uint volumeDimsX, volumeDimsY, volumeDimsZ;
		chvolume.GetDimensions(volumeDimsX, volumeDimsY, volumeDimsZ);
		float w = yuv.x * float(volumeDimsZ - 1) / float(volumeDimsZ) + 1.0f / float(volumeDimsZ * 2);
        
		// triplanar projection sample the crosshatching texture
        float crossHatchTexel =
            chvolume.Sample(SamplerLinearWrap, float3(uvx, w)).r * rayHitInfo.m_surfaceNormal.x +
            chvolume.Sample(SamplerLinearWrap, float3(uvy, w)).r * rayHitInfo.m_surfaceNormal.y +
            chvolume.Sample(SamplerLinearWrap, float3(uvz, w)).r * rayHitInfo.m_surfaceNormal.z;
        crossHatchTexel = crossHatchTexel / (rayHitInfo.m_surfaceNormal.x + rayHitInfo.m_surfaceNormal.y + rayHitInfo.m_surfaceNormal.z);

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