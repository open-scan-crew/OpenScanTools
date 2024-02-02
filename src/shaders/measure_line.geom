#version 450

layout (points, invocations = 1) in;
layout (line_strip, max_vertices = 6) out;

layout(location = 0) in vec4 highPos[];
layout(location = 1) in vec4 orthoPos[];
layout(location = 2) in vec4 colorIn[];
layout(location = 3) in uint IdIn[];
layout(location = 4) in uint maskIn[];

layout(location = 0) out vec4 colorOut;
layout(location = 1) out uint fragID;
layout(location = 2) out float fragU;

layout(push_constant) uniform PC {
    layout(offset = 8) uint showMask;
} pc;

void main() {
    // common attributes
    fragID = IdIn[0];
    colorOut = colorIn[0];
    uint localMask = pc.showMask & maskIn[0];

    // Main line (Yellow)
    if ((localMask & 0x01) == 0x01)
    {
        fragID = IdIn[0];
        colorOut = vec4(1.0, 0.9, 0.0, 1.0);
        gl_Position = gl_in[0].gl_Position;
        fragU = 0.0;
        EmitVertex();

        fragID = IdIn[0];
        colorOut = vec4(1.0, 0.9, 0.0, 1.0);
        gl_Position = highPos[0];
        fragU = 1.0;
        EmitVertex();
        EndPrimitive();
    }
    // Vertical line (light blue)
    if ((localMask & 0x02) == 0x02)
    {
        fragID = IdIn[0];
        colorOut = vec4(0.0, 0.5, 1.0, 1.0);
        gl_Position = highPos[0];
        fragU = 0.0;
        EmitVertex();
        
        fragID = IdIn[0];
        colorOut = vec4(0.0, 0.5, 1.0, 1.0);
        gl_Position = orthoPos[0];
        fragU = 1.0;
        EmitVertex();
        EndPrimitive();    
    }
    // Horizontal line (Pink)
    if ((localMask & 0x04) == 0x04)
    {
        fragID = IdIn[0];
        colorOut = vec4(1.0, 0.0, 1.0, 1.0);
        gl_Position = gl_in[0].gl_Position;
        fragU = 0.0;
        EmitVertex();

        fragID = IdIn[0];
        colorOut = vec4(1.0, 0.0, 1.0, 1.0);
        gl_Position = orthoPos[0];
        fragU = 1.0;
        EmitVertex();
        EndPrimitive();
    }
}
