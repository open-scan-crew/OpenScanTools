#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC {
    layout(offset = 0) float ptSize;
    layout(offset = 8) float contrast;
    layout(offset = 12) float brightness;
    layout(offset = 16) float saturation;
    layout(offset = 20) float luminance;
    layout(offset = 24) float blending;
    layout(offset = 28) float rampMin;
    layout(offset = 32) float rampMax;
    layout(offset = 36) int rampSteps;
    layout(offset = 48) vec3 ptColor;
    layout(offset = 60) int roundPoint;
} pc;

void main() {
    if (pc.roundPoint != 0)
    {
        vec2 pixCentered = gl_PointCoord.st * 2.0 - vec2(1.0);
        float dist2 = dot(pixCentered, pixCentered);
        if (dist2 > 1.0)
            discard;
    }
	outColor = fragColor;
}

//https://github.com/potree/potree/blob/develop/src/materials/shaders/edl.fs
void main_poc_circle() {
	vec2 pixCentered=gl_PointCoord.st-vec2(0.5f);
	float dist=length(pixCentered);
	if (dist < 0.5f)
	{
		if (dist > 0.4f)
			outColor=vec4(0.0f, 0.0f, 0.0f, 1.0f);
		else
			outColor = fragColor;
	}
	else
	{
		discard;
	}
}
