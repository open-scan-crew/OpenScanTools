// Vertext Shader for points with "intensity" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "./blocks/block_point_input_geom.glsl"

void main() {
    for(int iterator = 1; iterator <= pc.clippingIndex[0]; iterator++)
    {
        vec4 cboxPos = uCBox.normalizedMat[pc.clippingIndex[iterator]] * gl_in[0].gl_Position;
        if (abs(cboxPos.x) < 1.0 &&
            abs(cboxPos.y) < 1.0 &&
            abs(cboxPos.z) < 1.0)
            return;
    }
    gl_Position = uCam.projView * gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    colorOut = colorIn[0];
    EmitVertex();
}