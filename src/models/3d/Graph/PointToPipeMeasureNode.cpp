#include "models/3d/Graph/PointToPipeMeasureNode.h"

PointToPipeMeasureNode::PointToPipeMeasureNode()
    : AMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_POINT_TO_PIPE.toStdWString());
}

PointToPipeMeasureNode::PointToPipeMeasureNode(const PointToPipeMeasureNode& node)
	: AMeasureNode(node)
	, PointToPipeMeasureData(node)
{
    // PREVIOUS CODE (from MeasureDrawData.cpp)  - Pourquoi
    /*
    measurePoints.push_back(PointBufferData(poplm->getPipeCenter()));
    measurePoints.push_back(PointBufferData(poplm->getProjPoint()));
    measureColors.push_back(0xFFFFFFFF); // white
    measureIds.push_back(poplm->getId());
    measurePoints.push_back(PointBufferData(poplm->getProjPoint()));
    measurePoints.push_back(PointBufferData(poplm->getPointCoord()));
    measureColors.push_back(0xFFFFFFFF); // white
    measureIds.push_back(poplm->getId());
    storedMeasure.insert({ poplm->getId(), poplm->getMeasures() });
    */
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
            { gOri.x, gOri.y, gOri.z },
            { gFin.x, gFin.y, gFin.z },
            0xFFFFFFFF,
            m_graphicId,
            SHOW_ALL_SEGMENT));
    }
}