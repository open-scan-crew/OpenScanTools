// Vertext Shader for points with "rgb" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable
#define ATTRIB_RGB
#define RAMP_FLAT
#define RAMP_DIST

#include "./blocks/block_point_input_vert.glsl"

#include "./blocks/block_hsv_functions.glsl"

#include "./blocks/block_point_main_ramp_vert.glsl"