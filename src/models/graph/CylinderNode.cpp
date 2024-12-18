#include "models/graph/CylinderNode.h"
#include "vulkan/MeshManager.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/texts/DefaultNameTexts.hpp"

CylinderNode::CylinderNode(const CylinderNode& node)
    : SimpleObjectNode(node)
    , StandardRadiusData(node)
{
    addGenericMeshInstance();
}

CylinderNode::CylinderNode(const double& detectedRadius)
    : SimpleObjectNode()
    , StandardRadiusData(detectedRadius)
{
    addGenericMeshInstance();
    setName(TEXT_DEFAULT_NAME_PIPE.toStdWString());
}

CylinderNode::CylinderNode()
    : SimpleObjectNode()
{
    addGenericMeshInstance();
    setName(TEXT_DEFAULT_NAME_PIPE.toStdWString());
}

//FixMe (aurélien) Should be set here not in parent class
/*
bool CylinderNode::isAcceptingManipulatorMode(const ManipulationMode& mode) const
{
    if(MeshManager::SimpleObjectAcceptableManipulators.find(m_data->m_id.def.type)!= MeshManager::SimpleObjectAcceptableManipulators.end()
        && MeshManager::SimpleObjectAcceptableManipulators.at(m_data->m_id.def.type).find(mode) != MeshManager::SimpleObjectAcceptableManipulators.at(m_data->m_id.def.type).end())
            return MeshManager::SimpleObjectAcceptableManipulators.at(m_data->m_id.def.type).at(mode);
    return (false);
}

std::unordered_set<Selection> CylinderNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    if (MeshManager::SimpleObjectManipulatorSelections.find(m_data->m_id.def.type) != MeshManager::SimpleObjectManipulatorSelections.end()
        && MeshManager::SimpleObjectManipulatorSelections.at(m_data->m_id.def.type).find(mode) != MeshManager::SimpleObjectManipulatorSelections.at(m_data->m_id.def.type).end())
        return MeshManager::SimpleObjectManipulatorSelections.at(m_data->m_id.def.type).at(mode);
    return {};
}
*/

ElementType CylinderNode::getType() const
{
    return ElementType::Cylinder;
}

TreeType CylinderNode::getDefaultTreeType() const
{
    return TreeType::Pipe;
}

void CylinderNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    float r = getRadius();
    glm::vec4 params;
    params.x = r + m_minClipDist;
    params.y = r + m_maxClipDist;
    params.z = getLength() / 2;
    params.w = 0.f; // not used

    std::shared_ptr<IClippingGeometry> geom = std::make_shared<CylinderClippingGeometry>(m_clippingMode, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->isSelected = m_selected;

    if (m_clippingMode == ClippingMode::showInterior)
        clipAssembly.clippingUnion.push_back(geom);
    else
        clipAssembly.clippingIntersection.push_back(geom);
}

void CylinderNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
    float r = getRadius();
    glm::vec4 params;
    params.x = r + m_rampMin;
    params.y = r + m_rampMax;
    params.z = getLength() / 2;
    params.w = r; // not used

    auto geom = std::make_shared<CylinderClippingGeometry>(ClippingMode::showInterior, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->color = getColor().toVector();
    geom->isSelected = m_selected;
    retGeom.push_back(geom);
}

void CylinderNode::setDefaultData(const Controller& controller)
{
    AClippingNode::setDefaultData(controller);
    setInsulationRadius(controller.cgetContext().getPipeDetectionOptions().insulatedThickness);
    setStandard(controller.cgetContext().getCurrentStandard(StandardType::Pipe));
}

void CylinderNode::addGenericMeshInstance()
{
    MeshManager::getInstance().getCylinderId(m_meshId);
}

double CylinderNode::getLength() const
{
    return m_length;
}

double CylinderNode::getRadius() const
{
    double radius = 0.0;
    switch (m_diameterSet)
    {
        case DiameterSet::Detected:
        {
            radius = m_detectedRadius - m_insulationRadius;
        }
        break;
        case DiameterSet::Forced:
        {
            radius = m_forcedRadius;
        }
        break;
        case DiameterSet::Standard:
        {
            radius = m_standardRadius;
        }
        break;
    }
    return radius;
}

void CylinderNode::setLength(double length)
{
    m_length = length;

    updateScale();
}

void CylinderNode::updateScale()
{
    double radius = getRadius();
    m_scale.x = radius;
    m_scale.y = radius;

    m_scale.z = m_length / 2;
}

void CylinderNode::addScale(const glm::dvec3& addScale)
{
    m_length += 2 * addScale.z;

    updateScale();
}