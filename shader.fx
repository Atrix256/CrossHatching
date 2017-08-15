/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    float4 pixelColor;
};

Texture2D shaderTexture;
SamplerState SampleType;

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType vs_main (VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;
    output.position = input.position;
    output.position.w = 2.0f;

    output.color = input.color * pixelColor;

    output.uv = input.uv;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ps_main (PixelInputType input) : SV_TARGET
{
    return input.color * shaderTexture.Sample(SampleType, input.uv);
}