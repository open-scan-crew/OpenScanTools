void main() {
    vec4 worldPos = getWorldPosition();
    gl_PointSize = computePointSize(worldPos);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos;
#else
    gl_Position = uCam.projView * worldPos;
#endif
    vec3 hsl = rgb2hsl(color.rgb/255.f);
    hsl.z *= pc.luminance;
    hsl.y *= pc.saturation;
    //fragColor = vec4(hsl2rgb(hsl) / 255.0, pc.transparency);
    fragColor = vec4(hsl2rgb(hsl), 1.0);
}