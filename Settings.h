#pragma once

#include "Utils.h"

// settings
#define c_width 800
#define c_height 600
#define c_fullScreen false
#define c_vsync false
#define c_shaderDebug true
#define c_d3ddebug true // TODO: turn these off
#define c_fovX DegreesToRadians(40.0f)
#define c_fovY (c_fovX * float(c_height) / float(c_width)) 
