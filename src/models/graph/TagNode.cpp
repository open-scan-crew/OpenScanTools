#include "models/graph/TagNode.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/texts/DefaultNameTexts.hpp"

TagNode::TagNode(const TagNode& node)
    : AClippingNode(node)
    , TagData(node)
{
}

TagNode::TagNode()
{
    setName(TEXT_DEFAULT_NAME_TAG.toStdWString());
    // Default init, can be changed later using the ControllerContext default icon.
    Data::marker_icon_ = scs::MarkerIcon::Tag_Base;
}

TagNode::~TagNode()
{
}

ElementType TagNode::getType() const
{
    return ElementType::Tag;
}

TreeType TagNode::getDefaultTreeType() const
{
    return TreeType::Tags;
}

void TagNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_minClipDist;
    params.y = m_maxClipDist;

    std::shared_ptr<IClippingGeometry> geom = std::make_shared<SphereClippingGeometry>(m_clippingMode, transfo.getInverseRotationTranslation(), params, 0);
    geom->isSelected = m_selected;

    if (m_clippingMode == ClippingMode::showInterior)
        clipAssembly.clippingUnion.push_back(geom);
    else
        clipAssembly.clippingIntersection.push_back(geom);
}

void TagNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
    glm::vec4 params;
    params.x = m_rampMin;
    params.y = m_rampMax;

    auto geom = std::make_shared<SphereClippingGeometry>(ClippingMode::showInterior, transfo.getInverseRotationTranslation(), params, m_rampSteps);
    geom->color = getColor().toVector();
    geom->isSelected = m_selected;
    retGeom.push_back(geom);
}

std::unordered_set<Selection> TagNode::getAcceptableSelections(ManipulationMode mode) const
{
    return {};
}

void TagNode::setDefaultData(const Controller& controller)
{
    TagData::setDefaultData(controller.cgetContext());
    AClippingNode::setDefaultData(controller);
    setMarkerIcon(controller.cgetContext().getActiveIcon());
}

