layout (points, invocations = 1) in;
layout (points, max_vertices = 1) out;

#define MAX_CLIPPING 512

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
} gl_in[];

layout(location = 0) in vec4 colorIn[];
layout(location = 0) out vec4 colorOut;

layout(set = 0, binding = 3) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

struct ClippingData
{
    mat4 transfo;
    vec4 limits;
    vec3 color;
    int steps;
};

layout(set = 0, binding = 4) uniform uniformClipping {
    ClippingData data[MAX_CLIPPING];
} uClip;

layout(push_constant) uniform PC {
    layout(offset = 64) uint clippingIndex[16];
} pc;
