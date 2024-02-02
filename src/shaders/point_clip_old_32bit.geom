// Vertext Shader for points with "intensity" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "./blocks/block_point_input_geom.glsl"

bool isPointInteriorPerMatrix_Square(in uint iterator)
{
    vec4 cboxPos = uCBox.normalizedMat[iterator] * gl_in[0].gl_Position;
    if (abs(cboxPos.x) > 1.0 ||
        abs(cboxPos.y) > 1.0 ||
        abs(cboxPos.z) > 1.0)
        return false;
    return true;
}

bool isPointInteriorPerMatrix_Sphere(in uint iterator)
{
    vec4 cboxPos = uCBox.normalizedMat[iterator] * gl_in[0].gl_Position;;
    if (length(cboxPos.xyz) > 1.0)
        return false;
    return true;
}

bool isPointInteriorPerMatrix_Cylindric(in uint iterator)
{
    vec4 cboxPos = uCBox.normalizedMat[iterator] * gl_in[0].gl_Position;;
    if (length(cboxPos.xy) > 1.0 || abs(cboxPos.z) > 1.0)
        return false;
    return true;
}

bool isPointInteriorPerMatrix(in uint type, in uint iterator)
{
    if(type == 0)
        return isPointInteriorPerMatrix_Square(iterator);
    else if((type & 0x20000000) != 0)
        return isPointInteriorPerMatrix_Sphere(iterator);
    else //if((type & 0x40000000) != 0)
       return isPointInteriorPerMatrix_Cylindric(iterator);
}

bool validPoint(in uint size, in uint mode)
{
    bool checkExterior = true;
    bool checkInterior = false;
    bool unionOnce = false;
    bool intersOnce = false;
    for(int iterator = 1; iterator <= size; iterator++)
    {
        uint value = pc.clippingIndex[iterator];
        if((value & 0x80000000) == 0)
        {
            checkInterior = checkInterior || isPointInteriorPerMatrix(value & 0x70000000, value & 0x0FFFFFFF);
            unionOnce = true;
        }
        else
        {
            checkExterior = checkExterior && !isPointInteriorPerMatrix(value & 0x70000000, value & 0x0FFFFFFF);
            intersOnce = true;
        }
    }
    if(intersOnce && unionOnce)
    {
        //if(mode == 0)
            return checkExterior && checkInterior;
        //else
            //return checkExterior || checkInterior;
    }
    else if(unionOnce)
        return checkInterior;
    else
        return checkExterior;
}

void main()
{
    // QUESTION(robin) - À quoi sert le bit de poids fort (0x80000000) dans le premier index ?
    //                 - Est-ce toujours utilisé ?
    uint size = pc.clippingIndex[0] & 0x7FFFFFFF;
    if( (size == 0) || validPoint(size, pc.clippingIndex[0] & 0x80000000) )
    {
        gl_Position = uCam.projView * gl_in[0].gl_Position;
        gl_PointSize = gl_in[0].gl_PointSize;
        colorOut = colorIn[0];
        EmitVertex();
    }
}