#version 440

// TODO - input attachment
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;

layout(push_constant) uniform PC {
   layout(offset = 0) float alpha;
} pc;

layout(location = 0) out vec4 outColor;

void main() {
   vec4 sumColor = subpassLoad(inputColor);
   if (sumColor.a > 0.0)
   {
      float n = sumColor.a / pc.alpha;
      float a = 1.0 - pow(1.0 - pc.alpha, n);
      vec3 rgb = sumColor.rgb / n;
      outColor = vec4(rgb, a);
   }
   else
   {
      outColor = vec4(0);
   }
}