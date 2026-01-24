#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outWeightDepth;

layout(push_constant) uniform PC {
    layout(offset = 0) float ptSize;
    layout(offset = 40) float splatRadiusPx;
    layout(offset = 44) int pointShape;
} pc;

void main() {
	float weight = 1.0;
	if (pc.pointShape == 1)
	{
		vec2 centeredPx = (gl_PointCoord - vec2(0.5)) * pc.ptSize;
		float dist2 = dot(centeredPx, centeredPx);
		float sigma = max(pc.splatRadiusPx, 0.001);
		weight = exp(-0.5 * dist2 / (sigma * sigma));
		if (weight < 0.0005)
		{
			discard;
		}
	}

	outColor = vec4(fragColor.rgb * weight, weight);
	outWeightDepth = vec2(weight, gl_FragCoord.z * weight);
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
