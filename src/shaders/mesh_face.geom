#version 450

layout (triangles, invocations = 3) in;
layout (triangle_strip, max_vertices = 3) out;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(location = 0) out vec3 fragNormal;

void main() {
    vec4 AB = uCam.view * (gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec4 AC = uCam.view * (gl_in[2].gl_Position - gl_in[0].gl_Position);
    vec3 N = cross(AB.xyz, AC.xyz);
    N /= length(N);

    gl_Position = uCam.projView * gl_in[0].gl_Position;
    fragNormal = N;
    EmitVertex();

    gl_Position = uCam.projView * gl_in[1].gl_Position;
    fragNormal = N;
    EmitVertex();

    gl_Position = uCam.projView * gl_in[2].gl_Position;
    fragNormal = N;
    EmitVertex();
    EndPrimitive();
}