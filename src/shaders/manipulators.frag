#version 450
// Input attributes

layout(location = 1) in vec3 in_normal;
layout(location = 0) in vec3 in_position;
layout(location = 2) in vec3 in_mod;

layout(push_constant) uniform PC {
    layout(offset = 64) uint fragID;
    layout(offset = 80) vec3 color;
} pc;

layout(location = 0) out vec4 out_color;
layout(location = 1) out uint outID;

void main()
{
    vec3 L = normalize(vec3(0.0f,0.0f,900.0f) - in_position);
	vec3 N = normalize(in_normal);

    vec3 kd = pc.color * 0.25;   // diffuse reflection
    vec3 ka = pc.color * 0.85;   // ambient reflection
    vec3 ks = pc.color * in_mod * 0.25;   // kind of specular reflection 

    vec3 Id = kd * max(dot(L,N), 0.0);
	Id = clamp(Id, 0.0, 1.0);
   
    out_color = vec4((ka + Id + ks), 1.0f);
    outID = pc.fragID;
}