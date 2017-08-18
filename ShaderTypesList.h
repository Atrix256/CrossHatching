/*=================================================================

These types are macro expanded to make C++ side structs.
They also generate shader code at runtime that the shaders use.

===================================================================

BUFFER_FIELD(Name, Type) : defines a field
  Name - the name of the field
  Type - the type of the field

===================================================================

CONSTANT_BUFFER_BEGIN(Name) : starts a constant buffer definition
  Name - the type of the constant buffer to begin

CONSTANT_BUFFER_END() : ends a constant buffer definition

===================================================================
*/

//=================================================================
// define anything the caller didn't define
#ifndef BUFFER_FIELD
#define BUFFER_FIELD(NAME, TYPE)
#endif

#ifndef CONSTANT_BUFFER_BEGIN
#define CONSTANT_BUFFER_BEGIN(NAME)
#endif

#ifndef CONSTANT_BUFFER_END
#define CONSTANT_BUFFER_END
#endif

//=================================================================
// The types

CONSTANT_BUFFER_BEGIN(Constants)
  BUFFER_FIELD(pixelColor, float4)
CONSTANT_BUFFER_END

//=================================================================
// undefine everything for the caller's convinience
#undef BUFFER_FIELD
#undef CONSTANT_BUFFER_BEGIN
#undef CONSTANT_BUFFER_END