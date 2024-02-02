// Vertext Shader for points with "intensity" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable
#define ATTRIB_I
#include "./blocks/block_point_input_vert.glsl"

#include "./blocks/block_point_i_main_standard_vert.glsl"