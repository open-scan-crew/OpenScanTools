void main() {
    gl_PointSize = pc.ptSize;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#else
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#endif

    fragColor = vec4(hsl2rgb(vec3(pc.blending + (intensity / 255.0), (pc.saturation + 1.0) / 4.0, pc.luminance / 2.0 )), 1.0);
    filterMask = colorFilterPass(vec3(0.0), fragColor.rgb * 255.0, float(intensity)) ? 1.0 : 0.0;
}
