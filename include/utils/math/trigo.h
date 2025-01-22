#ifndef TLS_MATH_TRIGO_H
#define TLS_MATH_TRIGO_H

#include "glm/glm.hpp"
#include "utils/math/basic_define.h"

namespace tls::math
{
    double radiansToDegrees(const double& degrees);

    glm::dvec3 quat_to_euler_zyx_rad(glm::dvec4 const& quaternion);
    glm::dvec3 quat_to_euler_zyx_deg(glm::dvec4 const& quaternion);

    glm::dvec3 quat_to_euler_zyx_rad(glm::dquat const& quaternion);
    glm::dvec3 quat_to_euler_zyx_deg(glm::dquat const& quaternion);

    glm::dvec3 quat_to_euler_zyx_rad(const double quaternion[4]);
    glm::dvec3 quat_to_euler_zyx_deg(const double quaternion[4]);
    glm::dquat euler_zyx_to_quat(const double r[3]);

    glm::dquat euler_zxz_to_quat(glm::dvec3 const& eulers);
    glm::dvec3 quat_to_euler_zxz(glm::dquat const& q);

    glm::dquat euler_deg_to_quat(glm::dvec3 const& euler);
    glm::dquat euler_rad_to_quat(glm::dvec3 const& eulers);


    // WIP - Ne fonctionne pas encore...
    // Compute a quaternion that transforms _u_ into _v_ and let _axis_ relatively unchanged.
    // It is the same that the glm::quat(u, v) but with an imposed rotation axis.
    glm::dquat quat_from_3_vector(glm::dvec3 u, glm::dvec3 v, glm::dvec3 axis);

    glm::dmat3 getRotationMatrix(const double quaternion[4]);

    template<typename T>
    T correctAngle(const T& angle)
    {
        int it = int(floor(angle / (M_2PI)));
        return ((T)(angle - (it * M_2PI)));
    }
}

#endif