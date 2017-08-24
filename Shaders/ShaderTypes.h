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

cbuffer Scene
{
  float4 cameraPos_FOVX;
  float4 cameraAt_FOVY;
  float4 numSpheres_numTris_nearPlaneDist_missColor;
  float4 frameRnd_appTime_sampleCount_numQuads;
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
struct SpherePrim
{
  float4 position_Radius;
  float4 albedo_Emissive_zw;
};

struct TrianglePrim
{
  float4 positionA_Albedo;
  float4 positionB_Emissive;
  float4 positionC_w;
  float4 normal_w;
};

struct QuadPrim
{
  float4 positionA_Albedo;
  float4 positionB_Emissive;
  float4 positionC_w;
  float4 positionD_w;
  float4 normal_w;
};

struct SBufferItem
{
  float4 c;
};

//----------------------------------------------------------------------------
//Structured Buffers
//----------------------------------------------------------------------------
StructuredBuffer<SpherePrim> Spheres;

StructuredBuffer<TrianglePrim> Triangles;

StructuredBuffer<QuadPrim> Quads;

StructuredBuffer<SBufferItem> Input;

