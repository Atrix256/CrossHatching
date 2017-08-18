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

StructuredBuffer<Triangle>Triangles;

StructuredBuffer<SBufferItem>Input;

