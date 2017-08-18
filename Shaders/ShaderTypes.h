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

Texture2D stone;

Texture2D rwtexture;

StructuredBuffer<Triangle> Triangles;

StructuredBuffer<SBufferItem> Input;

