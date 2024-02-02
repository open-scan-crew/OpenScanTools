void main() {
    gl_PointSize = pc.ptSize;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#else
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#endif
    vec3 hsv = rgb2hsv(color.rgb);
    float intensity = pc.contrast * (hsv.z / 255.0 + pc.brightness) + 0.5;
    //fragColor = vec4(pc.ptColor * intensity, pc.transparency);
    fragColor = vec4(pc.ptColor * intensity, 1.0);
}
