#version 450
// Attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(push_constant) uniform PC {
    layout(offset = 0) mat4 transfo;
} pc;


layout(location = 0) out vec3 out_normal;

void main() 
{
    gl_Position = uCam.projView * pc.transfo * vec4(inPosition, 1.0);
    vec4 normal = uCam.view * pc.transfo * vec4(inNormal, 0.0);
    out_normal = (normal.xyz / length(normal));
}