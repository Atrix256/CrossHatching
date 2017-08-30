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

STRUCTURED_BUFFER_BEGIN(Name, TypeName, Count) : starts a structured buffer definition
  Name     - the name of the structured buffer to begin
  TypeName - the name of the individual struct type
  Count    - how many items there are in the buffer

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

*/

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
#define STRUCTURED_BUFFER_BEGIN(NAME, TYPENAME, COUNT)
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

//=================================================================
//                     Constant Buffers
//=================================================================

CONSTANT_BUFFER_BEGIN(ConstantsOnce)
    CONSTANT_BUFFER_FIELD(cameraPos_FOVX, float4)
    CONSTANT_BUFFER_FIELD(cameraAt_FOVY, float4)
    CONSTANT_BUFFER_FIELD(nearPlaneDist_missColor_zw, float4)
    CONSTANT_BUFFER_FIELD(numSpheres_numTris_numOBBs_numQuads, float4)
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(ConstantsPerFrame)
    CONSTANT_BUFFER_FIELD(frameRnd_appTime_zw, float4)
    CONSTANT_BUFFER_FIELD(sampleCount_yzw, uint4)
CONSTANT_BUFFER_END

//=================================================================
//                    Structured Buffers
//=================================================================

STRUCTURED_BUFFER_BEGIN(Spheres, SpherePrim, 10)
    STRUCTURED_BUFFER_FIELD(position_Radius, float4)
    STRUCTURED_BUFFER_FIELD(albedo_Emissive_zw, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(Triangles, TrianglePrim, 10)
    STRUCTURED_BUFFER_FIELD(positionA_Albedo, float4)
    STRUCTURED_BUFFER_FIELD(positionB_Emissive, float4)
    STRUCTURED_BUFFER_FIELD(positionC_w, float4)
    STRUCTURED_BUFFER_FIELD(normal_w, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(Quads, QuadPrim, 10)
    STRUCTURED_BUFFER_FIELD(positionA_Albedo, float4)
    STRUCTURED_BUFFER_FIELD(positionB_Emissive, float4)
    STRUCTURED_BUFFER_FIELD(positionC_w, float4)
    STRUCTURED_BUFFER_FIELD(positionD_w, float4)
    STRUCTURED_BUFFER_FIELD(normal_w, float4)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(OBBs, OBBPrim, 10)
    STRUCTURED_BUFFER_FIELD(position_Albedo, float4)
    STRUCTURED_BUFFER_FIELD(radius_Emissive, float4)
    STRUCTURED_BUFFER_FIELD(XAxis_w, float4)
    STRUCTURED_BUFFER_FIELD(YAxis_w, float4)
    STRUCTURED_BUFFER_FIELD(ZAxis_w, float4)
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

TEXTURE_IMAGE(blueNoise256, "Art/BlueNoise256.tga")
TEXTURE_BUFFER(pathTraceOutput, float, DXGI_FORMAT_R32_FLOAT)

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