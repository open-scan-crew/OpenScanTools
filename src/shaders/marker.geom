// Geometry Shader //
// Emit the vertex of the billboard for the marker

#version 450
layout (points, invocations = 1) in;
layout (triangle_strip, max_vertices = 8) out;

// input attributes
//layout(location = 0) in vec4 geomPos[];
layout(location = 1) in vec4 geomColor[];
layout(location = 2) in uint geomGraphicID[];
layout(location = 3) in uint geomTextureID[];
layout(location = 4) in uint geomFirstVertex[];
layout(location = 5) in uint geomVertexCount[];
layout(location = 6) in uint geomStatus[];
layout(location = 7) in float billboardScale[];

in gl_PerVertex {
    vec4 gl_Position;
}gl_in[];

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragUVW;
layout(location = 2) out uint fragID;
layout(location = 3) out uint fragStatus;

layout(set = 3, binding = 4) uniform uniformCamera {
    mat4 view;
    mat4 proj;
} uCam;

layout(set = 4, binding = 5) buffer vertexPosition
{
    vec2 pos[];
};

layout(set = 4, binding = 6) buffer vertexUV
{
    vec2 uv[];
};

void main() {
    for (uint i = geomFirstVertex[0]; i < geomFirstVertex[0] + geomVertexCount[0]; ++i)
    {
        vec4 viewPos = gl_in[0].gl_Position + vec4(pos[i] * billboardScale[0], 0.0, 0.0);
        gl_Position = uCam.proj * viewPos;
        fragColor = geomColor[0];
        fragUVW = vec3(uv[i], geomTextureID[0]);
        fragID = geomGraphicID[0];
        fragStatus = geomStatus[0];
        EmitVertex();
    }
    EndPrimitive();
}