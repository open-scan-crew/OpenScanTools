#include "models/graph/SphereNode.h"
#include "vulkan/MeshManager.h"
#include "gui/texts/DefaultNameTexts.hpp"

SphereNode::SphereNode(const SphereNode& node)
    : SimpleObjectNode(node)
    , m_radius(node.getRadius())
{
    m_scale = glm::dvec3(m_radius);
    addGenericMeshInstance();
}

SphereNode::SphereNode(const double& detectedRadius)
    : SimpleObjectNode()
    , m_radius(detectedRadius)
{
    m_scale = glm::dvec3(m_radius);
    addGenericMeshInstance();
    setName(TEXT_DEFAULT_NAME_SPHERE.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::Sphere;
}

SphereNode::SphereNode()
    : SimpleObjectNode()
    , m_radius(1.)
{
    m_scale = glm::dvec3(m_radius);
    addGenericMeshInstance();
    setName(TEXT_DEFAULT_NAME_SPHERE.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::Sphere;
}

void SphereNode::addGenericMeshInstance()
{
    MeshManager::getInstance().getSphereId(m_meshId);
}

ElementType SphereNode::getType() const
{
    return ElementType::Sphere;
}

TreeType SphereNode::getDefaultTreeType() const
{
    return TreeType::Sphere;
}

void SphereNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = getRadius() + m_minClipDist;
    params.y = getRadius() + m_maxClipDist;
    params.z = 0.f; // not used
    params.w = 0.f; // not used

    std::shared_ptr<IClippingGeometry> geom = std::make_shared<SphereClippingGeometry>(m_clippingMode, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->isSelected = m_selected;

    if (m_clippingMode == ClippingMode::showInterior)
        clipAssembly.clippingUnion.push_back(geom);
    else
        clipAssembly.clippingIntersection.push_back(geom);
}


void SphereNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = getRadius() + m_rampMin;
    params.y = getRadius() + m_rampMax;
    params.z = 0.f;
    params.w = getRadius(); // utilisé pour l'échelle

    auto geom = std::make_shared<SphereClippingGeometry>(ClippingMode::showInterior, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->color = getColor().toVector();
    geom->isSelected = m_selected;
    retGeom.push_back(geom);
}

void SphereNode::setRadius(double radius)
{
    m_scale = glm::dvec3(radius);
    m_radius = radius;
}

double SphereNode::getRadius() const
{
    return m_radius;
}