#version 450
// Attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(push_constant) uniform PC {
    layout(offset = 0) mat4 transfo;
} pc;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_mod;

void main() 
{
    gl_Position = uCam.projView * pc.transfo * vec4(position, 1.0);
    out_position = position;
    out_normal = normal;
    vec4 mod4 = uCam.view * pc.transfo * vec4(normal, 1.0);
    out_mod = (mod4 / length(mod4)).xyz;
}