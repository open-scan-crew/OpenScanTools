void main() {
    gl_PointSize = pc.ptSize;
    vec4 worldPos4 = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos4;
#else
    gl_Position = uCam.projView * worldPos4;
#endif

    // Pass 2 defaults for the cartoon look:
    // - quantize value (lightness proxy in HSV) for posterization
    // - enforce and quantize saturation to reduce noisy desaturated tones
    const float kValueLevels = 8.0;
    const float kSaturationLevels = 4.0;
    const float kMinSaturation = 0.12;

    vec3 hsv = rgb2hsv(color.rgb / 255.0);
    hsv.y = max(hsv.y, kMinSaturation);

    float satDen = max(kSaturationLevels - 1.0, 1.0);
    float valDen = max(kValueLevels - 1.0, 1.0);
    hsv.y = floor(hsv.y * satDen + 0.5) / satDen;
    hsv.z = floor(hsv.z * valDen + 0.5) / valDen;

    // Keep existing toolbar controls active in cartoon mode.
    hsv.y = clamp(hsv.y * pc.saturation, 0.0, 1.0);
    hsv.z = clamp(hsv.z * pc.luminance, 0.0, 1.0);

    fragColor = vec4(hsv2rgb(hsv), 1.0);
    filterReject = evaluateColorimetricFilter(getRgbNorm(), getIntensityNorm(), worldPos4.xyz);
    if (gPolygonHighlight > 0.5)
        fragColor.rgb = mix(fragColor.rgb, vec3(242.0 / 255.0, 214.0 / 255.0, 0.0), 0.85);
}
