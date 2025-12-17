void main() {
    vec4 worldPos = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
    vec4 clipPos = uCam.projView * worldPos;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = clipPos;
#endif

    gl_PointSize = computeAdaptivePointSize(clipPos);

    fragColor = vec4(hsl2rgb(vec3(pc.blending + (intensity / 255.0), (pc.saturation + 1.0) / 4.0, pc.luminance / 2.0 )), 1.0);
}