void main() {
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = uCam.projView * worldPos;
#endif
    fragColor = vec4(pc.ptColor, 1.0);
}