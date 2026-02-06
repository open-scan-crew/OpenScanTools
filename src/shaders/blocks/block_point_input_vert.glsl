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
layout(location = 1) out flat int filterReject;

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

layout(set = 0, binding = 2) uniform uniformColorimetricFilter {
    vec4 settings;
    vec4 colors[4];
} uFilter;

vec3 closestPointOnSegment(vec3 p, vec3 a, vec3 b)
{
    vec3 ab = b - a;
    float denom = dot(ab, ab);
    if (denom <= 0.000001)
        return a;
    float t = dot(p - a, ab) / denom;
    t = clamp(t, 0.0, 1.0);
    return a + t * ab;
}

vec3 closestPointOnTriangle(vec3 p, vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    vec3 ac = c - a;
    vec3 ap = p - a;
    float d1 = dot(ab, ap);
    float d2 = dot(ac, ap);
    if (d1 <= 0.0 && d2 <= 0.0)
        return a;

    vec3 bp = p - b;
    float d3 = dot(ab, bp);
    float d4 = dot(ac, bp);
    if (d3 >= 0.0 && d4 <= d3)
        return b;

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
    {
        float v = d1 / (d1 - d3);
        return a + v * ab;
    }

    vec3 cp = p - c;
    float d5 = dot(ab, cp);
    float d6 = dot(ac, cp);
    if (d6 >= 0.0 && d5 <= d6)
        return c;

    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0)
    {
        float w = d2 / (d2 - d6);
        return a + w * ac;
    }

    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0)
    {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b);
    }

    float denom = 1.0 / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

bool colorimetricInside(vec3 colorValue)
{
    if (uFilter.settings.x < 0.5)
        return true;

    int colorCount = int(uFilter.settings.z + 0.5);
    if (colorCount <= 0)
        return true;

    float tolerance = uFilter.settings.w;
    vec3 tol = vec3(tolerance);

    vec3 c0 = uFilter.colors[0].xyz;
    if (colorCount == 1)
        return all(lessThanEqual(abs(colorValue - c0), tol));

    vec3 c1 = uFilter.colors[1].xyz;
    if (colorCount == 2)
    {
        vec3 closest = closestPointOnSegment(colorValue, c0, c1);
        return all(lessThanEqual(abs(colorValue - closest), tol));
    }

    vec3 c2 = uFilter.colors[2].xyz;
    if (colorCount == 3)
    {
        vec3 closest = closestPointOnTriangle(colorValue, c0, c1, c2);
        return all(lessThanEqual(abs(colorValue - closest), tol));
    }

    vec3 c3 = uFilter.colors[3].xyz;
    vec3 closestA = closestPointOnTriangle(colorValue, c0, c1, c2);
    vec3 closestB = closestPointOnTriangle(colorValue, c0, c2, c3);
    float distA = length(colorValue - closestA);
    float distB = length(colorValue - closestB);
    vec3 closest = distA < distB ? closestA : closestB;
    return all(lessThanEqual(abs(colorValue - closest), tol));
}

int colorimetricReject(vec3 colorValue)
{
    if (uFilter.settings.x < 0.5)
        return 0;
    bool inside = colorimetricInside(colorValue);
    bool showColors = uFilter.settings.y > 0.5;
    bool allow = showColors ? inside : !inside;
    return allow ? 0 : 1;
}
