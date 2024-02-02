#version 440

// Vertex attributes
layout(location = 0) in vec2 vertexPos;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PC {
    vec4 color;
    int width;
    int height;
} pcst;


void main() {
    gl_Position = vec4((vertexPos.x / pcst.width - 0.5) * 2.0, (vertexPos.y / pcst.height - 0.5) * 2.0, 0.0, 1.0);
    fragColor = pcst.color;
}