#version 450

layout (points, invocations = 6) in;
layout (points, max_vertices = 6) out;

layout(location = 0) in vec4 highPos[];
layout(location = 1) in vec4 orthoPos[];
layout(location = 2) in uint IdIn[];
layout(location = 3) in uint maskIn[];
layout(location = 4) in float geomRadiusIn[];

layout(location = 0) out float fragRadius;
layout(location = 1) out uint fragID;

layout(push_constant) uniform PC {
    layout(offset = 8) uint showMask;
} pc;

void main() {
    // common attributes
    fragID = IdIn[0];
    uint localMask = pc.showMask & maskIn[0];
    gl_PointSize = gl_in[0].gl_PointSize;
    fragID = IdIn[0];
    fragRadius = geomRadiusIn[0];
    if ((localMask & 0x01) == 0x01)
    {
        if(gl_InvocationID == 0)
        {
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();
            EndPrimitive();
        }
        if(gl_InvocationID == 1)
        {
            gl_Position = highPos[0]; 
            EmitVertex();
            EndPrimitive();
        }
    }
    // Vertical line (light blue)
    if ((localMask & 0x02) == 0x02)
    {
        if(gl_InvocationID == 2)
        {
            gl_Position = highPos[0];
            EmitVertex();
            EndPrimitive();   
        }
        if(gl_InvocationID == 3)
        {
            gl_Position = orthoPos[0];
            EmitVertex();
            EndPrimitive();    
        }
    }
    // Horizontal line (Pink)
    if ((localMask & 0x04) == 0x04)
    {
        if(gl_InvocationID == 4)
        {
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();
            EndPrimitive();    
        }
        if(gl_InvocationID == 5)
        {
            gl_Position = orthoPos[0];
            EmitVertex();
            EndPrimitive();
        }
    }
}