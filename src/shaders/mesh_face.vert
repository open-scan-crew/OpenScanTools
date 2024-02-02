#version 450
// Attributes
layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PC {
    layout(offset = 0) mat4 transfo;
} pc;

void main() 
{
    gl_Position = pc.transfo * vec4(inPosition, 1.0);
}