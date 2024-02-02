// Vertex Shader //
// Transform the 2D vertex in view space then project it in real size

#version 450
// Vertex attributes
layout(location = 0) in vec3 markerPos;
layout(location = 1) in uvec4 markerColor;
layout(location = 2) in uint graphicID;
layout(location = 3) in uint textureID;
layout(location = 4) in uint firstVertex;
layout(location = 5) in uint vertexCount;
layout(location = 6) in uint style;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 1) out vec4 geomColor;
layout(location = 2) out uint geomGraphicID;
layout(location = 3) out uint geomTextureID;
layout(location = 4) out uint geomFirstVertex;
layout(location = 5) out uint geomVertexCount;
layout(location = 6) out uint geomStyle;
layout(location = 7) out float billboardScale;

layout(push_constant) uniform PC {
    layout(offset = 0) float nearScale;
    layout(offset = 4) float farScale;
    layout(offset = 8) float near;
    layout(offset = 12) float far;
} pcst;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

void main() {
    // We compute the position in the camera space
    gl_Position = uCam.view * vec4(markerPos, 1.0);
    float z = gl_Position.z;
    // The other parameters are simply copied to the geometry shader
    geomColor = markerColor / 255.f;
    geomGraphicID = graphicID;
    geomTextureID = textureID;
    geomFirstVertex = firstVertex;
    geomVertexCount = vertexCount;
    geomStyle = style;

    // Clamp the coord of the vertex (when z is too small)
    if (z <= pcst.near)
        billboardScale = pcst.nearScale;
    else if (z > pcst.near && z <= pcst.far)
    {
        float p = (log(pcst.far / z)) / (log(pcst.far / pcst.near));
        billboardScale = p * pcst.nearScale + (1.0 - p) * pcst.farScale;
    }
    else
        billboardScale = pcst.farScale;

    billboardScale *= (uCam.proj * gl_Position).w;
}
