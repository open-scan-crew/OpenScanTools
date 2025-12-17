void main() {
    vec4 worldPos = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
    vec4 clipPos = uCam.projView * worldPos;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = clipPos;
#endif
    gl_PointSize = computeAdaptivePointSize(clipPos);
    float fi = pc.contrast * (intensity / 255.0 + pc.brightness) + 0.5;

    //fragColor = vec4(fi, fi, fi, pc.transparency);
    fragColor = vec4(fi, fi, fi, 1.0);
}