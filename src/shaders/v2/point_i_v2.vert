// Vertex Shader for points version 2.0
#version 450

// Vertex attributes
layout(location = 0) in uvec2 posXY;
layout(location = 1) in uint posZ;
layout(location = 2) in uint intensity;

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
    layout(offset = 40) float adaptivePointMinDistance;
    layout(offset = 44) float adaptivePointMaxDistance;
    layout(offset = 8) float contrast;
    layout(offset = 12) float brightness;
    layout(offset = 16) float saturation;
    layout(offset = 20) float luminance;
    layout(offset = 24) float blending;
    layout(offset = 28) float rampMin;
    layout(offset = 32) float rampMax;
    layout(offset = 36) int rampSteps;
    layout(offset = 48) vec3 ptColor;
    layout(offset = 60) float adaptivePointSizeEnabled;
} pc;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(set = 0, binding = 1) uniform uniformModelScan{
    mat4 model;
} uScan;

vec4 getWorldPosition()
{
    return uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
}

float computePointSize(const vec4 worldPos)
{
    if (pc.adaptivePointSizeEnabled < 0.5f)
        return pc.ptSize;

    float minDistance = max(pc.adaptivePointMinDistance, 0.0001f);
    float maxDistance = max(minDistance, pc.adaptivePointMaxDistance);
    float distanceToCamera = length((uCam.view * worldPos).xyz);

    if (maxDistance <= minDistance)
        return distanceToCamera <= minDistance ? pc.ptSize : 1.0f;

    if (distanceToCamera <= minDistance)
        return pc.ptSize;

    if (distanceToCamera >= maxDistance)
        return 1.0f;

    float t = (distanceToCamera - minDistance) / (maxDistance - minDistance);
    return mix(pc.ptSize, 1.0f, t);
}

void main() {
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);
    gl_Position = uCam.projView * worldPos;
    float fi = pc.contrast * (intensity / 255.0 + pc.brightness) + 0.5;
    fragColor = vec4(fi, fi, fi, pc.transparency);
}