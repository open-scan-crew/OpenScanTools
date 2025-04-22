#include "models/graph/PolylineMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

PolylineMeasureNode::PolylineMeasureNode()
    : AMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_POLYLINE.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::Polyline_Measure;
}

PolylineMeasureNode::PolylineMeasureNode(const PolylineMeasureNode& node)
    : AMeasureNode(node)
    , PolylineMeasureData(node)
{}

std::vector<Measure> PolylineMeasureNode::getMeasures() const
{
    return PolylineMeasureData::getMeasures();
}

ElementType PolylineMeasureNode::getType() const
{
    return ElementType::PolylineMeasure;
}

TreeType PolylineMeasureNode::getDefaultTreeType() const
{
    return TreeType::Measures;
}

void PolylineMeasureNode::getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const
{
    for (const Measure& measure : PolylineMeasureData::getMeasures())
    {
        glm::dvec4 gOri = gTransfo * glm::dvec4(measure.origin, 1.0);
        glm::dvec4 gFin = gTransfo * glm::dvec4(measure.final, 1.0);
        segments.push_back(SegmentDrawData(
            gOri,
            gFin,
            0x555555AA,
            m_graphicId,
            SHOW_MAIN_SEGMENT));
    }

    glm::dvec4 first = gTransfo * glm::dvec4(getFirstPos(), 1.0);
    glm::dvec4 last = gTransfo * glm::dvec4(getLastPos(), 1.0);

    segments.push_back(SegmentDrawData(
        first,
        last,
        0x00AABBCC,
        m_graphicId,
        SHOW_HORIZONTAL_SEGMENT | SHOW_VERTICAL_SEGMENT));
}
