#ifndef TRANSFORMATION_MODULE_H_
#define TRANSFORMATION_MODULE_H_

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class TransformationModule
{
public:
    TransformationModule();
    TransformationModule(const glm::dvec3& center, const glm::dquat& quaternion, const glm::dvec3& size);
    TransformationModule(const glm::dmat4& matrix);
    TransformationModule(const TransformationModule& module);
    ~TransformationModule();

    static void getTrRtSc(const glm::dmat4& mat, glm::dvec3& translation, glm::dquat& qrotation, glm::dvec3& scale);
    static void getTrRtSc(const glm::mat4& mat, glm::vec3& translation, glm::quat& qrotation, glm::vec3& scale);

    const glm::dvec3& getCenter() const;
    const glm::dquat& getOrientation() const; 
    const glm::dvec3& getScale() const;
    glm::dvec3 getSize() const;

    void addLocalTranslation(const glm::dvec3& translation);
    void addGlobalTranslation(const glm::dvec3& translation);
    virtual void setPosition(const glm::dvec3& position);

    void addPhiAndTheta(const double& phi, const double& theta);
    void addPreRotation(const glm::dquat& qrotation);
    void addPostRotation(const glm::dquat& qrotation);
    void addPostRotation(const glm::dquat& qrotation, const glm::dvec3& relativeRotationCenter);
    void addPreRotation(const glm::dmat3& mrotation);
    void setPhiAndTheta(const double& phi, const double& theta);
    void setRotation(const glm::dmat3& mrotation);
    virtual void setRotation(const glm::dquat& qrotation);

    virtual void addScale(const glm::dvec3& scale);
    virtual void setScale(const glm::dvec3& scale);
    // Use this fonction if you intend to use the scale as a size.
    // Internally, set the scale as the (size / 2)
    virtual void setSize(const glm::dvec3& size);

    void addTransformation(const glm::dmat4& transformation);
    void compose_right(const TransformationModule& rhs);
    void compose_left(const TransformationModule& lhs);
    void compose_inverse_right(const TransformationModule& rhs);
    void compose_inverse_left(const TransformationModule& lhs);
    void setTransformation(const glm::dmat4& transformation);
    void setTransformationModule(const TransformationModule& transformation);

    TransformationModule getTransformationModule() const;
    glm::dmat4 getTransformation() const;
    glm::dmat4 getInverseTransformation() const;
    glm::dmat4 getInverseRotationTranslation() const;

    bool operator==(const TransformationModule& rhs) const;
    TransformationModule operator*(const TransformationModule& rhs) const;

protected:
    glm::dvec3 m_center;
    glm::dquat m_quaternion;
    glm::dvec3 m_scale;
};
#endif // !TRANSFORMATION_MODULE_H_
