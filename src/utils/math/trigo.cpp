#include "utils/math/trigo.h"

#include <glm/detail/type_quat.hpp>
#include <glm/gtx/quaternion.hpp>

double tls::math::radiansToDegrees(const double& degrees) {
    return (degrees / M_PI * 180.0);
}

glm::dvec3 tls::math::quat_to_euler_zyx_rad(glm::dvec4 const& q)
{
    glm::dvec3 euler_zyx;

    // Rotation in Euler angles (Z-Y-X)
    // Psi (x-axis rotation)
    double sinPsi_cosTheta = 2.0 * (q.w * q.x + q.y * q.z);
    double cosPsi_cosTheta = 1.0 + 2.0 * (q.x * q.x + q.y * q.y);
    euler_zyx.x = atan2(sinPsi_cosTheta, cosPsi_cosTheta);

    // Theta (y-axis rotation)
    double sinTheta = 2.0 * (q.w * q.y - q.z * q.x);
    if (fabs(sinTheta) >= 1)
        euler_zyx.y = copysign(M_PI / 2, sinTheta);
    else
        euler_zyx.y = asin(sinTheta);

    // Phi (z-axis rotation)
    double sinPhi_cosTheta = 2.0 * (q.w * q.z + q.x * q.y);
    double cosPhi_cosTheta = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
    euler_zyx.z = atan2(sinPhi_cosTheta, cosPhi_cosTheta);

    return euler_zyx;
}

glm::dvec3 tls::math::quat_to_euler_zyx_deg(glm::dvec4 const& q)
{
    glm::dvec3 euler_zyx = quat_to_euler_zyx_rad(q);
    euler_zyx.x = radiansToDegrees(euler_zyx.x);
    euler_zyx.y = radiansToDegrees(euler_zyx.y);
    euler_zyx.z = radiansToDegrees(euler_zyx.z);
    return euler_zyx;
}

glm::dvec3 tls::math::quat_to_euler_zyx_rad(glm::dquat const& quaternion)
{
    return quat_to_euler_zyx_rad(glm::dvec4(quaternion.x, quaternion.y, quaternion.z, quaternion.w));
}

glm::dvec3 tls::math::quat_to_euler_zyx_deg(glm::dquat const& quaternion)
{
    glm::dvec3 eulers(glm::eulerAngles(quaternion) / M_PI * 180.0);
    if (eulers.x < 0.001 && eulers.x > -0.001)
        eulers.x = 0.0;
    if (eulers.y < 0.001 && eulers.y > -0.001)
        eulers.y = 0.0;
    if (eulers.z < 0.001 && eulers.z > -0.001)
        eulers.z = 0.0;
    if (eulers.x >= 180.0 && eulers.y >= 180.0)
    {
        eulers.x -= 180.0;
        eulers.y -= 180.0;
    }
    if (eulers.y >= 180.0 && eulers.z >= 180.0)
    {
        eulers.z -= 180.0;
        eulers.y -= 180.0;
    }
    if (eulers.x >= 180.0 && eulers.z >= 180.0)
    {
        eulers.x -= 180.0;
        eulers.z -= 180.0;
    }
    return eulers;
}

glm::dquat tls::math::euler_deg_to_quat(glm::dvec3 const& eulers)
{
    return glm::qua(eulers * DegToRad);
}

glm::dquat tls::math::euler_rad_to_quat(glm::dvec3 const& eulers)
{
    return glm::qua(eulers);
}

glm::dvec3 tls::math::quat_to_euler_zyx_rad(const double q[4])
{
    glm::dvec4 glm_q(q[0], q[1], q[2], q[3]);

    return quat_to_euler_zyx_rad(glm_q);
}

glm::dvec3 tls::math::quat_to_euler_zyx_deg(const double q[4])
{
    glm::dvec4 glm_q(q[0], q[1], q[2], q[3]);

    return quat_to_euler_zyx_deg(glm_q);
}

glm::dquat tls::math::euler_zyx_to_quat(const double r[3])
{
    double cx = cos(r[0] * 0.5);
    double sx = sin(r[0] * 0.5);
    double cy = cos(r[1] * 0.5);
    double sy = sin(r[1] * 0.5);
    double cz = cos(r[2] * 0.5);
    double sz = sin(r[2] * 0.5);

    double w = (cz * cy * cx) + (sz * sy * sx);
    double x = (cz * cy * sx) - (sz * sy * cx);
    double y = (sz * cy * sx) + (cz * sy * cx);
    double z = (sz * cy * cx) - (cz * sy * sx);

    return glm::dquat(x, y, z, w);
}

