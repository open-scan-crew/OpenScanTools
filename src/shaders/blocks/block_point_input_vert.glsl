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
layout(location = 1) out float filterReject;

layout(push_constant) uniform PC {
    layout(offset = 0) float ptSize;
    layout(offset = 8) float contrast;
    layout(offset = 12) float brightness;
    layout(offset = 16) float saturation;
    layout(offset = 20) float luminance;
    layout(offset = 24) float blending;
    layout(offset = 28) float rampMin;
    layout(offset = 32) float rampMax;
    layout(offset = 36) int rampSteps;
    layout(offset = 48) vec3 ptColor;
} pc;

layout(set = 0, binding = 0) uniform uniformCamera {
    mat4 view;
    mat4 proj;
    mat4 projView;
} uCam;

layout(set = 0, binding = 1) uniform uniformScan{
    mat4 model;
} uScan;

layout(set = 0, binding = 4) uniform uniformColorimetricFilter {
    vec4 colors[4];
    vec4 settings; // x: enabled, y: showColors, z: tolerance(0-1), w: intensityMode
} uColorFilter;

const float COLORIMETRIC_MAX_DISTANCE = 1.7320508;

#ifdef ATTRIB_I
float getIntensityNorm()
{
    return intensity / 255.0;
}
#else
float getIntensityNorm()
{
    return 0.0;
}
#endif

#ifdef ATTRIB_RGB
vec3 getRgbNorm()
{
    return color.rgb / 255.0;
}
#else
vec3 getRgbNorm()
{
    return vec3(getIntensityNorm());
}
#endif

float evaluateColorimetricFilter(vec3 rgb, float intensityNorm)
{
    if (uColorFilter.settings.x < 0.5)
        return 0.0;

    bool match = false;
    if (uColorFilter.settings.w > 0.5)
    {
        if (uColorFilter.colors[0].w > 0.5)
        {
            float diff = abs(intensityNorm - uColorFilter.colors[0].x);
            match = diff <= uColorFilter.settings.z;
        }
    }
    else
    {
        float threshold = uColorFilter.settings.z * COLORIMETRIC_MAX_DISTANCE;
        for (int i = 0; i < 4; ++i)
        {
            if (uColorFilter.colors[i].w > 0.5)
            {
                float dist = distance(rgb, uColorFilter.colors[i].xyz);
                if (dist <= threshold)
                {
                    match = true;
                    break;
                }
            }
        }
    }

    bool showColors = (uColorFilter.settings.y > 0.5);
    bool reject = showColors ? !match : match;
    return reject ? 1.0 : 0.0;
}
