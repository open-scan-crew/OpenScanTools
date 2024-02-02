#version 450
// Input attributes
layout(location = 0) in float radius;
layout(location = 1) in flat uint fragID;

// Output
layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outID;

layout(push_constant) uniform PC {
    layout(offset = 16) vec3 lightDir;
    layout(offset = 36) float nearZ;
    layout(offset = 40) float farZ;
    layout(offset = 44) uint flags;
} pc;

void main()
{
    vec2 pixCentered = gl_PointCoord.st * 2 - vec2(1.f, 1.f);
    float dist=length(pixCentered);

    if (dist > 1.f)
        discard;

    float h = sin(acos(dist));
    vec3 n = vec3(pixCentered, h);
    vec3 id = vec3(1.0, 1.0, 1.0);    // light color
    // Material properties
    vec3 kd;   // diffuse reflection
    if((pc.flags & 0x01) != 0)
        kd = vec3(0.59, 0.05, 0.87); // Purple
    else if ((pc.flags & 0x02) != 0)
        kd = vec3(0.92, 0.7, 0.0); // Yellow
    else
        kd = vec3(0.2, 0.2, 0.2); // Dark grey
    vec3 ka = kd / 3.0;              // ambient reflection
    //vec3 ks;

    vec3 Ia = ka * id;
    vec3 Id = kd * max(0, dot(n, pc.lightDir)) * id;
    outColor = vec4(Ia + Id, 1.0);

    // Change the depth of the fragment
    float zEye = (1.0 / gl_FragCoord.w) - (h * radius);
    float zClip = pc.farZ * (zEye - pc.nearZ) / (pc.farZ - pc.nearZ);
    gl_FragDepth = zClip / zEye;

    outID = fragID;
}