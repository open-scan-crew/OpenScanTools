#ifndef TLS_MATH_GEOMETRY_H
#define TLS_MATH_GEOMETRY_H

#include "glm/glm.hpp"

namespace tls
{
    namespace math
    {
        // The two lines are defined by a point (a, b) and a vector (u, v)
        // The first line is {a, u}
        // The second line is {b, v}
        // The result give the projection of each line on the other.
        glm::dvec2 intersect_line_line(glm::dvec3 a, glm::dvec3 u, glm::dvec3 b, glm::dvec3 v);

        // Gives the projection of a 'point' over a line.
        // The line is defined by a point 'a' and its normalized direction 'u'.
        glm::dvec2 projection_point_line_2d(glm::dvec2 point, glm::dvec2 a, glm::dvec2 u);

        glm::dvec3 intersect_line_plane(const glm::dvec3& a, const glm::dvec3& u, const glm::dvec3& n, double d);
    }
}

#endif