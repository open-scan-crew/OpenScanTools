#include "models/graph/PointToPlaneMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

PointToPlaneMeasureNode::PointToPlaneMeasureNode()
    : AMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_POINT_TO_PLANE.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::PointToPlane_Measure;
}

PointToPlaneMeasureNode::PointToPlaneMeasureNode(const PointToPlaneMeasureNode& node)
	: AMeasureNode(node)
	, PointToPlaneMeasureData(node)
{
}

std::vector<Measure> PointToPlaneMeasureNode::getMeasures() const
{
    return PointToPlaneMeasureData::getMeasures();
}

ElementType PointToPlaneMeasureNode::getType() const
{
    return ElementType::PointToPlaneMeasure;
}

TreeType PointToPlaneMeasureNode::getDefaultTreeType() const
{
    return TreeType::Measures;
}

void PointToPlaneMeasureNode::getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const
{
    for (const Measure& measure : PointToPlaneMeasureData::getMeasures())
    {
        glm::dvec4 gOri = gTransfo * glm::dvec4(measure.origin, 1.0);
        glm::dvec4 gFin = gTransfo * glm::dvec4(measure.final, 1.0);
        segments.push_back(SegmentDrawData(
            gOri,
            gFin,
            0xDEAD00FF,
            m_graphicId,
            SHOW_ALL_SEGMENT));
    }
}