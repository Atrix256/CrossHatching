/*

These types are macro expanded to make C++ side structs.
They also generate shader code at runtime that the shaders use.

===================================================================
                      Constant Buffers
===================================================================

CONSTANT_BUFFER_BEGIN(Name) : starts a constant buffer definition
  Name - the name of the constant buffer to begin

CONSTANT_BUFFER_FIELD(Name, Type) : defines a field
  Name - the name of the field
  Type - the type of the field

CONSTANT_BUFFER_END() : ends a constant buffer definition

===================================================================
                      Structured Buffers
===================================================================

STRUCTURED_BUFFER_BEGIN(Name, TypeName, Count, CPUWrites) : starts a structured buffer definition
  Name      - the name of the structured buffer to begin
  TypeName  - the name of the individual struct type
  Count     - how many items there are in the buffer
  CPUWrites - if true, written by CPU, read by GPU. If false, written/read by GPU

STRUCTURED_BUFFER_FIELD(Name, Type) : defines a field
  Name - the name of the field
  Type - the type of the field

STRUCTURED_BUFFER_END() : ends a structured buffer definition

===================================================================
                       Vertex Formats
===================================================================

VERTEX_FORMAT_BEGIN(Name)
  Name - the name of the vertex format in both c++ and shader code

VERTEX_FORMAT_FIELD(Name, Semantic, Index, Type, Format)
  Name - the name of the field
  Semantic - the shader semantic
  Index - the semantic index
  Type - the data type
  Format - the format

VERTEX_FORMAT_END

===================================================================
                          Textures
===================================================================

TEXTURE_IMAGE(Name, FileName)
  Name     - The name of the texture as it appears in C++ and shader code
  FileName - The file name of the texture to load. If nullptr, will create a read write texture.

TEXTURE_BUFFER(Name, ShaderType, Format)
  Name       - The name of the texture as it appears in C++ and shader code
  ShaderType - as declared in shader, such as float or float4
  Format     - d3d image format, like DXGI_FORMAT_R32_FLOAT or DXGI_FORMAT_R8G8B8A8_UNORM

TEXTURE_VOLUME_BEGIN(Name)
  Name       - The name of the volume texture as it appears in C++ and in the shader

TEXTURE_VOLUME_SLICE(Texture)
  Texture    - The name of the texture to use for a slice of the volume texture

TEXTURE_VOLUME_END

TEXTURE_ARRAY_BEGIN(Name)
  Name       - The name of the texture array as it appears in C++ and in the shader

TEXTURE_ARRAY_SLICE(Texture)
  Texture    - The name of the texture to use for a slice of the volume texture

TEXTURE_ARRAY_END

===================================================================
                          Shaders
===================================================================

SHADER_VSPS(Name, FileName, VSEntry, PSEntry, VertexFormat)
  Name         - the name of the shader
  FileName     - the file name of the shader
  VSEntry      - the name of the entry point for the vertex shader
  PSEntry      - the name of the entry point for the pixel shader
  VertexFormat - the name of the vertex format to use

SHADER_CS(Name, FileName, Entry)
  Name     - the name of the shader
  FileName - the file name of the shader
  VSEntry  - the name of the entry point for the compute shader

*/

#include "Settings.h"

//=================================================================
// define anything the caller didn't define for convenience

#ifndef CONSTANT_BUFFER_BEGIN
#define CONSTANT_BUFFER_BEGIN(NAME)
#endif

#ifndef CONSTANT_BUFFER_FIELD
#define CONSTANT_BUFFER_FIELD(NAME, TYPE)
#endif

#ifndef CONSTANT_BUFFER_END
#define CONSTANT_BUFFER_END
#endif

#ifndef STRUCTURED_BUFFER_BEGIN
#define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT, CPUWRITES)
#endif

#ifndef STRUCTURED_BUFFER_FIELD
#define STRUCTURED_BUFFER_FIELD(NAME, TYPE)
#endif

#ifndef STRUCTURED_BUFFER_END
#define STRUCTURED_BUFFER_END
#endif

#ifndef VERTEX_FORMAT_BEGIN
#define VERTEX_FORMAT_BEGIN(NAME)
#endif

#ifndef VERTEX_FORMAT_FIELD
#define VERTEX_FORMAT_FIELD(NAME, SEMANTIC, INDEX, TYPE, FORMAT)
#endif

#ifndef VERTEX_FORMAT_END
#define VERTEX_FORMAT_END
#endif

