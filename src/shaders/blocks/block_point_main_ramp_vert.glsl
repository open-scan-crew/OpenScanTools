
vec3 colorRamp(float r)
{
    r = floor(r * (pc.rampSteps)) / (pc.rampSteps - 1);
    float hue = ((1.f - r) * 2.f) / 3.f;
    return vec3(hue, 1.f, 0.5f);
}

void main()
{
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);

#ifdef ATTRIB_RGB
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
#else
    float gray = intensity;
#endif
    float iNorm = pc.contrast * (gray / 255.0 + pc.brightness) + 0.5;
    vec3 outColor = vec3(iNorm);

#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = uCam.projView * worldPos;
#endif

    // Apply the ramp on the color
    vec4 rampPos = uCam.view * worldPos;

    float r = pc.rampMax - pc.rampMin != 0 ? (length(rampPos.xyz) - pc.rampMin) / (pc.rampMax - pc.rampMin) : 0.f;
    if (r < 1.f && r > 0.0f)
    {
    #ifdef RAMP_FLAT
        outColor = hsl2rgb(colorRamp(r));
    #else
        outColor = hsl2rgb(colorRamp(r)) * iNorm;
    #endif
    }

    fragColor = vec4(outColor, 1.0);
}
