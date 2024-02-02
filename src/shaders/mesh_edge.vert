#version 450
// Attributes
layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(push_constant) uniform PC {
    layout(offset = 0) mat4 transfo;
} pc;

void main() 
{
    gl_Position = uCam.projView * pc.transfo * vec4(inPosition, 1.0);
}
