layout(set = 0, binding = 2) uniform uniformColorFilter {
    vec4 params;
    vec4 colorA;
    vec4 colorB;
    vec4 colorC;
} uColorFilter;

vec3 getFilterColor(int index)
{
    if (index == 0)
        return uColorFilter.colorA.xyz;
    if (index == 1)
        return vec3(uColorFilter.colorA.w, uColorFilter.colorB.x, uColorFilter.colorB.y);
    if (index == 2)
        return vec3(uColorFilter.colorB.z, uColorFilter.colorB.w, uColorFilter.colorC.x);
    return vec3(uColorFilter.colorC.y, uColorFilter.colorC.z, uColorFilter.colorC.w);
}

float labPivot(float t)
{
    return (t > 0.008856) ? pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);
}

vec3 rgbToLab(vec3 rgb255)
{
    vec3 srgb = rgb255 / 255.0;
    vec3 linear = pow((srgb + 0.055) / 1.055, vec3(2.4));
    linear = mix(srgb / 12.92, linear, step(0.04045, srgb));

    vec3 xyz;
    xyz.x = dot(linear, vec3(0.4124, 0.3576, 0.1805));
    xyz.y = dot(linear, vec3(0.2126, 0.7152, 0.0722));
    xyz.z = dot(linear, vec3(0.0193, 0.1192, 0.9505));

    vec3 ref = vec3(0.95047, 1.0, 1.08883);
    vec3 v = xyz / ref;
    v = vec3(labPivot(v.x), labPivot(v.y), labPivot(v.z));

    float L = max(0.0, 116.0 * v.y - 16.0);
    float a = 500.0 * (v.x - v.y);
    float b = 200.0 * (v.y - v.z);
    return vec3(L, a, b);
}

float distanceToSegment(vec3 p, vec3 a, vec3 b)
{
    vec3 ab = b - a;
    float denom = dot(ab, ab);
    if (denom <= 1e-6)
        return length(p - a);
    float t = clamp(dot(p - a, ab) / denom, 0.0, 1.0);
    vec3 closest = a + t * ab;
    return length(p - closest);
}

bool insideConvexPolygon(vec2 p, vec2 pts[4], int count, float tol)
{
    float signRef = 0.0;
    for (int i = 0; i < count; ++i)
    {
        vec2 a = pts[i];
        vec2 b = pts[(i + 1) % count];
        vec2 edge = b - a;
        float edgeLen = max(length(edge), 1e-6);
        float crossVal = (edge.x * (p.y - a.y) - edge.y * (p.x - a.x)) / edgeLen;
        if (abs(crossVal) < 1e-6)
            continue;
        if (signRef == 0.0)
            signRef = (crossVal > 0.0) ? 1.0 : -1.0;
        if (signRef * crossVal < -tol)
            return false;
    }
    return true;
}

bool colorFilterPass(vec3 rawRgb, vec3 displayRgb, float intensity)
{
    uint packed = floatBitsToUint(uColorFilter.params.x);
    bool enabled = (packed & 1u) != 0u;
    if (!enabled)
        return true;

    int action = int((packed >> 1) & 1u);
    int space = int((packed >> 2) & 1u);
    int source = int((packed >> 3) & 1u);
    int colorCount = int((packed >> 4) & 7u);
    bool useIntensity = ((packed >> 7) & 1u) != 0u;

    if (colorCount <= 0)
        return true;

    float tolerance = uColorFilter.params.y;
    if (useIntensity)
    {
        float tol = 255.0 * tolerance / 100.0;
        float target = getFilterColor(0).x;
        bool pass = abs(intensity - target) <= tol;
        return (action == 1) ? !pass : pass;
    }

    vec3 ref1 = getFilterColor(0);
    vec3 ref2 = getFilterColor(1);
    vec3 ref3 = getFilterColor(2);
    vec3 ref4 = getFilterColor(3);

    vec3 colorRgb = (source == 0) ? rawRgb : displayRgb;
    vec3 colorSpaceValue = (space == 0) ? colorRgb : rgbToLab(colorRgb);

    vec3 tolVec = (space == 0) ? vec3(255.0) : vec3(100.0, 255.0, 255.0);
    tolVec *= tolerance / 100.0;
    float tol = max(max(tolVec.x, tolVec.y), tolVec.z);

    bool pass = true;
    if (colorCount == 1)
    {
        vec3 delta = abs(colorSpaceValue - ref1);
        pass = all(lessThanEqual(delta, tolVec));
    }
    else if (colorCount == 2)
    {
        pass = distanceToSegment(colorSpaceValue, ref1, ref2) <= tol;
    }
    else
    {
        vec3 v1 = ref2 - ref1;
        vec3 v2 = ref3 - ref1;
        vec3 normal = cross(v1, v2);
        float nLen = length(normal);
        if (nLen <= 1e-6)
        {
            pass = distanceToSegment(colorSpaceValue, ref1, ref2) <= tol;
        }
        else
        {
            normal /= nLen;
            vec3 axisU = normalize(v1);
            vec3 axisV = normalize(cross(normal, axisU));
            vec2 pts[4];
            pts[0] = vec2(0.0, 0.0);
            pts[1] = vec2(dot(v1, axisU), dot(v1, axisV));
            pts[2] = vec2(dot(ref3 - ref1, axisU), dot(ref3 - ref1, axisV));
            pts[3] = vec2(dot(ref4 - ref1, axisU), dot(ref4 - ref1, axisV));
            vec2 p2 = vec2(dot(colorSpaceValue - ref1, axisU), dot(colorSpaceValue - ref1, axisV));
            float planeDistance = abs(dot(colorSpaceValue - ref1, normal));
            if (planeDistance > tol)
            {
                pass = false;
            }
            else
            {
                int count = (colorCount == 3) ? 3 : 4;
                pass = insideConvexPolygon(p2, pts, count, tol);
            }
        }
    }

    return (action == 1) ? !pass : pass;
}
