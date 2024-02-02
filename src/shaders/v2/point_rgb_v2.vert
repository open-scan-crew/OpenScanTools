// Vertex Shader for points version 2.0
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../blocks/block_hsv_functions.glsl"

// Vertex attributes
layout(location = 0) in uvec2 posXY;
layout(location = 1) in uint posZ;
layout(location = 3) in uvec4 color;

// Instanced attributes
layout(location = 5) in vec3 origin;
layout(location = 6) in float coordPrec;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PC {
    layout(offset = 0) float ptSize;
    layout(offset = 4) float transparency;
    layout(offset = 8) float contrast;
    layout(offset = 12) float brightness;
    layout(offset = 16) float saturation;
    layout(offset = 20) float luminance;
    layout(offset = 24) float blending;
    layout(offset = 28) float rampMin;
    layout(offset = 32) float rampMax;
    layout(offset = 36) int rampSteps;
    layout(offset = 48) vec3 ptColor;
} pc;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 projView;
} uCam;

layout(set = 0, binding = 1) uniform uniformModelScan{
    mat4 model;
} uScan;

void main() {
    gl_PointSize = pc.ptSize;
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
    vec3 hsl = rgb2hsl(color.rgb);
    hsl.z *= pc.luminance;
    hsl.y *= pc.saturation;
    fragColor = vec4(hsl2rgb(hsl) / 255.0, pc.transparency);
}