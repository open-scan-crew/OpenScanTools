// Vertext Shader for points with "intensity" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable
#define _CLIPPING_ACTIVATED
#include "./blocks/block_point_input_vert.glsl"

#include "./blocks/block_point_main_flat_vert.glsl"