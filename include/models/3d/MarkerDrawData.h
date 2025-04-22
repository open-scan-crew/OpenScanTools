#ifndef MARKER_DRAW_DATA_H
#define MARKER_DRAW_DATA_H

#include <cstdint>

// NEW - Feed to the vertex shader
struct MarkerDrawData
{
    float position[3];    // the anchor position
    uint32_t color_rgba;  // rgba color
    uint32_t graphicID;   // reference to the object. Enable action on click.
    uint32_t textureID;   // indicate which layer to sample in the 3D texture.
    uint16_t firstVertex; // used in the geometry shader
    uint16_t vertexCount; // used in the geometry shader
    uint32_t style;       // trueColor, hover, select
}; // size = 32

#endif