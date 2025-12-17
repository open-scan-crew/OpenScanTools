#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC {
    layout(offset = 40) float billboardEnable;
    layout(offset = 44) float billboardFeather;
} pc;

void main() {
        if (pc.billboardEnable > 0.5f)
        {
                vec2 pixCentered = gl_PointCoord.st-vec2(0.5f);
                float dist = length(pixCentered);
                float radius = 0.5f;
                float feather = clamp(pc.billboardFeather, 0.0f, 0.5f);
                float softness = max(feather, 0.001f);
                float mask = clamp((radius - dist) / softness, 0.0f, 1.0f);
                if (mask <= 0.0f)
                        discard;
                outColor = vec4(fragColor.rgb, fragColor.a * mask);
        }
        else
        {
                outColor = fragColor;
        }
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