glm::dquat tls::math::euler_zxz_to_quat(glm::dvec3 const& eulers)
{
    // Convention :
    // 'a' is the first angle (Oz)
    // 'b' is the second angle (Ox)
    // 'c' is the third angle (Oz)
    double ca = cos(eulers.x / 2.0);
    double sa = sin(eulers.x / 2.0);
    double cb = cos(eulers.y / 2.0);
    double sb = sin(eulers.y / 2.0);
    double cc = cos(eulers.z / 2.0);
    double sc = sin(eulers.z / 2.0);

    double ca_cc = ca * cc;
    double sa_sc = sa * sc;
    double ca_sc = ca * sc;
    double sa_cc = sa * cc;

    glm::dquat q{}; // q = { ca, 0, 0, sa } * { cb, sb, 0, 0 } * { cc, 0, 0, sc }
    //q.w = ca * cb * cc - sa * cb * sc;
    q.w = cb * (ca_cc - sa_sc);
    //q.x = ca * sb * cc + sa * sb * sc;
    q.x = sb * (ca_cc + sa_sc);
    //q.y = sa * sb * cc - ca * sb * sc;
    q.y = sb * (sa_cc - ca_sc);
    //q.z = sa * cb * cc + ca * cb * sc;
    q.z = cb * (sa_cc + ca_sc);
    return q;
}

glm::dvec3 tls::math::quat_to_euler_zxz(glm::dquat const& q)
{
    // Convention :
    // 'a' is the first angle (Oz)
    // 'b' is the second angle (Ox)
    // 'c' is the third angle (Oz)
    glm::dvec3 euler{};

    // Note : after a mathematical simplification we show that
    // q.w = cos(b/2) * cos((a+c)/2)
    // q.x = sin(b/2) * cos((a-c)/2)
    // q.y = sin(b/2) * sin((a-c)/2)
    // q.z = cos(b/2) * sin((a+c)/2)

    // Solve 'b' with b < 0
    double cosb = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
    euler.y = (cosb > 1.0) ? 0.0 : ((cosb < -1.0) ? -M_PI : -acos(cosb));

    // if 'b' ~= 0 or ~= Pi, 'a' and 'c' have almost the same axis (Oz is unchanged)
    if (fabs(euler.y) < 0.000001)
    {
        euler.x = 2.0 * atan2(q.z, q.w);
        euler.z = 0.0;
    }
    else if (fabs(euler.y) > M_PI - 0.000001)
    {
        euler.x = 2.0 * atan2(-q.y, -q.x);
        euler.z = 0.0;
    }
    else
    {
        // As -Pi <= b <= 0, cos(b/2) >= 0 and sin(b/2) <= 0
        double half_a_plus_c = atan2(q.z, q.w);
        double half_a_minus_c = atan2(-q.y, -q.x);

        euler.x = half_a_plus_c + half_a_minus_c;
        euler.z = half_a_plus_c - half_a_minus_c;
    }

    return euler;
}

// FIXME(robin) - Ça ne fonctionne pas, il faut que je repose les maths
glm::dquat tls::math::quat_from_3_vector(glm::dvec3 u, glm::dvec3 v, glm::dvec3 pseudoAxis)
{
    double norm_u = sqrt(dot(u, u));
    double norm_v = sqrt(dot(v, v));
    //double norm_u_norm_v = sqrt(dot(u, u) * dot(v, v));
    //double real_part = norm_u_norm_v + dot(u, v);

    glm::dvec3 bisectrice = u / norm_u + v / norm_v; // do not need to normalize
    glm::dvec3 orthoBisec = glm::cross(u, bisectrice);
    glm::dvec3 planNormal = glm::cross(bisectrice, orthoBisec);

    // Compute the projection of the _pseudoAxis_ on the equidistant plan
    glm::dvec3 A = glm::cross(pseudoAxis, planNormal);
    glm::dvec3 rotationAxis = glm::cross(planNormal, A);

    // Project _u_ and _v_ in the plan of normal _rotationAxis_
    glm::dvec3 projU = u - rotationAxis * dot(rotationAxis, u);
    glm::dvec3 projV = v - rotationAxis * dot(rotationAxis, v);

    double real_part = sqrt(dot(projU, projU) * dot(projV, projV)) + dot(projU, projV);

    return glm::normalize(glm::dquat(real_part, rotationAxis.x, rotationAxis.y, rotationAxis.z));
}

glm::dmat3 tls::math::getRotationMatrix(const double q[4])
{
    double xx = q[0] * q[0];
    double yy = q[1] * q[1];
    double zz = q[2] * q[2];
    double ww = q[3] * q[3];

    double xy = q[0] * q[1];
    double xz = q[0] * q[2];
    double xw = q[0] * q[3];
    double yz = q[1] * q[2];
    double yw = q[1] * q[3];
    double zw = q[2] * q[3];

    return glm::dmat3 {
        ww + xx - yy - zz,   2 * (zw + xy),      2 * (xz - yw),
        2 * (xy - zw),       ww - xx + yy - zz,  2 * (xw + yz),
        2 * (yw + xz),       2 * (yz - xw),      ww - xx - yy + zz };
}

/*glm::dvec3 tls::math::getOrientationVector(const glm::dvec3& inputVector)		//outputs a vector of angle (0;alpha;beta) such that if we rotate inputVector the result is (0;0;1)
{																		// said rotation starts with beta around z axis, then alpha around y axis
	if(inputVector[0]>0)
		return glm::dvec3(0.0, atan(sqrt(inputVector[0] * inputVector[0] + inputVector[1] * inputVector[1]) / inputVector[2]), atan(inputVector[1] / inputVector[0]));
	else
		return glm::dvec3(0.0, -atan(sqrt(inputVector[0] * inputVector[0] + inputVector[1] * inputVector[1]) / inputVector[2]), atan(inputVector[1] / inputVector[0]));
}*/