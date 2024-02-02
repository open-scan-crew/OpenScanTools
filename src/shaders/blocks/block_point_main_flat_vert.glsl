void main() {
    gl_PointSize = pc.ptSize;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#else
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#endif
    fragColor = vec4(pc.ptColor, 1.0);
}