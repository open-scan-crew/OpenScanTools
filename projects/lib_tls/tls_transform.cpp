#include "tls_transform.h"


glm::mat4 tls::transform::getTransformMatrix(const double translation[3], const double q[4])
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

    glm::mat4 translation_rotation = {
        ww + xx - yy - zz,   2 * (zw + xy),      2 * (xz - yw),       0.f,
        2 * (xy - zw),       ww - xx + yy - zz,  2 * (xw + yz),       0.f,
        2 * (yw + xz),       2 * (yz - xw),      ww - xx - yy + zz,   0.f,
        translation[0],      translation[1],     translation[2],      1.f };

    return (translation_rotation);
}

glm::dmat4 tls::transform::getTransformDMatrix(const double translation[3], const double q[4])
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

    glm::dmat4 translation_rotation = {
        ww + xx - yy - zz,   2 * (zw + xy),      2 * (xz - yw),       0.f,
        2 * (xy - zw),       ww - xx + yy - zz,  2 * (xw + yz),       0.f,
        2 * (yw + xz),       2 * (yz - xw),      ww - xx - yy + zz,   0.f,
        translation[0],      translation[1],     translation[2],      1.f };

    return (translation_rotation);
}

glm::mat4 tls::transform::getInverseTransformMatrix(const double translation[3], const double q[4])
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

    glm::mat4 rotation_inv = {
        ww + xx - yy - zz,   2 * (xy - zw),      2 * (yw + xz),       0.f,
        2 * (zw + xy),       ww - xx + yy - zz,  2 * (yz - xw),       0.f,
        2 * (xz - yw),       2 * (xw + yz),      ww - xx - yy + zz,   0.f,
        0.f,     0.f,    0.f,     1.f };

    glm::mat4 translation_inv = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        -translation[0], -translation[1], -translation[2], 1.f, };

    return (rotation_inv * translation_inv);
}

glm::dmat4 tls::transform::getInverseTransformDMatrix(const double translation[3], const double q[4])
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

    glm::dmat4 rotation_inv = {
        ww + xx - yy - zz,   2 * (xy - zw),      2 * (yw + xz),       0.f,
        2 * (zw + xy),       ww - xx + yy - zz,  2 * (yz - xw),       0.f,
        2 * (xz - yw),       2 * (xw + yz),      ww - xx - yy + zz,   0.f,
        0.f,     0.f,    0.f,     1.f };

    glm::dmat4 translation_inv = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        -translation[0], -translation[1], -translation[2], 1.f, };

    return (rotation_inv * translation_inv);
}

glm::mat4 tls::transform::getTransformMatrix(const double translation[3], const double q[4], const double scale[3])
{
    glm::mat4 translation_rotation = getTransformMatrix(translation, q);

    glm::mat4 scaleM = { scale[0], 0.f, 0.f, 0.f,
                        0.f, scale[1], 0.f, 0.f,
                        0.f, 0.f, scale[2], 0.f,
                        0.f, 0.f, 0.f, 1.f };

    return (translation_rotation * scaleM);
}

glm::mat4 tls::transform::getInverseTransformMatrix(const double translation[3], const double q[4], const double scale[3])
{
    glm::mat4 rotation_translation_inv = getInverseTransformMatrix(translation, q);

    glm::mat4 scale_inv = { 1 / scale[0], 0.f, 0.f, 0.f,
                            0.f, 1 / scale[1], 0.f, 0.f,
                            0.f, 0.f, 1 / scale[2], 0.f,
                            0.f, 0.f, 0.f, 1.f };

    return (scale_inv * rotation_translation_inv);
}
