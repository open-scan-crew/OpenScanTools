// Vertex attributes
layout(location = 0) in uvec2 posXY;
layout(location = 1) in uint posZ;
#ifdef ATTRIB_I
layout(location = 2) in uint intensity;
#endif
#ifdef ATTRIB_RGB
layout(location = 3) in uvec4 color;
#endif
// Instanced attributes
layout(location = 5) in vec3 origin;
layout(location = 6) in float coordPrec;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PC {
    layout(offset = 0) float ptSize;
    layout(offset = 4) float transparency;
    layout(offset = 8) float contrast;
    layout(offset = 12) float brightness;
    layout(offset = 16) float saturation;
    layout(offset = 20) float luminance;
    layout(offset = 24) float blending;
    layout(offset = 28) float rampMin;
    layout(offset = 32) float rampMax;
    layout(offset = 36) int rampSteps;
    layout(offset = 40) float billboardEnable;
    layout(offset = 44) float billboardFeather;
    layout(offset = 48) vec3 ptColor;
    layout(offset = 60) float adaptiveSizeEnable;
    layout(offset = 64) float adaptiveSizeStrength;
} pc;

float computeAdaptivePointSize(vec4 clipPos)
{
    float pointSize = pc.ptSize;
    if (pc.adaptiveSizeEnable > 0.5f)
    {
        float depthMetric = log2(abs(clipPos.w) + 1.001f);
        float depthFactor = clamp(depthMetric * 0.35f, 0.0f, 1.0f);
        pointSize *= mix(1.0f, 1.0f + pc.adaptiveSizeStrength, depthFactor);
    }
    return pointSize;
}

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(set = 0, binding = 1) uniform uniformScan{
    mat4 model;
} uScan;
