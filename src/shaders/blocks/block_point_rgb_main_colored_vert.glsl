void main() {
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = uCam.projView * worldPos;
#endif
    vec3 hsv = rgb2hsv(color.rgb);
    float intensity = pc.contrast * (hsv.z / 255.0 + pc.brightness) + 0.5;
    //fragColor = vec4(pc.ptColor * intensity, pc.transparency);
    fragColor = vec4(pc.ptColor * intensity, 1.0);
}