#ifndef TEXTURE_IMAGE
#define TEXTURE_IMAGE(NAME, FILENAME)
#endif

#ifndef TEXTURE_BUFFER
#define TEXTURE_BUFFER(NAME, SHADERTYPE, FORMAT)
#endif

#ifndef TEXTURE_VOLUME_BEGIN
#define TEXTURE_VOLUME_BEGIN(NAME)
#endif

#ifndef TEXTURE_VOLUME_SLICE
#define TEXTURE_VOLUME_SLICE(TEXTURE)
#endif

#ifndef TEXTURE_VOLUME_END
#define TEXTURE_VOLUME_END
#endif

#ifndef TEXTURE_ARRAY_BEGIN
#define TEXTURE_ARRAY_BEGIN(NAME)
#endif

#ifndef TEXTURE_ARRAY_SLICE
#define TEXTURE_ARRAY_SLICE(TEXTURE)
#endif

#ifndef TEXTURE_ARRAY_END
#define TEXTURE_ARRAY_END
#endif

#ifndef SHADER_VSPS
#define SHADER_VSPS(NAME, FILENAME, VSENTRY, PSENTRY, VERTEXFORMAT)
#endif

#ifndef SHADER_CS
#define SHADER_CS(NAME, FILENAME, ENTRY)
#endif

//=================================================================
//                     Constant Buffers
//=================================================================

CONSTANT_BUFFER_BEGIN(ConstantsOnce)
    CONSTANT_BUFFER_FIELD(cameraPos_FOVX, float4)
    CONSTANT_BUFFER_FIELD(cameraAt_FOVY, float4)
    CONSTANT_BUFFER_FIELD(nearPlaneDist_missColor, float4)
    CONSTANT_BUFFER_FIELD(numSpheres_numTris_numOBBs_numQuads, uint4)
	CONSTANT_BUFFER_FIELD(uvmultiplier_blackPoint_whitePoint_w, float4)
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(ConstantsPerFrame)
    CONSTANT_BUFFER_FIELD(frameRnd_appTime_zw, float4)
    CONSTANT_BUFFER_FIELD(sampleCount_yzw, uint4)
CONSTANT_BUFFER_END

//=================================================================
//                    Structured Buffers
//=================================================================

