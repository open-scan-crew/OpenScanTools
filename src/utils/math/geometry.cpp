#include "utils/math/geometry.h"
#include "utils/math/basic_define.h"

glm::dvec2 tls::math::intersect_line_line(glm::dvec3 a, glm::dvec3 u, glm::dvec3 b, glm::dvec3 v)
{
    const double au = glm::dot(a, u);
    const double av = glm::dot(a, v);
    const double bu = glm::dot(b, u);
    const double bv = glm::dot(b, v);
    const double uv = glm::dot(u, v);
    if (uv * uv > 1 - 1e-5) {
        return glm::dvec2(0.0, 0.0);
    }
    const double s = (bu + av * uv - bv * uv - au) / (1 - uv * uv);
    const double t = (av + bu * uv - au * uv - bv) / (1 - uv * uv);
    return glm::dvec2(s, t);
}

glm::dvec2 tls::math::projection_point_line_2d(glm::dvec2 point, glm::dvec2 a, glm::dvec2 u)
{
    assert(abs(length(u) - 1.0) < 1.e-6);
    double s = glm::dot(point - a, u);
    glm::dvec2 proj = a + u * s;
    return proj;
}

glm::dvec3 tls::math::intersect_line_plane(const glm::dvec3& a, const glm::dvec3& u, const glm::dvec3& n, double d)
{
    double scalar = glm::dot(u, n);
    double pointDist = glm::dot(a, n) - d;

    if (scalar == 0.0) // we should test with an epsilon
        return glm::vec3(std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN()); // return NaN ?

    glm::dvec3 I = a - u * (pointDist / scalar); // Intersection
    return (I);
}
