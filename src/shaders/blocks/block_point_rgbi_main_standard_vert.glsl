void main() {
    gl_PointSize = pc.ptSize;
#ifdef _CLIPPING_ACTIVATED
    gl_Position = uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#else
    gl_Position = uCam.projView * uScan.model * vec4(vec3(posXY, posZ) * coordPrec + origin, 1.0);
#endif
    vec3 hsl = rgb2hsl(color.rgb/255.0f);
    float l = hsl.x;
    if(hsl.z < ((pc.blending + 1.0) / 2.0))
        hsl.x = intensity / 255.0f;
    if((intensity / 255.0f) < ((pc.blending + 1.0) / 2.0))
        hsl.x = l;
    hsl.z *= pc.luminance;
    hsl.y *= pc.saturation;
    //fragColor = vec4(hsl2rgb(hsl), pc.transparency);
    fragColor = vec4(hsl2rgb(hsl), 1.0);
}