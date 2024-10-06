#include "models/graph/SimpleMeasureNode.h"
#include "utils/math/trigo.h"
#include <glm/gtx/rotate_vector.hpp>

SimpleMeasureNode::SimpleMeasureNode()
    : AMeasureNode()
{
    setName(TEXT_DEFAULT_NAME_SIMPLE_MEASURE.toStdWString());
}

SimpleMeasureNode::SimpleMeasureNode(const SimpleMeasureNode& node)
    : AMeasureNode(node)
    , SimpleMeasureData(node)
{}

std::vector<Measure> SimpleMeasureNode::getMeasures() const
{
    return SimpleMeasureData::getMeasures();
}

ElementType SimpleMeasureNode::getType() const
{
    return ElementType::SimpleMeasure;
}

TreeType SimpleMeasureNode::getDefaultTreeType() const
{
    return TreeType::Measures;
}

TransformationModule SimpleMeasureNode::updateClipping()
{
    TransformationModule mod;
    glm::dvec3 vect(glm::normalize(getDestinationPos() - getOriginPos()));
    vect.z *= -1;
    mod.setRotation(glm::qua(glm::orientation(glm::dvec3(0.0, 0.0, -1.0), vect)));
    mod.setSize(glm::dvec3(0.001,0.001,glm::distance(getDestinationPos(), getOriginPos())));

    glm::dmat4 tranfo = getCumulatedTransformation();
    mod.setPosition(tranfo * glm::dvec4((getDestinationPos() + getOriginPos()) / 2.0, 1.0));

    return mod;
}

void SimpleMeasureNode::getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const
{
    glm::dvec4 gOri = gTransfo * glm::dvec4(m_measure.origin, 1.0);
    glm::dvec4 gFin = gTransfo * glm::dvec4(m_measure.final, 1.0);
    segments.push_back(SegmentDrawData(
        { gOri.x, gOri.y, gOri.z },
        { gFin.x, gFin.y, gFin.z },
        0xFF00DDFF,
        m_graphicId,
        SHOW_ALL_SEGMENT));
}
