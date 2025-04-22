#include "models/graph/PipeToPlaneMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

PipeToPlaneMeasureNode::PipeToPlaneMeasureNode(const PipeToPlaneMeasureNode& node)
	: AMeasureNode(node)
	, PipeToPlaneMeasureData(node)
{
}

PipeToPlaneMeasureNode::PipeToPlaneMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_PIPE_TO_PLANE.toStdWString());
    Data::marker_icon_ = scs::MarkerIcon::PipeToPlane_Measure;
}

std::vector<Measure> PipeToPlaneMeasureNode::getMeasures() const
{
    return PipeToPlaneMeasureData::getMeasures();
}

ElementType PipeToPlaneMeasureNode::getType() const
{
    return ElementType::PipeToPlaneMeasure;
}

TreeType PipeToPlaneMeasureNode::getDefaultTreeType() const
{
    return TreeType::Measures;
}

void PipeToPlaneMeasureNode::getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const
{
    for (const Measure& measure : PipeToPlaneMeasureData::getMeasures())
    {
        glm::dvec4 gOri = gTransfo * glm::dvec4(measure.origin, 1.0);
        glm::dvec4 gFin = gTransfo * glm::dvec4(measure.final, 1.0);
        segments.push_back(SegmentDrawData(
            gOri,
            gFin,
            0xFF00DDFF,
            m_graphicId,
            SHOW_ALL_SEGMENT));
    }
}