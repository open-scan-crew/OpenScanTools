void main() {
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = uCam.projView * worldPos;
#endif
    float fi = pc.contrast * (intensity / 255.0 + pc.brightness) + 0.5;

    //fragColor = vec4(fi, fi, fi, pc.transparency);
    fragColor = vec4(fi, fi, fi, 1.0);
}