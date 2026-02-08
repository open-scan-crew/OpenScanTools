void main() {
    gl_PointSize = pc.ptSize;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#else
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#endif
    float fi = pc.contrast * (intensity / 255.0 + pc.brightness) + 0.5;

    //fragColor = vec4(fi, fi, fi, pc.transparency);
    fragColor = vec4(fi, fi, fi, 1.0);
    filterReject = evaluateColorimetricFilter(getRgbNorm(), getIntensityNorm());
}
