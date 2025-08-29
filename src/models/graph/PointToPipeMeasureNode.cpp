#include "models/graph/PointToPipeMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

PointToPipeMeasureNode::PointToPipeMeasureNode()
    : AMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_POINT_TO_PIPE.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::PointToPipe_Measure;
}

PointToPipeMeasureNode::PointToPipeMeasureNode(const PointToPipeMeasureNode& node)
	: AMeasureNode(node)
	, PointToPipeMeasureData(node)
{
}

std::vector<Measure> PointToPipeMeasureNode::getMeasures() const
{
    return PointToPipeMeasureData::getMeasures();
}

ElementType PointToPipeMeasureNode::getType() const
{
    return ElementType::PointToPipeMeasure;
}

TreeType PointToPipeMeasureNode::getDefaultTreeType() const
{
    return TreeType::Measures;
}

void PointToPipeMeasureNode::getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const
{
    for (const Measure& measure : PointToPipeMeasureData::getMeasures())
    {
        glm::dvec4 gOri = gTransfo * glm::dvec4(measure.origin, 1.0);
        glm::dvec4 gFin = gTransfo * glm::dvec4(measure.final, 1.0);
        segments.push_back(SegmentDrawData(
            gOri,
            gFin,
            0xFFFFFFFF,
            graphic_id_,
            SHOW_ALL_SEGMENT));
    }
}