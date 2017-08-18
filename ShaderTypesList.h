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
                          TEXTURES
===================================================================

TEXTURE(Name, FileName)
  Name     - The name of the texture as it appears in C++ and shader code
  FileName - The file name of the texture to load. If nullptr, will create a read write texture.

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

#ifndef TEXTURE
#define TEXTURE(NAME, FILENAME)
#endif

//=================================================================
//                     Constant Buffers
//=================================================================

CONSTANT_BUFFER_BEGIN(Constants)
    CONSTANT_BUFFER_FIELD(pixelColor, float4)
CONSTANT_BUFFER_END

//=================================================================
//                    Structured Buffers
//=================================================================

STRUCTURED_BUFFER_BEGIN(Triangles, Triangle, 10)
    STRUCTURED_BUFFER_FIELD(position, float3)
STRUCTURED_BUFFER_END

STRUCTURED_BUFFER_BEGIN(Input, SBufferItem, 1)
    STRUCTURED_BUFFER_FIELD(c, float4)
STRUCTURED_BUFFER_END

//=================================================================
//                       TEXTURES
//=================================================================

TEXTURE(stone, "stone01.tga")
TEXTURE(rwtexture, nullptr)

//=================================================================
// undefine everything for the caller's convenience
#undef CONSTANT_BUFFER_BEGIN
#undef CONSTANT_BUFFER_FIELD
#undef CONSTANT_BUFFER_END
#undef STRUCTURED_BUFFER_BEGIN
#undef STRUCTURED_BUFFER_FIELD
#undef STRUCTURED_BUFFER_END
#undef TEXTURE