// Vertext Shader for points with "intensity" attribute and NO clipping
#version 450
#extension GL_GOOGLE_include_directive : enable
#include "./blocks/block_point_input_geom.glsl"
#include "./blocks/block_hsv_functions.glsl"

vec3 flatColorSum = vec3(0.f);
int flatColorCount = 0;

float rampMinT = 1.0f;
float rampHue = 0.0f;

void addFlatColor(in uint index)
{
    flatColorSum += uClip.data[index].color;
    flatColorCount++;
}

void setRampColor(in float d, in float d_min, in float d_max, in int steps)
{
    float t = (d - d_min) / (d_max - d_min);
    if (t >= 0.f && t < rampMinT)
    {
        rampMinT = t;
        t = floor(t * steps) / (steps - 1);
        rampHue = (1.f - t) * 2.f / 3.f;
    }
}

bool validPoint(in uint size)
{
    bool checkExterior = true;
    bool checkInterior = false;
    bool unionOnce = false;
    bool intersOnce = false;
    for(int iterator = 1; iterator <= size; iterator++)
    {
        // On stocke 2 uint16 dans un uint32
        uint rawIndex = pc.clippingIndex[iterator / 2];
        rawIndex = (iterator % 2 == 0) ? rawIndex & 0x0000FFFF : rawIndex >> 16;
        uint type = (rawIndex & 0xC000) >> 14;
        uint shape = (rawIndex & 0x3000) >> 12;
        uint index = rawIndex & 0x0FFF;

        float d, d_min, d_max;
        bool inside = false;
        vec4 clipPos = uClip.data[index].transfo * gl_in[0].gl_Position;
        vec4 limits = uClip.data[index].limits;

        switch (shape)
        {
        case 0: // Box
            d = clipPos.z;
            d_min = -limits.z;
            d_max = limits.z;
            inside = (abs(clipPos.x) <= limits.x &&
                      abs(clipPos.y) <= limits.y &&
                      abs(d) <= limits.z);
            break;
        case 1: // Cylinder
            d = length(clipPos.xy);
            d_min = limits.x;
            d_max = limits.y;
            inside = (d >= d_min &&
                      d <= d_max &&
                      abs(clipPos.z) <= limits.z);
            break;
        case 2: // Sphere
            d = length(clipPos.xyz);
            d_min = limits.x;
            d_max = limits.y;
            inside = (d >= d_min &&
                      d <= d_max);
            break;
        case 3: // Torus
            float l = length(clipPos.xy);
            d = sqrt(pow(l - limits.x, 2) + pow(clipPos.z, 2));
            d_min = limits.z;
            d_max = limits.w;
            inside = (d >= d_min &&
                      d <= d_max &&
                      clipPos.x / l >= limits.y &&
                      clipPos.y >= 0.f);
            break;
        }

        switch (type)
        {
        case 0: // interior clipping
            checkInterior = checkInterior || inside;
            unionOnce = true;
            break;
        case 1: // exterior clipping
            checkExterior = checkExterior && !inside;
            intersOnce = true;
            break;
        case 2: // coloration
            if (inside)
                addFlatColor(index);
            break;
        case 3: // ramp
            if (inside)
                setRampColor(d, d_min, d_max, uClip.data[index].steps);
            break;
        }
    }

    // On choisi si on garde le point
    if(intersOnce && unionOnce)
    {
        return checkExterior && checkInterior;
    }
    else if(unionOnce)
        return checkInterior;
    else
        return checkExterior;
}

void main()
{
    uint size = pc.clippingIndex[0] & 0x0000FFFF;

    if (size > 0 && validPoint(size))
    {
        gl_Position = uCam.projView * gl_in[0].gl_Position;
        gl_PointSize = gl_in[0].gl_PointSize;
        if (rampMinT < 1.0f)
        {
            // Convert the original color of the point to hsl to get luminance
            float l = rgb2luminance(colorIn[0].xyz);
            colorOut = vec4(hsl2rgb(vec3(rampHue, 1.f, l)), 1.f);
        }
        else if (flatColorCount > 0)
        {
            float l = rgb2luminance(colorIn[0].xyz);
            colorOut = vec4(flatColorSum / flatColorCount * l, 1.0);
        }
        else
            colorOut = colorIn[0];
        EmitVertex();
    }
}
