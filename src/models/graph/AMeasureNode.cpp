#include "models/graph/AMeasureNode.h"

AMeasureNode::AMeasureNode()
    : AClippingNode()
{}

AMeasureNode::AMeasureNode(const AMeasureNode& node)
    : AClippingNode(node)
{}

AMeasureNode::~AMeasureNode()
{}

std::unordered_set<Selection> AMeasureNode::getAcceptableSelections(ManipulationMode mode) const
{
    return {};
}

std::vector<Measure> AMeasureNode::getMeasures() const
{
    return std::vector<Measure>();
}

void AMeasureNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
    for (const Measure& measure : getMeasures())
    {
        glm::dvec3 dir(measure.final - measure.origin);
        glm::dvec3 center = (measure.final + measure.origin) / 2.0;
        glm::dquat quaternion(dir, glm::dvec3(0.0, 0.0, 1.0));

        glm::dmat4 localMat = glm::mat4_cast(quaternion);
        localMat *= glm::dmat4(1.0f, 0.f, 0.f, 0.f,
            0.f, 1.0f, 0.f, 0.f,
            0.f, 0.f, 1.0f, 0.f,
            -center.x, -center.y, -center.z, 1.f);

        glm::vec4 params(m_minClipDist, m_maxClipDist, glm::length(dir) / 2.0, 0.f);

        std::shared_ptr<IClippingGeometry> geom = std::make_shared<CylinderClippingGeometry>(m_clippingMode, localMat, params, 0);
        geom->isSelected = m_selected;
        geom->clipperPhase = getPhase();

        if (m_clippingMode == ClippingMode::showInterior)
            clipAssembly.clippingUnion.push_back(geom);
        else
            clipAssembly.clippingIntersection.push_back(geom);
    }
}
