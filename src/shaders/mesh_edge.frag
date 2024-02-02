#version 450
// Input attributes (None)

layout(push_constant) uniform PC {
    layout(offset = 64) uint id;
    layout(offset = 68) uint flags;
    layout(offset = 72) float alpha;
    layout(offset = 80) vec3 color;
} pc;

layout(location = 0) out vec4 out_color;
layout(location = 1) out uint out_id;

void main()
{
    out_id = pc.id;
    // Selection flag test
    if ((pc.flags & 0x01) != 0)
        out_color = vec4(0.59, 0.05, 0.87, 1.0); // Purple
    else if ((pc.flags & 0x02) != 0)
        out_color = vec4(0.92, 0.7, 0.0, 1.0); // Yellow
    else
        out_color = vec4(0.2, 0.2, 0.2, 1.0); // Dark grey
}