#include "models/graph/PointNode.h"
#include "services/MarkerDefinitions.hpp"

PointNode::PointNode(const PointNode& node)
    : AClippingNode(node)
{
    updateMarker();
}

PointNode::PointNode()
{
    setName(TEXT_DEFAULT_NAME_POINT.toStdWString());
    updateMarker();
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

std::unordered_set<Selection> PointNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    return {};
}

std::unordered_set<ManipulationMode> PointNode::getAcceptableManipulationModes() const
{
    return {};
}

void PointNode::setColor(const Color32& color)
{
    if (color == m_color)
        return;
    AObjectNode::setColor(color);
}

scs::MarkerIcon PointNode::getIconType() const
{
    return scs::MarkerIcon::PointObject;
}

MarkerDrawData PointNode::getMarkerDrawData(const glm::dmat4& gTransfo) const
{
    // Compose the style
    uint32_t status = 0;
    if (m_selected)
        status |= 0x01;
    if (m_isHovered)
        status |= 0x02;
    if (scs::markerStyleDefs.at(scs::MarkerIcon::PointObject).showTrueColor)
        status |= 0x04;

    return {
        { (float)gTransfo[3][0], (float)gTransfo[3][1], (float)gTransfo[3][2] },
        { m_color.r, m_color.g, m_color.b, m_color.a },
        m_graphicId,
        (uint32_t)scs::MarkerIcon::PointObject,
        m_primitiveDef.firstVertex,
        m_primitiveDef.vertexCount,
        status
    };
}

void PointNode::updateMarker()
{
    scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(scs::MarkerIcon::PointObject);
    m_showMarkerTrueColor = marker_style.showTrueColor;
    m_primitiveDef = scs::g_shapePrimitives.at(marker_style.shape);
}