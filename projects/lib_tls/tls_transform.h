#ifndef TLS_TRANSFORM_H
#define TLS_TRANSFORM_H

#include "../../ext/glm/glm.hpp"

namespace tls::transform
{
    glm::mat4 getTransformMatrix(const double translation[3], const double quaternion[4]);
    glm::dmat4 getTransformDMatrix(const double translation[3], const double quaternion[4]);
    glm::mat4 getInverseTransformMatrix(const double translation[3], const double quaternion[4]);
    glm::dmat4 getInverseTransformDMatrix(const double translation[3], const double quaternion[4]);

    glm::mat4 getTransformMatrix(const double translation[3], const double quaternion[4], const double scale[3]);
    glm::mat4 getInverseTransformMatrix(const double translation[3], const double quaternion[4], const double scale[3]);
}
#endif