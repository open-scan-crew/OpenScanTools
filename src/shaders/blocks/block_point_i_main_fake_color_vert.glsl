void main() {
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = uCam.projView * worldPos;
#endif

    fragColor = vec4(hsl2rgb(vec3(pc.blending + (intensity / 255.0), (pc.saturation + 1.0) / 4.0, pc.luminance / 2.0 )), 1.0);
}