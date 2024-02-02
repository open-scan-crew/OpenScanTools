// Vertext Shader for points with "intensity" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable

#define ATTRIB_I
#define _CLIPPING_ACTIVATED
#include "./blocks/block_point_input_vert.glsl"

#include "./blocks/block_hsv_functions.glsl"

#include "./blocks/block_point_i_main_fake_color_vert.glsl"