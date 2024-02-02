#version 450
// Vertex attributes
layout(location = 0) in vec3 firstPoint;
layout(location = 1) in vec3 secondPoint;
layout(location = 2) in uvec4 color;
layout(location = 3) in uint markerID;
layout(location = 4) in uint showMask;

layout(location = 0) out vec4 geomHighPoint;
layout(location = 1) out vec4 geomOrthoPoint;
layout(location = 2) out vec4 geomColor;
layout(location = 3) out uint geomId;
layout(location = 4) out uint geomMask;

layout(push_constant) uniform PC {
    layout(offset = 0) float lineWidth;
} pc;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

void main()
{
    vec4 modelFirst = vec4(firstPoint, 1.0);
    vec4 modelSecond = vec4(secondPoint, 1.0);
    if (modelFirst.z < modelSecond.z)
    {
        // the lowest point is passed in gl_Position
        gl_Position = uCam.projView * modelFirst;
        geomHighPoint = uCam.projView * modelSecond;
        geomOrthoPoint = uCam.projView * vec4(modelSecond.x, modelSecond.y, modelFirst.z, 1.0);
    }
    else
    {
        gl_Position = uCam.projView * modelSecond;
        geomHighPoint = uCam.projView * modelFirst;
        geomOrthoPoint = uCam.projView * vec4(modelFirst.x, modelFirst.y, modelSecond.z, 1.0);
    }
    geomColor = vec4(color / 255.0);
    geomId = markerID;
    geomMask = showMask;
}
