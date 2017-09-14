#pragma once

#include "Utils.h"

// settings
#define c_width 1024
#define c_height 768
#define c_fullScreen false
#define c_vsync false
#define c_shaderDebug false
#define c_d3ddebug false // TODO: turn these off
#define c_fovX DegreesToRadians(40.0f)
#define c_fovY (c_fovX * float(c_height) / float(c_width)) 
#define c_IMGUI_Verts 100000
