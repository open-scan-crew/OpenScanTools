#ifndef POINT_XYZIRGB_H
#define POINT_XYZIRGB_H

#include "glm/glm.hpp"
#include <cstdint>

struct PointXYZIRGB {
    float x;
    float y;
    float z;
    uint8_t i;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Coord16 {
    Coord16() {};
    Coord16(PointXYZIRGB const& pt, float precision, float origin[3]) {
        x = (uint16_t)((pt.x - origin[0]) / precision);
        y = (uint16_t)((pt.y - origin[1]) / precision);
        z = (uint16_t)((pt.z - origin[2]) / precision);
    };

    Coord16(uint16_t _x, uint16_t _y, uint16_t _z) : x(_x), y(_y), z(_z) { };

    uint16_t x;
    uint16_t y;
    uint16_t z;
};

struct Color24 {
    Color24() {};
    Color24(PointXYZIRGB const& pt) : r(pt.r), g(pt.g), b(pt.b) { };
    Color24(uint8_t const& _r, uint8_t const& _g, uint8_t const& _b) : r(_r), g(_g), b(_b) {};
    uint8_t r;
    uint8_t g;
    uint8_t b;
};



inline PointXYZIRGB convert_keepIRGB(const PointXYZIRGB& point)
{
    return { point.x, point.y, point.z, point.i, point.r, point.g, point.b };
}

inline PointXYZIRGB convert_overwriteI(const PointXYZIRGB& point)
{
    uint8_t newI = (point.r + point.g + point.b) / 3;

    return { point.x, point.y, point.z, newI, point.r, point.g, point.b };
}

inline PointXYZIRGB convert_overwriteRGB(const PointXYZIRGB& point)
{
    return { point.x, point.y, point.z, point.i, point.i, point.i, point.i };
}


inline PointXYZIRGB convert_keepIRGB(const PointXYZIRGB& point, const glm::mat4& matrix)
{
    glm::vec4 newPos = matrix * glm::vec4(point.x, point.y, point.z, 1.0);

    return { newPos.x, newPos.y, newPos.z, point.i, point.r, point.g, point.b };
}

inline PointXYZIRGB convert_overwriteI(const PointXYZIRGB& point, const glm::mat4& matrix)
{
    glm::vec4 newPos = matrix * glm::vec4(point.x, point.y, point.z, 1.0);
    uint8_t newI = (point.r + point.g + point.b) / 3;

    return { newPos.x, newPos.y, newPos.z, newI, point.r, point.g, point.b };
}

inline PointXYZIRGB convert_overwriteRGB(const PointXYZIRGB& point, const glm::mat4& matrix)
{
    glm::vec4 newPos = matrix * glm::vec4(point.x, point.y, point.z, 1.0);

    return { newPos.x, newPos.y, newPos.z, point.i, point.i, point.i, point.i };
}

#endif