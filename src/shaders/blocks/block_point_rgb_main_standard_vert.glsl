void main() {
    gl_PointSize = pc.ptSize;
    vec4 worldPos4 = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos4;
#else
    gl_Position = uCam.projView * worldPos4;
#endif
    vec3 hsl = rgb2hsl(color.rgb/255.f);
    hsl.z *= pc.luminance;
    hsl.y *= pc.saturation;
    //fragColor = vec4(hsl2rgb(hsl) / 255.0, pc.transparency);
    fragColor = vec4(hsl2rgb(hsl), 1.0);
    filterReject = evaluateColorimetricFilter(getRgbNorm(), getIntensityNorm(), worldPos4.xyz);
    if (gPolygonHighlight > 0.5)
        fragColor.rgb = mix(fragColor.rgb, vec3(242.0 / 255.0, 214.0 / 255.0, 0.0), 0.85);
}
