#include "models/graph/PipeToPipeMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

PipeToPipeMeasureNode::PipeToPipeMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_PIPE_TO_PIPE.toStdWString());
}

PipeToPipeMeasureNode::PipeToPipeMeasureNode(const PipeToPipeMeasureNode& node)
    : AMeasureNode(node)
    , PipeToPipeMeasureData(node)
{
    // INFO
    /*
    auto* pipim = static_cast<UIPipeToPipeMeasure*>(autoMeasure);
    measurePoints.push_back(PointBufferData(pipim->getPipe1Center()));
    measurePoints.push_back(PointBufferData(pipim->getProjPoint()));
    measureColors.push_back(0xFFFFFFFF); // white
    measureIds.push_back(pipim->getId());
    measurePoints.push_back(PointBufferData(pipim->getProjPoint()));
    measurePoints.push_back(PointBufferData(pipim->getPipe2Center()));
    measureColors.push_back(0xFFFFFFFF); // white
    measureIds.push_back(pipim->getId());
    storedMeasure.insert({ pipim->getId(), pipim->getMeasures() });
    */
}

ElementType PipeToPipeMeasureNode::getType() const
{
    return ElementType::PipeToPipeMeasure;
}

TreeType PipeToPipeMeasureNode::getDefaultTreeType() const
{
    return TreeType::Measures;
}

std::vector<Measure> PipeToPipeMeasureNode::getMeasures() const
{
    return PipeToPipeMeasureData::getMeasures();
}

void PipeToPipeMeasureNode::getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const
{
    for (const Measure& measure : PipeToPipeMeasureData::getMeasures())
    {
        glm::dvec4 gOri = gTransfo * glm::dvec4(measure.origin, 1.0);
        glm::dvec4 gFin = gTransfo * glm::dvec4(measure.final, 1.0);
        segments.push_back(SegmentDrawData(
            gOri,
            gFin,
            0xA12345EE,
            m_graphicId,
            SHOW_ALL_SEGMENT));
    }
}

