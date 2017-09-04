//This file is autogenerated by WriteShaderTypesHLSL(), using ShaderTypesList.h as source data

//----------------------------------------------------------------------------
//Samplers
//----------------------------------------------------------------------------
SamplerState SamplerLinearWrap;
SamplerState SamplerNearestWrap;

//----------------------------------------------------------------------------
//Textures
//----------------------------------------------------------------------------
Texture2D blueNoise256;
RWTexture2D<float4> blueNoise256_rw;

Texture2D pathTraceOutput;
RWTexture2D<float4> pathTraceOutput_rw;

//----------------------------------------------------------------------------
//Constant Buffers
//----------------------------------------------------------------------------
cbuffer ConstantsOnce
{
  float4 cameraPos_FOVX;
  float4 cameraAt_FOVY;
  float4 nearPlaneDist_missColor;
  uint4 numSpheres_numTris_numOBBs_numQuads;
};

cbuffer ConstantsPerFrame
{
  float4 frameRnd_appTime_zw;
  uint4 sampleCount_yzw;
};

//----------------------------------------------------------------------------
//Vertex Formats
//----------------------------------------------------------------------------
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
  float4 albedo_w;
  float4 emissive_w;
};

struct TrianglePrim
{
  float4 positionA_w;
  float4 positionB_w;
  float4 positionC_w;
  float4 normal_w;
  float4 albedo_w;
  float4 emissive_w;
};

struct QuadPrim
{
  float4 positionA_w;
  float4 positionB_w;
  float4 positionC_w;
  float4 positionD_w;
  float4 normal_w;
  float4 albedo_w;
  float4 emissive_w;
};

struct OBBPrim
{
  float4 position_w;
  float4 radius_w;
  float4 XAxis_w;
  float4 YAxis_w;
  float4 ZAxis_w;
  float4 albedo_w;
  float4 emissive_w;
};

//----------------------------------------------------------------------------
//Structured Buffers
//----------------------------------------------------------------------------
StructuredBuffer<SpherePrim> Spheres;

StructuredBuffer<TrianglePrim> Triangles;

StructuredBuffer<QuadPrim> Quads;

StructuredBuffer<OBBPrim> OBBs;

