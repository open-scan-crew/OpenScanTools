#include "models/3d/Graph/TransformationModule.h"

#include "utils/math/trigo.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

TransformationModule::TransformationModule()
    : m_center(0.0)
    , m_quaternion(1, 0, 0, 0)
    , m_scale(1.0)
{}

TransformationModule::TransformationModule(const glm::dvec3& center, const glm::dquat& quaternion, const glm::dvec3& size)
    : m_center(center)
    , m_quaternion(quaternion)
    , m_scale(size)
{}

TransformationModule::TransformationModule(const glm::dmat4& matrix)
    : m_center(0.0)
    , m_quaternion(1, 0, 0, 0) // (w, x, y, z) is the glm order
    , m_scale(1.0)
{
    glm::dvec3 skew;
    glm::dvec4 perspective;
    glm::decompose(matrix, m_scale, m_quaternion, m_center, skew, perspective);
}

TransformationModule::TransformationModule(const TransformationModule& module)
    : m_center(module.m_center)
    , m_quaternion(module.m_quaternion)
    , m_scale(module.m_scale)
{}

TransformationModule::~TransformationModule()
{}

void TransformationModule::getTrRtSc(const glm::dmat4& mat, glm::dvec3& translation, glm::dquat& qrotation, glm::dvec3& scale)
{
    glm::dvec3 skew;
    glm::dvec4 perspective;
    glm::decompose(mat, scale, qrotation, translation, skew, perspective);
}

void TransformationModule::getTrRtSc(const glm::mat4& mat, glm::vec3& translation, glm::quat& qrotation, glm::vec3& scale)
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, qrotation, translation, skew, perspective);
}

const glm::dvec3& TransformationModule::getCenter() const
{
    return m_center;
}

const glm::dquat& TransformationModule::getOrientation() const
{
    return m_quaternion;
}

const glm::dvec3& TransformationModule::getScale() const
{
    return m_scale;
}

glm::dvec3 TransformationModule::getSize() const
{
    return m_scale * 2.0;
}

void TransformationModule::addLocalTranslation(const glm::dvec3& translation)
{
    glm::dmat3 rotation = glm::mat3_cast(m_quaternion);
    m_center += rotation * translation;
}

void TransformationModule::addGlobalTranslation(const glm::dvec3& translation)
{
    m_center += translation;
}

void TransformationModule::setPosition(const glm::dvec3& position)
{
    m_center = position;
}

void TransformationModule::addPhiAndTheta(const double& phi, const double& theta)
{
    m_quaternion *= glm::dquat(glm::vec3(phi, theta, 0.0));
}

void TransformationModule::addPreRotation(const glm::dquat& qrotation)
{
    m_quaternion *= qrotation;
}

void TransformationModule::addPostRotation(const glm::dquat& qrotation)
{
    m_quaternion = qrotation * m_quaternion;
}

void TransformationModule::addPostRotation(const glm::dquat& qrotation, const glm::dvec3& relativeRotationCenter)
{
    glm::dvec3 decalCenter = m_center + relativeRotationCenter;
    m_center -= decalCenter;
    glm::dmat4 newTransfo(glm::mat4_cast(qrotation) * getTransformation());
    getTrRtSc(newTransfo, m_center, m_quaternion, m_scale);
    m_center += decalCenter;
}

void TransformationModule::addPreRotation(const glm::dmat3& rotation)
{
    glm::dmat4 transfo(rotation);
}

void TransformationModule::setPhiAndTheta(const double& phi, const double& theta)
{
    m_quaternion = glm::dquat(glm::vec3(phi, theta, 0.0));
}

void TransformationModule::setRotation(const glm::dmat3& mrotation)
{
    m_quaternion = glm::dquat(mrotation);
}

void TransformationModule::setRotation(const glm::dquat& qrotation)
{
    m_quaternion = qrotation;
}

void TransformationModule::addScale(const glm::dvec3& scale)
{
    m_scale += scale;
}

void TransformationModule::setScale(const glm::dvec3& scale)
{
    m_scale = scale;
}

void TransformationModule::setSize(const glm::dvec3& size)
{
    m_scale = size / 2.0;
}

void TransformationModule::addTransformation(const glm::dmat4& transformation)
{
    glm::dmat4 mat(transformation * getTransformation());
    getTrRtSc(mat, m_center, m_quaternion, m_scale);
}

void TransformationModule::compose_right(const TransformationModule& rhs)
{
    glm::dmat3 R(m_quaternion);
    m_center += R * rhs.m_center;
    m_scale *= rhs.m_scale;
    m_quaternion = m_quaternion * rhs.m_quaternion;
}

