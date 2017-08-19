cbuffer Constants
{
  float4 pixelColor;
};

struct Triangle
{
  float3 position;
};

struct SBufferItem
{
  float4 c;
};

struct PosColorUV
{
  float4 position : POSITION0;
  float4 color : COLOR0;
  float2 uv : TEXCOORD0;
};

Texture2D stone;
RWTexture2D<float4> stone_rw;

Texture2D rwtexture;
RWTexture2D<float4> rwtexture_rw;

StructuredBuffer<Triangle> Triangles;

StructuredBuffer<SBufferItem> Input;

