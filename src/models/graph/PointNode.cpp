#include "models/graph/PointNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

PointNode::PointNode(const PointNode& node)
    : AClippingNode(node)
{
}

PointNode::PointNode()
{
    setName(TEXT_DEFAULT_NAME_POINT.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::PointObject;
}

PointNode::~PointNode()
{
}

ElementType PointNode::getType() const
{
    return ElementType::Point;
}

TreeType PointNode::getDefaultTreeType() const
{
    return TreeType::Point;
}

void PointNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_minClipDist;
    params.y = m_maxClipDist;

    std::shared_ptr<IClippingGeometry> geom = std::make_shared<SphereClippingGeometry>(m_clippingMode, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->isSelected = m_selected;
    geom->clipperPhase = getPhase();

    if (m_clippingMode == ClippingMode::showInterior)
        clipAssembly.clippingUnion.push_back(geom);
    else
        clipAssembly.clippingIntersection.push_back(geom);

}

void PointNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_rampMin;
    params.y = m_rampMax;

    auto geom = std::make_shared<SphereClippingGeometry>(ClippingMode::showInterior, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->color = getColor().toVector();
    geom->isSelected = m_selected;
    retGeom.push_back(geom);
}

std::unordered_set<Selection> PointNode::getAcceptableSelections(ManipulationMode mode) const
{
    return {};
}