void TransformationModule::compose_left(const TransformationModule& lhs)
{
    glm::dmat3 R(lhs.m_quaternion);
    m_center = R * m_center + lhs.m_center;
    m_scale *= lhs.m_scale;
    m_quaternion = lhs.m_quaternion * m_quaternion;
}

void TransformationModule::compose_inverse_right(const TransformationModule& rhs)
{
    m_quaternion = m_quaternion * glm::conjugate(rhs.m_quaternion);
    //glm::dvec3 rhs_t__1 = -glm::mat3_cast(glm::conjugate(rhs.m_quaternion)) * rhs.m_center;
    m_center -= glm::mat3_cast(m_quaternion) * rhs.m_center;
    m_scale /= rhs.m_scale;
}

void TransformationModule::compose_inverse_left(const TransformationModule& lhs)
{
    m_quaternion = glm::conjugate(lhs.m_quaternion) * m_quaternion;
    m_center = glm::mat3_cast(glm::conjugate(lhs.m_quaternion)) * (m_center - lhs.m_center);
    m_scale /= lhs.m_scale;
}

void TransformationModule::setTransformation(const glm::dmat4& transformation)
{
    getTrRtSc(transformation, m_center, m_quaternion, m_scale);
}

void TransformationModule::setTransformationModule(const TransformationModule& transformation)
{
    m_center = transformation.getCenter();
    m_quaternion = transformation.getOrientation();
    m_scale = transformation.getScale();
}

glm::dmat4 TransformationModule::getTransformation() const
{
    glm::dmat4 translation = { 1.0f, 0.f, 0.f, 0.f,
                              0.f, 1.0f, 0.f, 0.f,
                              0.f, 0.f, 1.0f, 0.f,
                              m_center.x, m_center.y, m_center.z, 1.f };

    glm::dmat4 scale = { m_scale.x, 0.f, 0.f, 0.f,
                        0.f, m_scale.y, 0.f, 0.f,
                        0.f, 0.f, m_scale.z, 0.f,
                        0.f, 0.f, 0.f, 1.f };

    // Rotation by quaternion
    glm::dmat4 rotMat = glm::mat4_cast(m_quaternion);
    return (translation * rotMat * scale);
}

glm::dmat4 TransformationModule::getInverseTransformation() const
{
    glm::dmat4 translation_inv = { 1.0f, 0.f, 0.f, 0.f,
                                  0.f, 1.0f, 0.f, 0.f,
                                  0.f, 0.f, 1.0f, 0.f,
                                  -m_center.x, -m_center.y, -m_center.z, 1.f };

    
    glm::dmat4 scale_inv = { 1.0 / m_scale.x, 0.f, 0.f, 0.f,
                            0.f,  1.0 / m_scale.y, 0.f, 0.f,
                            0.f, 0.f, 1.0 / m_scale.z, 0.f,
                            0.f, 0.f, 0.f, 1.f };

    // Rotation by quaternion
    glm::dmat4 rotMat_inv = glm::transpose(glm::mat4_cast(m_quaternion));

    return (scale_inv * rotMat_inv * translation_inv);
}

glm::dmat4 TransformationModule::getInverseRotationTranslation() const
{
    glm::dmat4 translation_inv = { 1.0f, 0.f, 0.f, 0.f,
                              0.f, 1.0f, 0.f, 0.f,
                              0.f, 0.f, 1.0f, 0.f,
                              -m_center.x, -m_center.y, -m_center.z, 1.f };
    // Rotation by quaternion
    glm::dmat4 rotMat_inv = glm::transpose(glm::mat4_cast(m_quaternion));

    return (rotMat_inv * translation_inv);
}

bool TransformationModule::operator==(const TransformationModule& rhs) const
{
    return (this->m_center == rhs.m_center) &&
           (this->m_quaternion == rhs.m_quaternion) &&
           (this->m_scale == rhs.m_scale);
}

TransformationModule TransformationModule::getTransformationModule() const
{
    return TransformationModule(*this);
}

TransformationModule TransformationModule::operator*(const TransformationModule& rhs) const
{
    TransformationModule result;
    result.m_center = this->m_center + glm::mat3_cast(this->m_quaternion) * rhs.m_center;
    result.m_scale = this->m_scale * rhs.m_scale;
    result.m_quaternion = this->m_quaternion * rhs.m_quaternion;
    return result;
}
