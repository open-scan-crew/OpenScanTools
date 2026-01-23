#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC {
	layout(offset = 40) int roundPoint;
} pc;

void main() {
	if (pc.roundPoint != 0)
	{
		vec2 pixCentered = gl_PointCoord.st - vec2(0.5f);
		float dist = length(pixCentered);
		if (dist > 0.5f)
			discard;
	}
	outColor = fragColor;
}

//https://github.com/potree/potree/blob/develop/src/materials/shaders/edl.fs
void main_poc_circle() {
	vec2 pixCentered = gl_PointCoord.st-vec2(0.5f);
	float dist = length(pixCentered);
	if (dist < 0.5f)
	{
		if (dist > 0.4f)
			outColor = vec4(0.0f,0.0f,0.0f,1.0f);
		else
			outColor = fragColor;
	}
	else
	{
		discard;
	}
}
