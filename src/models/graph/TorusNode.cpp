#include "models/graph/TorusNode.h"
#include "models/3d/ManipulationTypes.h"
#include "vulkan/MeshManager.h"
#include "gui/texts/DefaultNameTexts.hpp"

TorusNode::TorusNode(const TorusNode& node)
    : AClippingNode(node)
    , TorusData(node)
    , m_meshId(node.m_meshId)
{}

TorusNode::TorusNode(const double& mainAngle, const double& mainRadius, const double& tubeRadius, const double& insulatedRadius)
    : TorusData(mainAngle, mainRadius, tubeRadius)
{
    setInsulationRadius(insulatedRadius);
    setName(TEXT_DEFAULT_NAME_TORUS.toStdWString());
    updateTorusMesh();
}

TorusNode::TorusNode()
    : TorusData(0., 0., 0.)
{}

TorusNode::~TorusNode()
{
    if(!m_isDead)
        MeshManager::getInstance().removeMeshInstance(m_meshId);
}

ElementType TorusNode::getType() const
{
    return ElementType::Torus;
}

TreeType TorusNode::getDefaultTreeType() const
{
    return TreeType::Pipe;
}

void TorusNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    float radius = getAdjustedTubeRadius();
    glm::vec4 params;
    params.x = m_mainRadius;
    params.y = std::cos(m_mainAngle);
    params.z = radius + m_minClipDist;
    params.w = radius + m_maxClipDist;

    std::shared_ptr<IClippingGeometry> geom = std::make_shared<TorusClippingGeometry>(m_clippingMode, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->isSelected = m_selected;

    if (m_clippingMode == ClippingMode::showInterior)
        clipAssembly.clippingUnion.push_back(geom);
    else
        clipAssembly.clippingIntersection.push_back(geom);
}

void TorusNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_mainRadius;
    params.y = std::cos(m_mainAngle);
    params.z = getAdjustedTubeRadius() + m_rampMin;
    params.w = getAdjustedTubeRadius() + m_rampMax;
    auto geom = std::make_shared<TorusClippingGeometry>(ClippingMode::showInterior, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->color = getColor().toVector();
    geom->isSelected = m_selected;
    retGeom.push_back(geom);
}

void TorusNode::setDead(bool isDead)
{
    if (!m_isDead && isDead)
        MeshManager::getInstance().removeMeshInstance(m_meshId);
    if (m_isDead && !isDead)
        MeshManager::getInstance().getTorusId(m_meshId, (float)m_mainAngle, (float)m_mainRadius, (float)getAdjustedTubeRadius());

    AGraphNode::setDead(isDead);
}

void TorusNode::updateTorusMesh()
{
    MeshManager::getInstance().removeMeshInstance(m_meshId);
    MeshManager::getInstance().getTorusId(m_meshId, (float)m_mainAngle, (float)m_mainRadius, (float)getAdjustedTubeRadius());
}

std::unordered_set<Selection> TorusNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    switch (mode)
    {
    case ManipulationMode::Translation:
        return { Selection::X, Selection::Y, Selection::Z };
    case ManipulationMode::Rotation:
    case ManipulationMode::Extrusion:
    case ManipulationMode::Scale:
    default:
        return {};
    };
}

std::unordered_set<ManipulationMode> TorusNode::getAcceptableManipulationModes() const
{
    return { ManipulationMode::Translation };
}

MeshId TorusNode::getMeshId() const
{
    return m_meshId;
}

MeshDrawData TorusNode::getMeshDrawData(const glm::dmat4& gTransfo) const
{
    MeshDrawData meshDrawData = AObjectNode::getMeshDrawData(gTransfo);
    meshDrawData.meshBuffer = MeshManager::getInstance().getMesh(m_meshId).m_mesh;
    return meshDrawData;
}
