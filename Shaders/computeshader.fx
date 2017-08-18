#include "ShaderTypes.h"

////////////////////////////////////////////////////////////////////////////////
// Compute Shader
////////////////////////////////////////////////////////////////////////////////

RWTexture2D<float4> Output : register(u0);

[numthreads(32, 32, 1)]
void cs_main (uint3 threadID : SV_DispatchThreadID)
{
    // GetDimensions() can tell you how many items there are in Triangles etc!

    float4 incolor = float4(Input[0].c[0], Input[1].c[0], Input[0].c[2], Input[0].c[3]);
    incolor.rgb *= Triangles[1].position;

    Output[threadID.xy] = float4(frac(threadID.xy / 100.0f), 0, 1) * incolor * pixelColor;
}
