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
    vec4 polygonSettings; // x: enabled, y: showSelected, z: active, w: appliedPolygonCount
    vec4 polygonCounts;   // x: totalPolygonCount, y: pendingApply(0/1)
    mat4 polygonViewProj[8];
    vec4 polygonVertices[8 * 32];
    vec4 polygonMeta[8];
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

float gPolygonHighlight = 0.0;

bool pointInPolygon(vec2 p, int polygonIndex)
{
    int vertexCount = int(uColorFilter.polygonMeta[polygonIndex].x);
    if (vertexCount < 3)
        return false;

    bool inside = false;
    vec2 prev = uColorFilter.polygonVertices[polygonIndex * 32 + (vertexCount - 1)].xy;
    for (int i = 0; i < vertexCount; ++i)
    {
        vec2 curr = uColorFilter.polygonVertices[polygonIndex * 32 + i].xy;
        bool condY = (curr.y > p.y) != (prev.y > p.y);
        if (condY)
        {
            float denom = (prev.y - curr.y);
            if (abs(denom) > 1e-7)
            {
                float xCross = (prev.x - curr.x) * (p.y - curr.y) / denom + curr.x;
                if (p.x < xCross)
                    inside = !inside;
            }
        }
        prev = curr;
    }
    return inside;
}

void evaluatePolygonSelector(vec3 worldPos, out bool insideApplied, out bool insidePending, out bool insideHighlighted)
{
    insideApplied = false;
    insidePending = false;
    insideHighlighted = false;

    int totalPolygons = int(uColorFilter.polygonCounts.x);
    int appliedCount = int(uColorFilter.polygonSettings.w);
    int highlightedPolygonIndex = int(uColorFilter.polygonCounts.z);

    for (int p = 0; p < totalPolygons; ++p)
    {
        vec4 clip = uColorFilter.polygonViewProj[p] * vec4(worldPos, 1.0);
        if (clip.w <= 1e-7)
            continue;

        vec3 ndc = clip.xyz / clip.w;
        if (ndc.z < -1.0 || ndc.z > 1.0)
            continue;
        if (ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0)
            continue;

        vec2 uv = ndc.xy * 0.5 + vec2(0.5, 0.5);
        if (!pointInPolygon(uv, p))
            continue;

        if (p < appliedCount)
            insideApplied = true;
        else
            insidePending = true;

        if (p == highlightedPolygonIndex)
            insideHighlighted = true;
    }
}

float evaluateColorimetricFilter(vec3 rgb, float intensityNorm, vec3 worldPos)
{
    bool rejectByColorimetric = false;

    if (uColorFilter.settings.x > 0.5)
    {
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
        rejectByColorimetric = showColors ? !match : match;
    }

    bool insideApplied = false;
    bool insidePending = false;
    bool insideHighlighted = false;
    evaluatePolygonSelector(worldPos, insideApplied, insidePending, insideHighlighted);

    bool selectorEnabled = uColorFilter.polygonSettings.x > 0.5;
    bool selectorShowSelected = uColorFilter.polygonSettings.y > 0.5;
    bool selectorActive = uColorFilter.polygonSettings.z > 0.5;
    int appliedCount = int(uColorFilter.polygonSettings.w);
    bool pendingApply = uColorFilter.polygonCounts.y > 0.5;
    bool manageMode = uColorFilter.polygonCounts.w > 0.5;

    // Pending polygons: preview only (point tint), they must not affect Show/Hide filtering before Apply.
    // Keep Lot 1 behavior: highlight as soon as polygons are created, even if selector.active is false.
    float pendingHighlight = (selectorEnabled && pendingApply && insidePending) ? 1.0 : 0.0;
    float manageHighlight = (selectorEnabled && manageMode && insideHighlighted) ? 1.0 : 0.0;
    gPolygonHighlight = max(pendingHighlight, manageHighlight);

    bool rejectByPolygon = false;
    if (selectorEnabled && selectorActive && appliedCount > 0)
    {
        if (selectorShowSelected)
            rejectByPolygon = !insideApplied;
        else
            rejectByPolygon = insideApplied;
    }

    return (rejectByColorimetric || rejectByPolygon) ? 1.0 : 0.0;
}
