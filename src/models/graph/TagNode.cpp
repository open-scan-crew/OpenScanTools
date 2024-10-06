#include "models/graph/TagNode.h"
#include "services/MarkerDefinitions.hpp"
#include "utils/ColorConversion.h"

#include "controller/Controller.h"

TagNode::TagNode(const TagNode& node)
    : AClippingNode(node)
    , TagData(node)
{
    updateMarker();
}

TagNode::TagNode()
{
    setName(TEXT_DEFAULT_NAME_TAG.toStdWString());
    updateMarker();
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

std::unordered_set<Selection> TagNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    return {};
}

std::unordered_set<ManipulationMode> TagNode::getAcceptableManipulationModes() const
{
    return {};
}

void TagNode::setMarkerIcon(scs::MarkerIcon icon)
{
    m_markerIcon = icon;
    updateMarker();
}

void TagNode::setColor(const Color32& color)
{
    if (color == m_color)
        return;
    AObjectNode::setColor(color);
    updateMarker();
}

MarkerDrawData TagNode::getMarkerDrawData(const glm::dmat4& gTransfo) const
{
    // Compose the style
    uint32_t status = 0;
    if (m_selected)
        status |= 0x01;
    if (m_isHovered)
        status |= 0x02;
    if (m_showMarkerTrueColor)
        status |= 0x04;

    return {
        { (float)gTransfo[3][0], (float)gTransfo[3][1], (float)gTransfo[3][2] },
        { m_color.r, m_color.g, m_color.b, m_color.a },
        m_graphicId,
        (uint32_t)m_markerIcon,
        m_primitiveDef.firstVertex,
        m_primitiveDef.vertexCount,
        status
    };
}

void TagNode::setDefaultData(const Controller& controller)
{
    TagData::setDefaultData(controller.cgetContext());
    AClippingNode::setDefaultData(controller);
    updateMarker();
}

void TagNode::updateMarker()
{
    scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(m_markerIcon);
    m_showMarkerTrueColor = marker_style.showTrueColor;
    m_primitiveDef = scs::g_shapePrimitives.at(marker_style.shape);
}