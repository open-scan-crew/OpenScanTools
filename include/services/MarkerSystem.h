#ifndef MARKER_SYSTEM_H
#define MARKER_SYSTEM_H

#include "models/data/Marker.h"
#include <QtCore/qstring.h>

enum class MarkerShape
{
    Centered = 0,
    Top_No_Arrow,
    Top_Arrow,
    Max_Enum
};

class MarkerSystem
{
public:
    struct Style {
        MarkerShape shape;
        bool showTrueColor;
        QString qresource;
        QString traduction;
    };

    struct RenderStyle {
        bool showTrueColor;
        uint16_t firstVertex;
        uint16_t vertexCount;
    };

    struct Primitive
    {
        uint16_t firstVertex;
        uint16_t vertexCount;
    };

    static Style getStyle(scs::MarkerIcon);
    static RenderStyle getRenderStyle(scs::MarkerIcon);
};

#endif