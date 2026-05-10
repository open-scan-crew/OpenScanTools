void main() {
    gl_PointSize = pc.ptSize;
    vec4 worldPos4 = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#ifdef _CLIPPING_ACTIVATED
    gl_Position = worldPos4;
#else
    gl_Position = uCam.projView * worldPos4;
#endif
    float fi = pc.contrast * (intensity / 255.0 + pc.brightness) + 0.5;

    //fragColor = vec4(pc.ptColor * fi, pc.transparency);
    fragColor = vec4(pc.ptColor * fi, 1.0);
    filterReject = evaluateColorimetricFilter(getRgbNorm(), getIntensityNorm(), worldPos4.xyz);
    if (gPolygonHighlight > 0.5)
        fragColor.rgb = mix(fragColor.rgb, vec3(242.0 / 255.0, 214.0 / 255.0, 0.0), 0.85);
}
