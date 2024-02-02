#version 450
// Input attributes

layout(location = 0) in vec3 in_normal;

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

    // Use a simple white light
    vec3 light = vec3(0.0, 0.0, 1.0);

   float shade = (0.25f + 0.75f * abs(dot(in_normal, light)));
   out_color =  vec4(pc.color * shade, pc.alpha);
}