STRUCTURED_BUFFER_BEGIN(Spheres, SpherePrim, 10, true)
    STRUCTURED_BUFFER_FIELD(position_Radius, float4)
    STRUCTURED_BUFFER_FIELD(albedo_w, float4)
    STRUCTURED_BUFFER_FIELD(emissive_w, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(Triangles, TrianglePrim, 1000, true)
    STRUCTURED_BUFFER_FIELD(positionA_w, float4)
    STRUCTURED_BUFFER_FIELD(positionB_w, float4)
    STRUCTURED_BUFFER_FIELD(positionC_w, float4)
    STRUCTURED_BUFFER_FIELD(normal_w, float4)
    STRUCTURED_BUFFER_FIELD(albedo_w, float4)
    STRUCTURED_BUFFER_FIELD(emissive_w, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(Quads, QuadPrim, 10, true)
    STRUCTURED_BUFFER_FIELD(positionA_w, float4)
    STRUCTURED_BUFFER_FIELD(positionB_w, float4)
    STRUCTURED_BUFFER_FIELD(positionC_w, float4)
    STRUCTURED_BUFFER_FIELD(positionD_w, float4)
    STRUCTURED_BUFFER_FIELD(normal_w, float4)
    STRUCTURED_BUFFER_FIELD(albedo_w, float4)
    STRUCTURED_BUFFER_FIELD(emissive_w, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(OBBs, OBBPrim, 10, true)
    STRUCTURED_BUFFER_FIELD(position_w, float4)
    STRUCTURED_BUFFER_FIELD(radius_w, float4)
    STRUCTURED_BUFFER_FIELD(XAxis_w, float4)
    STRUCTURED_BUFFER_FIELD(YAxis_w, float4)
    STRUCTURED_BUFFER_FIELD(ZAxis_w, float4)
    STRUCTURED_BUFFER_FIELD(albedo_w, float4)
    STRUCTURED_BUFFER_FIELD(emissive_w, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(FirstRayHits, FirstRayHit, c_width * c_height, false)
    STRUCTURED_BUFFER_FIELD(surfaceNormal_intersectTime, float4)
    STRUCTURED_BUFFER_FIELD(albedo_w, float4)
    STRUCTURED_BUFFER_FIELD(emissive_w, float4)
STRUCTURED_BUFFER_END

//=================================================================
//                     Vertex Formats
//=================================================================

VERTEX_FORMAT_BEGIN(Pos2D)
    VERTEX_FORMAT_FIELD(position, POSITION, 0, float4, DXGI_FORMAT_R32G32B32A32_FLOAT)
VERTEX_FORMAT_END

//=================================================================
//                       Textures
//=================================================================

TEXTURE_BUFFER(pathTraceOutput, float4, DXGI_FORMAT_R32G32B32A32_FLOAT)

TEXTURE_IMAGE(blueNoise256, "Art/BlueNoise256.tga")

TEXTURE_IMAGE(circleneg4, "Art/circlesneg4.tga")
TEXTURE_IMAGE(circleneg3, "Art/circlesneg3.tga")
TEXTURE_IMAGE(circleneg2, "Art/circlesneg2.tga")
TEXTURE_IMAGE(circleneg1, "Art/circlesneg1.tga")
TEXTURE_IMAGE(circles0, "Art/circles0.tga")
TEXTURE_IMAGE(circles1, "Art/circles1.tga")
TEXTURE_IMAGE(circles2, "Art/circles2.tga")
TEXTURE_IMAGE(circles3, "Art/circles3.tga")
TEXTURE_IMAGE(circles4, "Art/circles4.tga")

TEXTURE_ARRAY_BEGIN(circlesarray)
    TEXTURE_ARRAY_SLICE(circles4)
    TEXTURE_ARRAY_SLICE(circles3)
    TEXTURE_ARRAY_SLICE(circles2)
    TEXTURE_ARRAY_SLICE(circles1)
    TEXTURE_ARRAY_SLICE(circles0)
    TEXTURE_ARRAY_SLICE(circleneg1)
    TEXTURE_ARRAY_SLICE(circleneg2)
    TEXTURE_ARRAY_SLICE(circleneg3)
    TEXTURE_ARRAY_SLICE(circleneg4)
TEXTURE_ARRAY_END

//=================================================================
//                       Shaders
//=================================================================

SHADER_CS(pathTrace, L"Shaders/PathTrace.fx", "cs_main")
SHADER_CS(pathTraceFirstHit, L"Shaders/PathTrace.fx", "cs_main_FirstHit")

SHADER_VSPS(showPathTrace_Color_Shade_SmoothStep, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_color_shade_smoothstep", Pos2D)
SHADER_VSPS(showPathTrace_Grey_Shade_SmoothStep, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_grey_shade_smoothstep", Pos2D)
SHADER_VSPS(showPathTrace_Color_CrossHatch_SmoothStep, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_color_crosshatch_smoothstep", Pos2D)
SHADER_VSPS(showPathTrace_Grey_CrossHatch_SmoothStep, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_grey_crosshatch_smoothstep", Pos2D)

SHADER_VSPS(showPathTrace_Color_Shade_No, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_color_shade_no", Pos2D)
SHADER_VSPS(showPathTrace_Grey_Shade_No, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_grey_shade_no", Pos2D)
SHADER_VSPS(showPathTrace_Color_CrossHatch_No, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_color_crosshatch_no", Pos2D)
SHADER_VSPS(showPathTrace_Grey_CrossHatch_No, L"Shaders/ShowPathTrace.fx", "vs_main", "ps_main_grey_crosshatch_no", Pos2D)


//=================================================================
// undefine everything for the caller's convenience
#undef CONSTANT_BUFFER_BEGIN
#undef CONSTANT_BUFFER_FIELD
#undef CONSTANT_BUFFER_END
#undef STRUCTURED_BUFFER_BEGIN
#undef STRUCTURED_BUFFER_FIELD
#undef STRUCTURED_BUFFER_END
#undef VERTEX_FORMAT_BEGIN
#undef VERTEX_FORMAT_FIELD
#undef VERTEX_FORMAT_END
#undef TEXTURE_IMAGE
#undef TEXTURE_BUFFER
#undef TEXTURE_VOLUME_BEGIN
#undef TEXTURE_VOLUME_SLICE
#undef TEXTURE_VOLUME_END
#undef TEXTURE_ARRAY_BEGIN
#undef TEXTURE_ARRAY_SLICE
#undef TEXTURE_ARRAY_END
#undef SHADER_VSPS
#undef SHADER_CS