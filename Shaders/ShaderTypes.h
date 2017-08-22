//This file is autogenerated by WriteShaderTypesHLSL(), using ShaderTypesList.h as source data

//----------------------------------------------------------------------------
//Samplers
//----------------------------------------------------------------------------
SamplerState SamplerLinearWrap;

//----------------------------------------------------------------------------
//Textures
//----------------------------------------------------------------------------
Texture2D stone;
RWTexture2D<float4> stone_rw;

Texture2D rwtexture;
RWTexture2D<float4> rwtexture_rw;

Texture2D pathTraceOutput;
RWTexture2D<float4> pathTraceOutput_rw;

//----------------------------------------------------------------------------
//Constant Buffers
//----------------------------------------------------------------------------
cbuffer Constants
{
  float4 pixelColor;
};

//----------------------------------------------------------------------------
//Vertex Formats
//----------------------------------------------------------------------------
struct PosColorUV
{
  float3 position : POSITION0;
  float4 color : COLOR0;
  float2 uv : TEXCOORD0;
};

struct Pos2D
{
  float4 position : POSITION0;
};

//----------------------------------------------------------------------------
//Structured Buffer Types
//----------------------------------------------------------------------------
struct Triangle
{
  float3 position;
};

struct SBufferItem
{
  float4 c;
};

//----------------------------------------------------------------------------
//Structured Buffers
//----------------------------------------------------------------------------
StructuredBuffer<Triangle> Triangles;

StructuredBuffer<SBufferItem> Input;

