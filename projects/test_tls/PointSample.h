#ifndef POINT_SAMPLE_H
#define POINT_SAMPLE_H

#include <vector>

#include <tls_point.h>

#include <glm/glm.hpp>

class PointSample
{
public:
    static std::vector<tls::Point> Predefined();
    static std::vector<tls::Point> Brick(glm::vec3 size, glm::vec3 position, float density);
    static std::vector<tls::Point> Spheroid(glm::vec3 size, glm::vec3 position, uint32_t density);
};

#endif
