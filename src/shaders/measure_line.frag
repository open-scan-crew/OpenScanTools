#version 450
// Input attributes
layout(location = 0) in flat vec4 fragColor;
layout(location = 1) in flat uint fragID;
layout(location = 2) in float fragU;

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outID;

layout(push_constant) uniform PC {
    layout(offset = 16) vec3 lightDir;
    layout(offset = 28) float stripeCount;
} pc;

void main()
{
    float k = floor(cos(fragU * pc.stripeCount * 2 * 3.14159) + 1.0);
    outColor = mix(fragColor, vec4(0.2, 0.2, 0.2, 1.0), k);
    outID = fragID;
}