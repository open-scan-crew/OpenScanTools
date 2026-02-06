void main() {
    gl_PointSize = pc.ptSize;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#else
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#endif
    float fi = pc.contrast * (intensity / 255.0 + pc.brightness) + 0.5;

    //fragColor = vec4(pc.ptColor * fi, pc.transparency);
    fragColor = vec4(pc.ptColor * fi, 1.0);
    filterMask = colorFilterPass(vec3(0.0), fragColor.rgb * 255.0, float(intensity)) ? 1.0 : 0.0;
}
