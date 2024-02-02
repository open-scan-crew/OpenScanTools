#version 440

// Vertex attributes
layout(location = 0) in vec2 vertexPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(vertexPos, 0.0, 1.0);
}