float hueDistance(float a, float b)
{
    float d = abs(a - b);
    return min(d, 1.0 - d);
}

vec3 projectToExperimentalPalette(vec3 hsv, float paletteSize)
{
    // Internal pass 3A path:
    // keep hue close to source and only apply a gentle local quantization.
    float k = clamp(floor(paletteSize + 0.5), 3.0, 16.0);
    vec3 best = hsv;
    float bestDist = 1e9;
    float step = 1.0 / k;
    float anchorBias = 0.25;

    for (int i = 0; i < 16; ++i)
    {
        if (float(i) >= k)
            break;

        float hueAnchor = (float(i) + 0.5) / k;
        float hueDelta = hueAnchor - hsv.x;
        hueDelta -= floor(hueDelta + 0.5);
        float projectedHue = fract(hsv.x + hueDelta * anchorBias + 1.0);
        vec3 candidate = vec3(projectedHue, hsv.y, hsv.z);
        float dh = hueDistance(hsv.x, candidate.x);
        float ds = hsv.y - candidate.y;
        float dv = hsv.z - candidate.z;
        float dist = (dh * dh) * (0.35 / max(step, 1e-4)) + (ds * ds) * 1.0 + (dv * dv) * 0.9;
        if (dist < bestDist)
        {
            bestDist = dist;
            best = candidate;
        }
    }

    return best;
}

void main() {
    gl_PointSize = pc.ptSize;
    vec4 worldPos4 = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos4;
#else
    gl_Position = uCam.projView * worldPos4;
#endif

    vec3 hsv = rgb2hsv(color.rgb / 255.0);
    // Pass 3: configurable cartoon controls (driven from toolbar + persisted in viewpoints/project).
    float valueLevels = max(pc.cartoonValueLevels, 2.0);
    float saturationLevels = max(pc.cartoonSaturationLevels, 1.0);
    float minSaturation = clamp(pc.cartoonSaturationMin, 0.0, 1.0);
    hsv.y = max(hsv.y, minSaturation);

    float satDen = max(saturationLevels - 1.0, 1.0);
    float valDen = max(valueLevels - 1.0, 1.0);
    hsv.y = floor(hsv.y * satDen + 0.5) / satDen;
    hsv.z = floor(hsv.z * valDen + 0.5) / valDen;

    // Pass 3A (internal, non-UI): optional experimental palette projection.
    // 0.0 = disabled (legacy path), >0.0 = palette size.
    if (pc.cartoonExperimentalPaletteSize > 0.5)
        hsv = projectToExperimentalPalette(hsv, pc.cartoonExperimentalPaletteSize);

    // Keep existing toolbar controls active in cartoon mode.
    hsv.y = clamp(hsv.y * pc.saturation, 0.0, 1.0);
    hsv.z = clamp(hsv.z * pc.luminance, 0.0, 1.0);

    fragColor = vec4(hsv2rgb(hsv), 1.0);
    filterReject = evaluateColorimetricFilter(getRgbNorm(), getIntensityNorm(), worldPos4.xyz);
    if (gPolygonHighlight > 0.5)
        fragColor.rgb = mix(fragColor.rgb, vec3(242.0 / 255.0, 214.0 / 255.0, 0.0), 0.85);
}
