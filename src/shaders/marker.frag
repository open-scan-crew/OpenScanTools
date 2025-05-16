#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragUVW;
layout(location = 2) in flat uint fragID;
layout(location = 3) in flat uint fragStyle;

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outID;

layout(push_constant) uniform PC {
    layout(offset = 16) vec4 neutralColor;
    layout(offset = 32) int showDepth;
    layout(offset = 36) uint width;
    layout(offset = 40) uint height;
    layout(offset = 48) vec4 borderHover;
    layout(offset = 64) vec4 borderSelect;
} pcst;

layout(set = 1, binding = 2) uniform sampler2DArray texSampler;
layout(set = 2, binding = 3) buffer correctedDepth
{
    readonly float inDepth[ ];
};

void main()
{
    outID = fragID;

    if (fragUVW.y > 1.0)
    {
        float alpha = 0.4f;
        outColor = vec4(fragColor.rgb * alpha, alpha);
        return;
    }

    // NOTE - by using only the "border" we cannot show a marker both selected and hover style.
    vec4 border = pcst.neutralColor;
    if ((fragStyle & 0x01) != 0)
        border = pcst.borderSelect;
    else if ((fragStyle & 0x02) != 0)
        border = pcst.borderHover;

    vec4 texColor = texture(texSampler, fragUVW);

    // true color flag
    if ((fragStyle & 0x04) == 0)
        texColor.rgb = fragColor.rgb;

    // Create a smooth border blend
    float alphaU = fragUVW.x < 0.5 ? 1.0 - 5 * fragUVW.x : 5 * fragUVW.x - 4.0;
    float alphaV = fragUVW.y < 0.5 ? 1.0 - 5 * fragUVW.y : 5 * fragUVW.y - 4.0;
    float blend = max(0, max(alphaU, alphaV));

    if(texColor.a == 1.0f)
        texColor.a = min(1.0f, (1.0f - blend) * 2.0f);

    vec4 background = mix(pcst.neutralColor, border, blend);
    float outA = mix(background.a, 1.0, texColor.a);

    vec3 finalColor = mix(background.rgb, texColor.rgb, texColor.a);

    if (pcst.showDepth != 0)
    {
        uint index = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * pcst.width;
        float depth = inDepth[index];
        if (gl_FragCoord.z < depth)
            outColor = vec4(finalColor, outA);
        else
        {
            int X = int(gl_FragCoord.x) % 10;
            int Y = int(gl_FragCoord.y) % 10;
            float weight = (X >= 5) != (Y >= 5) ? 1.0 : 0.65;
            outColor = vec4(finalColor * weight, outA);
        }
    }
    else
    {
        outColor = vec4(finalColor, outA);
    }
}