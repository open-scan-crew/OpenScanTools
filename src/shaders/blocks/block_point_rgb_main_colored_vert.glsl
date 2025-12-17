void main() {
    vec4 worldPos = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
    vec4 clipPos = uCam.projView * worldPos;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = clipPos;
#endif
    gl_PointSize = computeAdaptivePointSize(clipPos);
    vec3 hsv = rgb2hsv(color.rgb);
    float intensity = pc.contrast * (hsv.z / 255.0 + pc.brightness) + 0.5;
    //fragColor = vec4(pc.ptColor * intensity, pc.transparency);
    fragColor = vec4(pc.ptColor * intensity, 1.0);
}
